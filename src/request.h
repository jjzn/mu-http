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

struct mu_request mu_parse_request(char *raw, struct mu_header *headers, size_t max_headers);

#endif
