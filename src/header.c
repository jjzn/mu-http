#include "header.h"
#include <ctype.h>
#include <string.h>

// mu_parse_headers: parse multiple HTTP field lines (headers)
//
// The string raw should point to the beginning of the field line section of a
// HTTP message, and may include (part of) the request body, separated by CRLF.
//
// Returns the number of field lines (headers) parsed, or -1 in case of error
ssize_t mu_parse_headers(char *raw, char **body, struct mu_header *headers, size_t max) {
	// Separate request body from header section
	*body = strstr(raw, "\r\n\r\n");
	(*body)[2] = '\0';
	(*body)[3] = '\0';

	*body += 4; // Advance past double CRLF

	// TODO: it may be more efficient to do manual parsing (no strtok)

	char *curr = strtok(raw, "\r\n");
	size_t idx = 0;

	while (curr != NULL && idx < max) {
		char *colon = strchr(curr, ':');
		if (colon == NULL) // No colon found
			return -1;

		*colon = '\0'; // Seperate field and value

		// Eliminate any optional white space (OWS) between field name and value
		char *value = colon + 1;
		while (isblank(*value))
			value++;

		headers[idx++] = (struct mu_header) { .field = curr, .value = value };

		curr = strtok(NULL, "\r\n");
	}

	return idx;
}
