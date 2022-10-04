#ifndef DAEMON_H
#define DAEMON_H


#define UDP_LISTENER 0
#define TCP_LISTENER 1
#define REQ_LISTENER 2

#define POLL_ARR_MIN 3 //Minimum index for connection sockets
#define POLL_TIMEOUT 1

#define CONN_MAX 32


int term_handler();
int term();
void main_daemon();
int init_daemon();




#endif
