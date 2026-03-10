#include <asm-generic/socket.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config.h"
#include "request.h"
#include "header.h"
#include "utils.h"
#include "handlers.h"

int sockfd;

void cleanup(int signum) {
    (void) signum; // Prevent compilation warning
    close(sockfd);
    exit(EXIT_SUCCESS);
}

/* handle_connection: handles an incoming connection request
 *
 * The function tries to read the request header into a static buffer, from
 * which the request method, version, target, and headers are parsed (see
 * mu_parse_request in src/request.h).
 *
 * From the parsed headers, the Content-Length is extracted, and a buffer of
 * that size is allocated. The remainder of the request body is then read into
 * the buffer, and merged with any part of the body which might have been read
 * by the initial call to recv.
 */
void handle_connection() {
    char buffer[CLIENT_BUFFER_SIZE] = {0};
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);

    int connfd = accept(sockfd, (struct sockaddr *) &peer_addr, &peer_addr_len);
    if (connfd < 0) {
        perror("accept");
        return;
    }

    puts("");
    logprint("(fd: %d) accepted connection from %s", connfd, inet_ntoa(peer_addr.sin_addr));

    ssize_t read = recv(connfd, buffer, sizeof(buffer), 0);
    if (read < 0) {
        perror("recv");
        return;
    }

    struct mu_header headers[CLIENT_MAX_HEADERS];
    struct mu_request req = mu_parse_request(buffer, headers, sizeof(headers) / sizeof(struct mu_header));
    if (mu_request_is_error(req)) {
        logprint("(fd: %d) failed to parse request, sending status 400", connfd);
        send_status(connfd, 400);
        return;
    }

    // TODO: check against CLIENT_MAX_BODY_LENGTH
    struct mu_header header_cl = mu_find_header(req, "Content-Length");
    size_t bodylen = strlen(req.body);
    size_t content_length = 0;
    if (!mu_header_is_error(header_cl))
        content_length = atoi(header_cl.value); // Returns 0 on erorr, which is fine

    int body_malloced = 0;
    if (content_length > bodylen) {
        char *bodybuff = malloc(content_length);
        if (bodybuff == NULL) {
            perror("malloc");
            return;
        }

        // Copy part of body already read into new buffer
        memcpy(bodybuff, req.body, bodylen);

        // Try to read rest of request body into buffer
        read = recv(connfd, bodybuff + bodylen, content_length - bodylen, 0);
        if (read < 0) {
            perror("recv");
            return;
        }

        if ((size_t) read != content_length - bodylen) // We can cast `read` to size_t because `read` > 0
            logprint("(fd: %d) error: expected to receive %d octets, recv'ed %d octets", connfd, content_length - bodylen, read);

        req.body = bodybuff; // Make the new body available as `body`
        body_malloced = 1;
    }

    config__handle(connfd, req);

    close(connfd);
    logprint("(fd: %d) connection closed", connfd);

    // Free body buffer if needed
    if (body_malloced)
        free(req.body);
}

int main(void) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = cleanup;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LISTEN_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    int reuseaddr_opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));

    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, LISTEN_BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    logprint("listening on 0.0.0.0:%d", LISTEN_PORT);

    while (1) {
        handle_connection();
    }

    close(sockfd);

    return 0;
}
