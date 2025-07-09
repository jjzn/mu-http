#ifndef HEADER_H
#define HEADER_H

#include <stdlib.h>

struct mu_header {
	char *field;
	char *value;
};

size_t mu_parse_headers(char *raw, struct mu_header *headers, size_t max);

#endif
