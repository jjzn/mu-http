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
#include "request.h"
#include "header.h"

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

	// TODO: setsockopt SO_REUSEADDR or SO_REUSEPORT

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

		//response_handler(conn);
		struct mu_header headers[10];
		struct mu_request req = mu_parse_request(buffer, headers, sizeof(headers) / sizeof(struct mu_header));

		logprint("request is %s %s %s", mu_http_method_labels[req.method], mu_http_version_labels[req.version], req.target);
		for (size_t i = 0; i < req.headers_length; i++)
			logprint("with header %s: %s", req.headers[i].field, req.headers[i].value);

		close(conn);
	}

	close(sockfd);

	return 0;
}
