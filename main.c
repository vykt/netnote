#include "daemon.h"
#include "request.h"
#include "net_transfer.h"
#include "net_tcp.h"
#include "net_udp.h"
#include "vector.h"
#include "error.h"

//TODO debug
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <errno.h>

#include <sys/poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define CONN_MAX 32
#define ARR_AVAILABLE 1
#define ARR_UNAVAILABLE 0

#define POLL_TIMEOUT 5 //Tenth's of a second


int main() {

	//TODO temporary values
	char * multicast_addr = "ff04:0000:ce73:602a:0942:bc84:f0b2:e25c";
	unsigned short port_udp = 5148;
	unsigned short port_tcp = 5149;

	char filepath[PATH_MAX] = "/home/vykt/programming/scarlet/";
	//TODO end temporary values

	//Testing variables
	int ret;
	send_ping_info_t si;
	recv_ping_info_t ri;
	conn_listener_info_t cli;
	struct sockaddr_in6 send_addr = {AF_INET6, htons(port_tcp)};
	struct sockaddr_in6 recv_addr;

	vector_t pings;
	vector_t conns;

	struct pollfd poll_fds[CONN_MAX] = {};
	int poll_fds_availability[CONN_MAX] = {ARR_AVAILABLE};
	unsigned short poll_fds_count = 0;
	//End testing variables


	//Daemon
	ret = init_daemon();

	//End Daemon


	//Request send
	/*req_info_t rsi;
	ret = init_req(&rsi, 0, "hello.txt");
	ret = req_send(&rsi);*/
	//End request send
	
	//Request receive
	/*req_listener_info_t rli;
	ret = init_req_listener(&rli);
	
	poll_fds[0].fd = rli.sock;
	poll_fds[0].events = POLLIN;
	poll_fds_availability[0] = ARR_UNAVAILABLE;
	poll_fds_count++;

	while (1) {
		ret = poll(poll_fds, poll_fds_count, POLL_TIMEOUT * 0); // * 100

		if (poll_fds[0].revents & POLLIN) {
			printf("Running poll\n");
			ret = req_receive(&rli);
			//Now, process request inside daemon
			break;
		}
	}
	ret = close_req_listener(&rli);
	if (ret != SUCCESS);
	*/
	//End request receive


	//UDP vectors
	//ret = vector_ini(&pings, sizeof(addr_ping_info_t));
	//printf("init ping info:  %d\n", ret);

	//ret = vector_ini(&conns, sizeof(conn_info_t));
	//printf("init hosts info: %d\n", ret);
	//END UDP vectors

	//UDP ping send & recv
	//ret = init_send_ping_info(&si, multicast_addr, port_udp);
	//printf("init send ping: %d\n", ret);

	//ret = init_recv_ping_info(&ri, multicast_addr, port_udp);
	//printf("init recv ping: %d\n", ret);

	//poll_fds[0].fd = ri.sock;
	//poll_fds[0].events = POLLIN;
	//poll_fds_availability[0] = ARR_UNAVAILABLE;
	//poll_fds_count++;

	//while (1) {
	//ret = poll(poll_fds, poll_fds_count, POLL_TIMEOUT * 0); // * 100

	//	if (poll_fds[0].revents & POLLIN) {
	//		ret = recv_ping(&pings, &ri);
	//		printf("recv ping: %d\n", ret);
	//	}

	//}


	//ret = send_ping(si, MSG_PING);
	//printf("send ping: %d\n", ret);

	//ret = close_send_ping_info(&si);
	//printf("close send info: %d\n", ret);

	//ret = close_recv_ping_info(&ri);
	//printf("close recv info: %d\n", ret);
	//END UDP ping send & recv


	//TCP listen & recv
	/*ret = init_conn_listener_info(&cli, port_tcp);
	printf("init conn listener: %d\n", ret);

	poll_fds[0].fd = cli.sock;
	poll_fds[0].events = POLLIN;
	poll_fds_availability[0] = ARR_UNAVAILABLE;
	poll_fds_count++;

	conn_info_t * ci;

	while (1) {

		ret = poll(poll_fds, poll_fds_count, POLL_TIMEOUT * 0); // * 100

		if (poll_fds[0].revents & POLLIN) {
			ret = conn_listener(&conns, cli, filepath);
			if (ret != SUCCESS && ret != FAIL) return ret;

			ret = vector_get_ref(&conns, 0, (char **) &ci);
			if (ret != SUCCESS) return ret;

			poll_fds[1].fd = ci->sock;
			poll_fds[1].events = POLLIN;
			poll_fds_count++;
		}

		if (poll_fds[1].revents & POLLIN) {


			ret = vector_get_ref(&conns, 0, (char **) &ci);
			if (ret != SUCCESS) return ret;

			ret = conn_recv(ci);
			if (ret != SUCCESS) return ret;
			if (ci->status == CONN_STAT_RECV_COMPLETE) break;

		} //end if

	}*/
	//END TCP connect & listen

	//TCP send
	/*char * addr_str = "2a02:c7f:f6ef:1c00:160e:1304:8cfe:2431";
	ret = inet_pton(AF_INET6, addr_str, &send_addr.sin6_addr);
	ret = conn_initiate(&conns, send_addr, "hello.txt");	
	
	conn_info_t * ci;
	ret = vector_get_ref(&conns, 0, (char **) &ci);
	if (ret != SUCCESS) return ret;

	poll_fds[0].fd = ci->sock;
	poll_fds[0].events = POLLOUT;
	poll_fds_availability[0] = ARR_UNAVAILABLE;
	poll_fds_count++;

	while (1) {

		ret = poll(poll_fds, poll_fds_count, POLL_TIMEOUT * 0); // * 100

		if (poll_fds[0].revents & POLLOUT) {
			
			ret = conn_send(ci);
			if (ret != SUCCESS) return ret;
			if (ci->status == CONN_STAT_SEND_COMPLETE) break;
		}

	}*/
	//END TCP send

	return 0;

}
