#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.h"

#define LISTEN_PORT 8880
#define LISTEN_BACKLOG 64

void logprint(char *fmt, ...) {
	time_t now = time(NULL);
	struct tm *time = localtime(&now);
	va_list args;

	char timefmt[26]; // yyyy-mm-dd hh:mm:ss +hhmm
	strftime(timefmt, sizeof(timefmt) / sizeof(char), "%F %T %z", time);

	printf("[%s] ", timefmt);

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	putchar('\n');
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

	logprint("listening on 0.0.0.0:%d", LISTEN_PORT);

	char buffer[CLIENT_BUFFER_SIZE] = {0};

	while (1) {
		int conn;
		struct sockaddr_in peer_addr;
		socklen_t peer_addr_len = sizeof(peer_addr);

		if ((conn = accept(sockfd, (struct sockaddr *) &peer_addr, &peer_addr_len)) < 0) {
			perror("accept() failed");
			continue;
		}

		logprint("accepted connection from %s", inet_ntoa(peer_addr.sin_addr));

		if (recv(conn, buffer, sizeof(buffer), 0) < 0)
			perror("recv() failed");

		response_handler(conn);

		close(conn);
	}

	close(sockfd);

	return 0;
}
