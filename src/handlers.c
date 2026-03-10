#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>

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

void handler_file(int connfd, struct mu_request req, char *root) {
	// Strip leading / from target, to prevent opening absolute paths
	// See path_resolution(7) for the definition of "absolute path"
	char *relative_target = req.target;
	while (*relative_target == '/')
		relative_target++;

	int rootdir = open(root, O_RDONLY | O_DIRECTORY);
	if (rootdir < 0) {
		perror("open");
		send_status(connfd, 500);
		return;
	}

	// openat ignores rootdir if the path is absolute, which is why we have
	// stripped leading / from the target
	int fd = openat(rootdir, relative_target, O_RDONLY | O_NOFOLLOW);
	if (fd < 0) {
		perror("openat");
		close(rootdir);
		send_status(connfd, 500);
		return;
	}

	// TODO: send Content-Type

	struct stat st;
	if (fstat(fd, &st) < 0 && st.st_size < 0) {
		perror("fstat");
		close(fd);
		close(rootdir);
		send_status(connfd, 500);
		return;
	}

	char content_length[18+20+1]; // 18 chars for header name and CRLF, at most 20 chars for st_size (64 bits), 1 null terminator
	snprintf(content_length, sizeof(content_length) / sizeof(char), "Content-Length: %ld\r\n", st.st_size);
	send_str(connfd, content_length);

	char buf[1024];
	ssize_t n;
	while ((n = read(fd, buf, sizeof(buf))) > 0)
		send(connfd, buf, n, 0);

	if (n < 0)
		perror("read");

	close(fd);
	close(rootdir);
}
