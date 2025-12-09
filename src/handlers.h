#ifndef HANDLERS_H
#define HANDLERS_H

void send_status(int connfd, unsigned short status);

void handler_logreq(int connfd, struct mu_request req);
void handler_echo(int connfd, struct mu_request req);

#endif
