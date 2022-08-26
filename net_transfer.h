#ifndef NET_TRANSFER_H
#define NET_TRANSFER_H

#include "net_tcp.h"


#define DF_BUF 4096
#define DF_BLOCK 2097152
#define DF_BUF_IN_BLOCK 512

int conn_send(conn_info_t * ci);
int conn_recv(conn_info_t * ci);

#endif
