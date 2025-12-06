#include <asm-generic/socket.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.h"
#include "request.h"
#include "header.h"

int sockfd, connfd;

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

void send_str(int connfd, char *str) {
	size_t len = strlen(str);
	send(connfd, str, len, 0);
}

void response_handler(int connfd) {
	char body[] = "hello world\r\n";

	send_str(connfd, "HTTP/1.1 200 OK\r\n\r\n");
	send_str(connfd, body);
}

void cleanup(int signum) {
	(void) signum; // Prevent compilation warning
	close(sockfd);
	close(connfd);
	exit(EXIT_SUCCESS);
}

int main(void) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	struct sigaction sa;
	sa.sa_handler = cleanup;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	if (sigaction(SIGTERM, &sa, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(LISTEN_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	int reuseaddr_opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
	// TODO: setsockopt SO_REUSEADDR or SO_REUSEPORT

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(sockfd, LISTEN_BACKLOG) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	logprint("listening on 0.0.0.0:%d", LISTEN_PORT);

	char buffer[CLIENT_BUFFER_SIZE] = {0};

	while (1) {
		struct sockaddr_in peer_addr;
		socklen_t peer_addr_len = sizeof(peer_addr);

		if ((connfd = accept(sockfd, (struct sockaddr *) &peer_addr, &peer_addr_len)) < 0) {
			perror("accept");
			continue;
		}

		logprint("accepted connection from %s", inet_ntoa(peer_addr.sin_addr));

		ssize_t read = recv(connfd, buffer, sizeof(buffer), 0);
		if (read < 0) {
			perror("recv");
			continue;
		}

		//response_handler(connfd);
		char *body;
		int body_malloced = 0;

		struct mu_header headers[10];
		struct mu_request req = mu_parse_request(buffer, &body, headers, sizeof(headers) / sizeof(struct mu_header));

		struct mu_header header_cl = mu_find_header(req, "Content-Length");
		size_t bodylen = strlen(body);
		size_t content_length = 0;
		if (!mu_header_is_error(header_cl))
			content_length = atoi(header_cl.value); // Returns 0 on erorr, which is fine

		if (content_length > bodylen) {
			char *bodybuff = malloc(content_length);
			if (bodybuff == NULL) {
				perror("malloc");
				continue;
			}

			// Copy part of body already read into new buffer
			memcpy(bodybuff, body, bodylen);

			// Try to read rest of request body into buffer
			read = recv(connfd, bodybuff + bodylen, content_length - bodylen, 0);
			if (read < 0) {
				perror("recv");
				continue;
			}

			if ((size_t) read != content_length - bodylen) // We can cast `read` to size_t because `read` > 0
				logprint("error: expected to receive %d octets, recv'ed %d octets", content_length - bodylen, read);

			body = bodybuff; // Make the new body available as `body`
			body_malloced = 1;
		}

		logprint("content length is %d", content_length);
		logprint("recv'ed body length is %d", strlen(body));

		logprint("%s", body);

		logprint("request is %s %s %s", mu_http_method_labels[req.method], mu_http_version_labels[req.version], req.target);
		for (size_t i = 0; i < req.headers_length; i++)
			logprint("with header %s: %s", req.headers[i].field, req.headers[i].value);

		close(connfd);

		// Free body buffer if needed
		if (body_malloced)
			free(body);
	}

	close(sockfd); // TODO: or should I call cleanup(-1)?

	return 0;
}
