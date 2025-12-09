#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "request.h"
#include "header.h"
#include "utils.h"
#include "handlers.h"

void send_str(int connfd, char *str) {
	size_t len = strlen(str);
	send(connfd, str, len, 0);
}

void send_status(int connfd, unsigned short status) {
	// We don't care about checking for valid status codes
	char buff[18];
	snprintf(buff, sizeof(buff) / sizeof(char), "HTTP/1.1 %hu \r\n", status);

	send_str(connfd, buff);
}

void handler_logreq(int connfd, struct mu_request req) {
	(void)connfd;

	size_t content_length = 0;
	struct mu_header header_cl = mu_find_header(req, "Content-Length");
	if (!mu_header_is_error(header_cl))
		content_length = atoi(header_cl.value);

	logprint("(fd: %d) content length is %d", connfd, content_length);
	logprint("(fd: %d) recv'ed body length is %d", connfd, strlen(req.body));

	logprint("(fd: %d) request is %s %s %s", connfd, mu_http_method_labels[req.method], mu_http_version_labels[req.version], req.target);
	for (size_t i = 0; i < req.headers_length; i++)
		logprint("(fd: %d) with header %s: %s", connfd, req.headers[i].field, req.headers[i].value);
}

void handler_echo(int connfd, struct mu_request req) {
	handler_logreq(connfd, req);

	logprint("(fd: %d) handler_echo()", connfd);

	send_str(connfd, "HTTP/1.1 200 OK\r\n\r\n");
	send_str(connfd, req.body);
}
