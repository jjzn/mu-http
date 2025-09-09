#ifndef REQUEST_H
#define REQUEST_H

#include <stdlib.h>

enum mu_http_method {
	HTTP_GET, HTTP_POST, HTTP_method_unknown
};

enum mu_http_version {
	HTTP_1_1, HTTP_2, HTTP_3, HTTP_version_unknown
};

struct mu_request {
	enum mu_http_method method;
	enum mu_http_version version;
	char *target;

	size_t headers_length;
	struct mu_header *headers;
};

static const struct mu_request mu_request_err = { .method = HTTP_method_unknown, .version = HTTP_version_unknown, .target = NULL, .headers_length = 0, .headers = 0 };

#define mu_request_is_error(req) ( \
	(req).method == mu_request_err.method && \
	(req).version == mu_request_err.version && \
	(req).target == mu_request_err.target && \
	(req).headers_length == mu_request_err.headers_length && \
	(req).headers == mu_request_err.headers)

struct mu_request mu_parse_request(char *raw, struct mu_header *headers, size_t max_headers);

#endif
