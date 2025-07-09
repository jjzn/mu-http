#include "request.h"
#include "header.h"
#include <string.h>

struct mu_request err = { .method = HTTP_method_unknown, .version = HTTP_version_unknown, .target = NULL, .headers_length = 0, .headers = NULL };

struct mu_request mu_parse_request(char *raw, struct mu_header *headers, size_t max_headers) {
	struct mu_request req;

	// Parse method
	char *curr = strtok(raw, " ");
	if (curr == NULL)
		return err;
	
	if (strcmp(curr, "GET") == 0)
		req.method = HTTP_GET;
	else if (strcmp(curr, "POST") == 0)
		req.method = HTTP_POST;
	else
	 	req.method = HTTP_method_unknown;

	// Parse target
	curr = strtok(NULL, " ");
	if (curr == NULL)
		return err;

	req.target = curr;

	// Parse version
	curr = strtok(NULL, " ");
	if (curr == NULL)
		return err;

	if (strcmp(curr, "HTTP/1.1") == 0)
		req.version = HTTP_1_1;
	else if (strcmp(curr, "HTTP/2") == 0)
		req.version = HTTP_2;
	else if (strcmp(curr, "HTTP/3") == 0)
		req.version = HTTP_3;
	else
	 	req.version = HTTP_version_unknown;

	// Parse headers
	curr = strtok(NULL, "\r\n");
	if (curr == NULL)
		return err;

	size_t headers_length = mu_parse_headers(curr, headers, max_headers);
	if (headers_length < 0)
		return err;

	req.headers_length = headers_length;
	req.headers = headers;

	return req;
}
