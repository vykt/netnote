#ifndef NET_UDP_H
#define NET_UDP_H

#include <time.h>

#include <netinet/in.h>

#include "vector.h"


#define PING_TIMEOUT 30 //seconds
#define PING_INTERVAL 8 //seconds
#define MSG_SIZE 32
#define IPV6_ADDR_ARR_SIZE 16 //man 7 ipv6
#define MSG_PING 0
#define MSG_EXIT 1


typedef struct addr_ping_info addr_ping_info_t;
typedef struct send_ping_info send_ping_info_t;
typedef struct recv_ping_info recv_ping_info_t;


struct addr_ping_info {

	struct sockaddr_in6 addr;
	time_t last_ping;
};

struct send_ping_info {

	int sock;
	struct sockaddr_in6 addr;
	time_t last_ping;
};

struct recv_ping_info {

	int sock;
	struct sockaddr_in6 addr;
	struct sockaddr_in6 addr_ignore;
};


int check_ping_times(vector_t * pings);

int send_ping(send_ping_info_t * si, int msg);
int recv_ping(vector_t * pings, recv_ping_info_t * ri);

int init_send_ping_info(send_ping_info_t * ri, char * group_addr_str,
						unsigned short port);
int init_recv_ping_info(recv_ping_info_t * ri, char * group_addr_str,
						unsigned short port);

int close_send_ping_info(send_ping_info_t * si);
int close_recv_ping_info(recv_ping_info_t * ri);


#endif
