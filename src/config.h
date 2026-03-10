#ifndef CONFIG_H
#define CONFIG_H

#include "request.h"

#define CLIENT_MAX_BODY_LENGTH 1048576
#define CLIENT_MAX_HEADERS_LENGTH 8192
#define CLIENT_MAX_HEADERS 128
#define CLIENT_MAX_URI_LENGTH 8192

// TODO: adapt for arbitrary methods
#define CLIENT_BUFFER_SIZE (17 + CLIENT_MAX_HEADERS_LENGTH + CLIENT_MAX_URI_LENGTH)

#define LISTEN_PORT 8880
#define LISTEN_BACKLOG 64

void config__handle(int connfd, struct mu_request req);

#endif
