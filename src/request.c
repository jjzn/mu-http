#include "request.h"
#include "header.h"
#include <string.h>

// Returns a partial mu_request containing no headers
struct mu_request _mu_parse_request_startline(char *startline) {
	struct mu_request req = mu_request_err;
	char *saveptr;

	// Parse method
	char *token = strtok_r(startline, " ", &saveptr);
	if (token == NULL)
		return mu_request_err;

	if (strcmp(token, "GET") == 0)
		req.method = HTTP_GET;
	else if (strcmp(token, "POST") == 0)
		req.method = HTTP_POST;
	else
	 	req.method = HTTP_method_unknown;

	// Parse target
	token = strtok_r(NULL, " ", &saveptr);
	if (token == NULL)
		return mu_request_err;

	req.target = token;

	// Parse version
	token = strtok_r(NULL, " ", &saveptr);
	if (token == NULL)
		return mu_request_err;

	if (strcmp(token, "HTTP/1.1") == 0)
		req.version = HTTP_1_1;
	else if (strcmp(token, "HTTP/2") == 0)
		req.version = HTTP_2;
	else if (strcmp(token, "HTTP/3") == 0)
		req.version = HTTP_3;
	else
	 	req.version = HTTP_version_unknown;

	return req;
}

struct mu_request mu_parse_request(char *raw, struct mu_header *headers, size_t max_headers) {
	struct mu_request req = mu_request_err;
	char *lineptr;

	char *line = strtok_r(raw, "\r\n", &lineptr);
	if (line == NULL)
		return mu_request_err;

	req = _mu_parse_request_startline(line);
	if (mu_request_is_error(req))
		return mu_request_err;
	
	// Parse headers
	line = strtok_r(NULL, "\r\n", &lineptr);
	if (line == NULL)
		return mu_request_err;

	ssize_t headers_length = mu_parse_headers(line, headers, max_headers);
	if (headers_length < 0)
		return mu_request_err;

	req.headers_length = headers_length;
	req.headers = headers;

	return req;
}
