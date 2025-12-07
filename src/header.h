#ifndef HEADER_H
#define HEADER_H

#include <stdlib.h>
#include <sys/types.h>

struct mu_header {
	char *field;
	char *value;
};

static const struct mu_header mu_header_err = { .field = NULL, .value = NULL };

#define mu_header_is_error(header) ((header).field == mu_header_err.field && (header).value == mu_header_err.value)
#define mu_header_expect(header, other) (mu_header_is_error(header) ? (other) : (header))

ssize_t mu_parse_headers(char *raw, char **body, struct mu_header *headers, size_t max);

#endif
