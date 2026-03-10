#include <string.h>

#include "request.h"
#include "handlers.h"

int has_prefix(char *target, char *prefix) {
    while (*target != '\0' && *prefix != '\0') {
        if (*target != *prefix)
            return 0;

        target++;
        prefix++;
    }

    return 1;
}

int is_exact(char *target, char *match) {
    return strcmp(target, match) == 0;
}

define(`begin', `void config__handle(int connfd, struct mu_request req) {')dnl
define(`end', `}')dnl
define(`HANDLER', `handler_$1(connfd, req`'ifelse(`$1', `file', `, root', `'))')dnl
define(`HANDLERS', `ifelse(`$1', `', `', `HANDLER($1);
        HANDLERS(shift($@))')')dnl
define(`prefix', `if (has_prefix(req.target, "$1")) {
        char root[] = "$1";
        HANDLERS(shift($@))return;
    }')dnl
define(`exact', `if (is_exact(req.target, "$1")) {
        HANDLERS(shift($@))return;
    }')dnl
