#ifndef HANDLERS_H
#define HANDLERS_H

/* send_status: sends a HTTP status code
 *
 * Sends a HTTP/1.1 status line with the specified status code
 */
void send_status(int connfd, unsigned short status);

/* handler_logreq: logs request info
 *
 * Prints to log details about the request, such as content length, actual
 * received content length, request method, HTTP version, request target, and
 * request headers.
 */
void handler_logreq(int connfd, struct mu_request req);

/* handler_echo: echoes back the request body
 *
 * Responds with a status code of 200, and the same body as the original request.
 */
void handler_echo(int connfd, struct mu_request req);

/* handler_file: sends the file specified by the request target
 *
 * Sends the file specified by the request target, relative to the specified
 * root. '.' and '..' are interpreted accordingly, while ensuring no files
 * outside of root are sent (no directory traversal).
 */
void handler_file(int connfd, struct mu_request req, char *root);

#endif
