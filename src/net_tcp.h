#ifndef NET_TCP_H
#define NET_TCP_H

#include <sys/stat.h>
#include <linux/limits.h>

#include <netinet/in.h>
#include <time.h>

#include "vector.h"


#define LISTEN_BACKLOG 16

#define CONN_STAT_NO_CONNECTION 0
#define CONN_STAT_SEND_INPROG 1
#define CONN_STAT_RECV_INPROG 2
#define CONN_STAT_SEND_COMPLETE 3
#define CONN_STAT_RECV_COMPLETE 4

#define POLL_CONN_TIMEOUT 10
#define CONN_TIMEOUT 3


typedef struct conn_listener_info conn_listener_info_t;
typedef struct conn_info conn_info_t;


struct conn_listener_info {

	int sock;
	struct sockaddr_in6 addr;

};

struct conn_info {

	//Conn info
	int sock;
	short status;

	//File send/recv info
	int fd;

	//File send info, present in all connections for convenience
	struct stat f_stat;
	char *mmap_addr;
	unsigned int mmap_iter; //Number of blocks written
	size_t mmap_size; //Size of block
	size_t mmap_prog; //Progress through block

    unsigned int seed;
};


int conn_initiate(vector_t * conns, struct sockaddr_in6 addr, char * file);
int conn_listener(vector_t * conns, conn_listener_info_t cli, char * options_arr);
int init_conn_listener_info(conn_listener_info_t * cli, unsigned short port);
int close_conn_listener_info(conn_listener_info_t * cli);


#endif
