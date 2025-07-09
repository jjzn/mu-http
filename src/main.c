#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LISTEN_PORT 8880
#define LISTEN_BACKLOG 64

#define RECV_BUFFER 1024

void logprint(char *msg) {
	time_t now = time(NULL);
	struct tm *time = localtime(&now);

	char fmt[26]; // yyyy-mm-dd hh:mm:ss +hhmm
	strftime(fmt, sizeof(fmt) / sizeof(char), "%F %T %z", time);

	printf("[%s] %s\n", fmt, msg);
}

void send_str(int conn, char *str) {
	size_t len = strlen(str);
	send(conn, str, len, 0);
}

void response_handler(int conn) {
	char body[] = "hello world\r\n";

	send_str(conn, "HTTP/1.1 200 OK\r\n\r\n");
	send_str(conn, body);
}

int main(void) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(LISTEN_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind() failed");
		exit(EXIT_FAILURE);
	}

	if (listen(sockfd, LISTEN_BACKLOG) < 0) {
		perror("listen() failed");
		exit(EXIT_FAILURE);
	}

	printf("listening on 0.0.0.0:%d\n", LISTEN_PORT);

	char buffer[RECV_BUFFER] = {0};

	while (1) {
		int conn;

		// We are not interested in the peer address, for now
		if ((conn = accept(sockfd, NULL, 0)) < 0) {
			perror("accept() failed");
			continue;
		}

		logprint("connected");

		if (recv(conn, buffer, sizeof(buffer), 0) < 0)
			perror("recv() failed");

		response_handler(conn);

		close(conn);
	}

	close(sockfd);

	return 0;
}
