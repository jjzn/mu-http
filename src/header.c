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
    char *curr = raw;
    size_t idx = 0;

    while (*curr != '\0' && idx < max) {
        char *field = curr;

        while (*curr != '\0' && *curr != ':')
            curr++;

        if (*curr == '\0') // No colon found
            return -1;

        // Eliminate any optional white space (OWS) between field name and value
        *curr = '\0';
        curr++;

        char *value = curr;
        while (isblank(*value))
            value++;

        while (*curr != '\0' && *curr != '\n')
            curr++;

        // Accept either bare LF, or CRLF (the CR gets replaced)
        if (*curr == '\n' && curr[-1] == '\r')
            curr[-1] = '\0';

        // Remove newline and advance past it
        if (*curr == '\n') {
            *curr = '\0';
            curr++;
        }

        // Finish parsing once we encounter another CRLF or LF (or the string ends)
        if (*curr == '\0') {
            break;
        } else if (*curr == '\r' && curr[1] == '\n') {
            curr += 2;
            break;
        } else if (*curr == '\n') {
            curr++;
            break; // TODO: what if we have CRLF, CR??
        }

        headers[idx++] = (struct mu_header) { .field = field, .value = value };
    }

    *body = curr;
    return idx;
}
