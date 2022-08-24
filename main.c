#include "net_tcp.h"
#include "net_udp.h"
#include "vector.h"
#include "error.h"


//TODO debug
#include <stdio.h> 


int main() {

	//TODO temporary values
	char * multicast_addr = "ff04:0000:ce73:602a:0942:bc84:f0b2:e25c";
	unsigned short port_udp = 5148;
	unsigned short port_tcp = 5149;
	//TODO end temporary values

	//Testing variables
	int ret;
	send_ping_info_t si;
	recv_ping_info_t ri;
	struct sockaddr_in6 recv_addr;

	vector_t pings;
	vector_t hosts;
	//End testing variables


	//UDP vectors
	ret = vector_ini(&pings, sizeof(addr_ping_info_t));
	printf("init ping info:  %d\n", ret);

	ret = vector_ini(&hosts, sizeof(conn_info_t));
	printf("init hosts info: %d\n", ret);


	//ret = init_send_ping_info(&si, multicast_addr, port_udp);
	//printf("init send ping: %d\n", ret);

	ret = init_recv_ping_info(&ri, multicast_addr, port_udp);
	printf("init recv ping: %d\n", ret);

	//ret = send_ping(&si, MSG_PING);
	//printf("send ping: %d\n", ret);

	ret = recv_ping(&pings, &ri);
	printf("recv ping: %d\n", ret);

	//ret = close_send_ping_info(&si);
	//printf("close send info: %d\n", ret);

	ret = close_recv_ping_info(&ri);
	printf("close recv info: %d\n", ret);

	return 0;

}
