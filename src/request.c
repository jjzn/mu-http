#include "request.h"
#include "header.h"
#include <string.h>

// Label arrays are initialized here to avoid duplication across translation
// units, which would happen if they were initialized in the header using static
char *mu_http_method_labels[] = { MU_HTTP_METHOD(GENERATE_LABELS) };
char *mu_http_version_labels[] = { MU_HTTP_VERSION(GENERATE_LABELS) };

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

// Parse the request startline and headers
struct mu_request mu_parse_request(char *raw, struct mu_header *headers, size_t max_headers) {
    struct mu_request req = mu_request_err;

    char *lineend = strstr(raw, "\n"); // Where the start line ends
    if (lineend == NULL || lineend == raw)
        return mu_request_err;

    // Replace LF, and CR if present
    *lineend = '\0';
    if (lineend[-1] == '\r')
        lineend[-1] = '\0';

    req = _mu_parse_request_startline(raw);
    if (mu_request_is_error(req))
        return mu_request_err;
    
    // Parse headers
    ssize_t headers_length = mu_parse_headers(lineend + 1, &req.body, headers, max_headers);
    if (headers_length < 0)
        return mu_request_err;

    req.headers_length = headers_length;
    req.headers = headers;

    return req;
}

struct mu_header mu_find_header(struct mu_request req, char *field) {
    for (size_t i = 0; i < req.headers_length; i++) {
        if (strcmp(req.headers[i].field, field) == 0)
            return req.headers[i];
    }

    return mu_header_err;
}
