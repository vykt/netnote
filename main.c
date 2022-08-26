#include "net_tcp.h"
#include "net_udp.h"
#include "vector.h"
#include "error.h"

//TODO debug
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main() {

	//TODO temporary values
	char * multicast_addr = "ff04:0000:ce73:602a:0942:bc84:f0b2:e25c";
	unsigned short port_udp = 5148;
	unsigned short port_tcp = 5149;

	char * filepath = "/home/vykt/programming/scarlet/"
	//TODO end temporary values

	//Testing variables
	int ret;
	send_ping_info_t si;
	recv_ping_info_t ri;
	conn_listener_info_t cli;
	struct sockaddr_in6 recv_addr;

	vector_t pings;
	vector_t conns;
	//End testing variables


	//UDP vectors
	ret = vector_ini(&pings, sizeof(addr_ping_info_t));
	printf("init ping info:  %d\n", ret);

	ret = vector_ini(&conns, sizeof(conn_info_t));
	printf("init hosts info: %d\n", ret);
	//END UDP vectors


	//UDP ping send & recv
	//ret = init_send_ping_info(&si, multicast_addr, port_udp);
	//printf("init send ping: %d\n", ret);

	//ret = init_recv_ping_info(&ri, multicast_addr, port_udp);
	//printf("init recv ping: %d\n", ret);

	//ret = send_ping(si, MSG_PING);
	//printf("send ping: %d\n", ret);

	//ret = recv_ping(&pings, &ri);
	//printf("recv ping: %d\n", ret);

	//ret = close_send_ping_info(&si);
	//printf("close send info: %d\n", ret);

	//ret = close_recv_ping_info(&ri);
	//printf("close recv info: %d\n", ret);
	//END UDP ping send & recv


	//TCP connect & listen
	//ret = init_conn_listener_info(&cli, port_tcp);
	//printf("init conn listener: %d\n", ret);

	//ret = conn_listener(&conns, cli);
	//printf("conn listener: %d\n", ret);


	
	//END TCP connect & listen

	return 0;

}
