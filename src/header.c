#include "header.h"
#include <string.h>

size_t mu_parse_headers(char *raw, struct mu_header *headers, size_t max) {
	char *curr = strtok(raw, "\r\n");
	size_t idx = 0;

	while (curr != NULL && idx < max) {
		char *colon = strchr(curr, ':');
		if (colon == NULL) // No colon found
			return -1;

		*colon = '\0'; // Seperate field and value
		headers[idx++] = (struct mu_header) { .field = curr, .value = colon + 1 };

		curr = strtok(NULL, "\r\n");
	}

	return idx;
}
