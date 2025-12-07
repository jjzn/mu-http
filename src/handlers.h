#ifndef HANDLERS_H
#define HANDLERS_H

void handler_logreq(int connfd, struct mu_request req);
void handler_echo(int connfd, struct mu_request req);

#endif
