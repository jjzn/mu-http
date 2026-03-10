#ifndef REQUEST_H
#define REQUEST_H

#include <stdlib.h>

#define GENERATE_ENUM(id, label) id,
#define GENERATE_LABELS(id, label) label,

#define MU_HTTP_METHOD(FN) \
    FN(HTTP_GET, "GET") \
    FN(HTTP_POST, "POST") \
    FN(HTTP_method_unknown, "[unknown method]")

#define MU_HTTP_VERSION(FN) \
    FN(HTTP_1_1, "HTTP/1.1") \
    FN(HTTP_2, "HTTP/2") \
    FN(HTTP_3, "HTTP/3") \
    FN(HTTP_version_unknown, "[unknown version]")

enum mu_http_method { MU_HTTP_METHOD(GENERATE_ENUM) };
extern char *mu_http_method_labels[];

enum mu_http_version { MU_HTTP_VERSION(GENERATE_ENUM) };
extern char *mu_http_version_labels[];

struct mu_request {
    enum mu_http_method method;
    enum mu_http_version version;
    char *target;

    size_t headers_length;
    struct mu_header *headers;

    char *body;
};

static const struct mu_request mu_request_err = { .method = HTTP_method_unknown, .version = HTTP_version_unknown, .target = NULL, .headers_length = 0, .headers = NULL, .body = NULL };

#define mu_request_is_error(req) ( \
    (req).method == mu_request_err.method && \
    (req).version == mu_request_err.version && \
    (req).target == mu_request_err.target && \
    (req).headers_length == mu_request_err.headers_length && \
    (req).headers == mu_request_err.headers && \
    (req).body == mu_request_err.body)

struct mu_request mu_parse_request(char *raw, struct mu_header *headers, size_t max_headers);
struct mu_header mu_find_header(struct mu_request req, char *field);

#endif
