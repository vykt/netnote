#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "net_tcp.h"
#include "vector.h"
#include "error.h"

//DEBUG
#include <stdio.h>


/*
 *  conn_initiate() - Called when sending file to specified address.
 *
 *  conn_listener() - Periodically called to accept incoming files.
 */


int conn_initiate(vector_t * conns, struct sockaddr_in6 addr, unsigned short port) {

	int ret;
	int sock_conn;
	conn_info_t ci;

	//Create socket
	sock_conn = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (sock_conn == -1) return SOCK_OPEN_ERR;

	//Try to connect to conn
	ret = connect(sock_conn, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1) { close(sock_conn); return SOCK_CONNECT_ERR; }

	//Build conn_info
	ci.sock = sock_conn;
	ci.status = CONN_STAT_SEND_WAITING;

	//Add to vector of ongoing connections
	ret = vector_add(conns, 0, (char *) &ci, VECTOR_APPEND_TRUE);
	if (ret != SUCCESS) return ret;

	return SUCCESS;
	
}


int conn_listener(vector_t * conns, conn_listener_info_t cli) {

	int ret;
	int sock_conn;
	conn_info_t * ci;

	socklen_t addr_len = sizeof(cli.addr);
	
	//Accept incoming connection
	sock_conn = accept(cli.sock, (struct sockaddr *) &cli.addr, &addr_len);
	if (sock_conn == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		return FAIL;
	} else if (sock_conn == -1) {
		close (cli.sock);
		return SOCK_RECV_ERR;
	}

	//Initialise conn
	ret = vector_add(conns, 0, NULL, VECTOR_APPEND_TRUE);
	if (ret != SUCCESS) return ret;
	ret = vector_get_ref(conns, conns->length, (char **) &ci);
	if (ret != SUCCESS) return ret;
	ci->sock = sock_conn;
	ci->status = CONN_STAT_RECV_WAITING;

	return SUCCESS;
}


int init_conn_listener_info(conn_listener_info_t * cli, unsigned short port) {

	int ret;
	int reuse = 1;

	//Create socket
	cli->sock = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (cli->sock == -1) return SOCK_OPEN_ERR;

	//Create listening addr & bind
	struct sockaddr_in6 addr = {AF_INET6, htons(port)};
	cli->addr = addr;
	cli->addr.sin6_addr = in6addr_any;

	ret = bind(cli->sock, (struct sockaddr *) &cli->addr, sizeof(cli->addr));
	if (ret == -1) { close(cli->sock); return SOCK_BIND_ERR; }
	
	//Set sock opts
	ret = setsockopt(cli->sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if (ret == -1) { close(cli->sock); return SOCK_OPT_ERR; }

	//Listen
	ret = listen(cli->sock, LISTEN_BACKLOG);
	if (ret == -1) { close(cli->sock); return SOCK_LISTEN_ERR; }

	return SUCCESS;
}


int close_conn_listener_info(conn_listener_info_t * cli) {

	int ret;
	ret = close(cli->sock);
	if (ret == -1) return SOCK_CLOSE_ERR;
	return SUCCESS;
}