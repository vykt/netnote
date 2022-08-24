#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net_udp.h"
#include "vector.h"
#include "error.h"

//DEBUG
#include <stdio.h>


int send_ping(send_ping_info_t * si, int msg) {

	ssize_t ret;
	char body[2][MSG_SIZE] = {"scarlet-ping", "scarlet-exit"};
	
	ret = sendto(si->sock, body[msg], strlen(body[msg]), 0,
			     (struct sockaddr *) &si->addr, sizeof(si->addr));
	if (ret == -1) { close(si->sock); return SOCK_SEND_ERR; }
	return SUCCESS;
}


int recv_ping(vector_t * pings, recv_ping_info_t * ri) {

	ssize_t ret;
	int rett;
	unsigned long pos;
	struct addr_ping_info * api;
	char body[MSG_SIZE] = {};
	struct sockaddr_in6 recv_addr;
	socklen_t recv_addr_len = sizeof(recv_addr);

	ret = recvfrom(ri->sock, body, sizeof(body), 0,
			       (struct sockaddr *) &recv_addr, &recv_addr_len);
	if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		return FAIL;
	} else if (ret == -1) {
		close(ri->sock);
		return SOCK_RECV_ERR;
	}

	rett = vector_get_pos_by_dat(pings, (char *) &recv_addr, &pos);
	if (rett != SUCCESS && rett != FAIL) return rett;

	//Check contents of message
	//If exit
	if (!(strcmp(body, "scarlet-exit"))) {

		if (rett != SUCCESS) return SOCK_EXIT_UNKNOWN_ERR;
		
		rett = vector_rmv(pings, pos);
		if (rett != SUCCESS) return ret;
		return SOCK_RECV_EXIT_ERR;
	
	//Else if ping
	} else if (!(strcmp(body, "scarlet-ping"))) {

		//If ping from new, untracked host
		if (rett == FAIL) {

			rett = vector_add(pings, pos, NULL, VECTOR_APPEND_TRUE);
			if (rett != SUCCESS) return ret;
			rett = vector_get_ref(pings, pos, (char **) &api);
			if (ret != SUCCESS) return ret;
			api->addr = recv_addr;

		//Else if ping from known, tracked host
		} else if (rett == SUCCESS) {

			rett = vector_get_ref(pings, pos, (char **) &api);
			if (ret != SUCCESS) return ret;
		}

		//Set last ping time
		api->last_ping = time(NULL);
	}

	return SUCCESS;
}


int init_send_ping_info(send_ping_info_t * si, char * group_addr_str,
						unsigned short port) {
	int ret;

	//Create socket
	si->sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if (si->sock == -1) return SOCK_OPEN_ERR;

	//Create destination address
	struct sockaddr_in6 addr = {AF_INET6, htons(port)};
	si->addr = addr;
	ret = inet_pton(AF_INET6, group_addr_str, &si->addr.sin6_addr);
	if (ret == -1) { close(si->sock); return CONF_ADDR_ERR; }
	return SUCCESS;
}


int init_recv_ping_info(recv_ping_info_t * ri, char * group_addr_str,
						unsigned short port) {
	int ret;
	int reuse = 1;

	//Create socket
	ri->sock = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (ri->sock == -1) return SOCK_OPEN_ERR;

	//Create listening addr & bind
	struct sockaddr_in6 addr = {AF_INET6, htons(port)};
	ri->addr = addr;

	ret = bind(ri->sock, (struct sockaddr *) &ri->addr, sizeof(ri->addr));
	if (ret == -1) { close(ri->sock); return SOCK_BIND_ERR; }

	//Set sock opts
	ret = setsockopt(ri->sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if (ret == -1) { close(ri->sock); return SOCK_OPT_ERR; }

	//Join multicast group
	struct ipv6_mreq group;
	group.ipv6mr_interface = 0;

	ret = inet_pton(AF_INET6, group_addr_str, &group.ipv6mr_multiaddr);
	if (ret != 1) { close(ri->sock); return CONF_ADDR_ERR; }

	ret = setsockopt(ri->sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &group, sizeof(group));
	if (ret == -1) { close(ri->sock); return SOCK_OPT_ERR; }

	return SUCCESS;
}


int close_send_ping_info(send_ping_info_t * si) {

	int ret;
	ret = close(si->sock);
	if (ret == -1) return SOCK_CLOSE_ERR;
	return SUCCESS;
}


int close_recv_ping_info(recv_ping_info_t * ri) {
	
	int ret;
	ret = close(ri->sock);
	if (ret == -1) return SOCK_CLOSE_ERR; 
	return SUCCESS;
}


