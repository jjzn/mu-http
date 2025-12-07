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

void handler_logreq(int connfd, struct mu_request req) {
	(void)connfd;

	size_t content_length = 0;
	struct mu_header header_cl = mu_find_header(req, "Content-Length");
	if (!mu_header_is_error(header_cl))
		content_length = atoi(header_cl.value);

	logprint("content length is %d", content_length);
	logprint("recv'ed body length is %d", strlen(req.body));

	logprint("%s", req.body);

	logprint("request is %s %s %s", mu_http_method_labels[req.method], mu_http_version_labels[req.version], req.target);
	for (size_t i = 0; i < req.headers_length; i++)
		logprint("with header %s: %s", req.headers[i].field, req.headers[i].value);
}

void handler_echo(int connfd, struct mu_request req) {
	handler_logreq(connfd, req);

	logprint("handler_echo()");

	send_str(connfd, "HTTP/1.1 200 OK\r\n\r\n");
	send_str(connfd, req.body);
}
