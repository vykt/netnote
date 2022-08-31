#ifndef NET_TRANSFER_H
#define NET_TRANSFER_H

#include <unistd.h>

#include "net_tcp.h"

#define DF_BLOCK 2097152 //Default mmap block size  (2Mb)
#define DF_LEN 4096		 //Default send buffer size (4Kb)


int conn_send(conn_info_t * ci);
int conn_recv(conn_info_t * ci);

#endif
