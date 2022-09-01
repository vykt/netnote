#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>

#include <sys/socket.h>
#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/un.h>

#include "request.h"
#include "error.h"


/* TODO
 *
 *	When user requests to send some file, check their UID has read permission to said
 *	file.
 *
 *	Request format:
 *		"destination ID\file path"
 *
 *  Check user credentials using SO_PEERCRED, seteuid(2) to their UID, perform
 *  command, return to root permissions.
 */


int req_initiate(req_info_t * ri) {

	int ret;
	ssize_t rd_wr;
	char itoa_buf[4] = {};
	char sock_path[108] = {};	
	char request[PATH_MAX+4] = {};
	
	const char * req_root = "/var/run/scarlet";
	const char * req_sock = "sock";

	ri->sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (ri->sock == -1) return SOCK_OPEN_ERR;

	//Build socket location	
	strcat(sock_path, req_root);
	strcat(sock_path, sock);

	//Build address
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, sock_path);

	//Build request
	itoa(ri->target_host_id, itoa_buf, 10);
	strcpy(request, itoa_buf);
	strcpy(request, "\\");
	strcpy(request, file);

	//Connect
	ret = connect(ri->sock, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1) { close(ri->sock); return SOCK_CONNECT_ERR; }

	//Write to socket
	rd_wr = send(ri->sock, request, strlen(request), 0);
	if (rd_wr == -1) { close(ri->sock); return SOCK_SEND_ERR; }

	//Receive response
	rd_wr = recv(ri->sock, ri->reply, REQ_REPLY_SIZE, 0);
	if (rd_wr <= 0) { close(ri->sock); return SOCK_RECV_ERR: }

	close(ri->sock);
	return SUCCESS;
}


int init_req(req_info_t * ri, int target_host_id, char * file) {

	ri->target_host_id = target_host_id;
	strcpy(ri->file, file);

	return SUCCESS;
}


int init_req_listener(req_listener_info_t * rli) {

	int ret;
	char sock_path[108] = {}; //Max path of sockets, see man unix(7).

	const char * req_root = "/var/run/scarlet";
	const char * req_sock = "sock";

	//Create socket
	rli->sock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (rli->sock == -1) return SOCK_OPEN_ERR;

	//Create path for socket
	strcat(sock_path, req_root);
	strcat(sock_path, sock);

	//Create address
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, sock_path);

	//Bind & listen
	ret = bind(rli->sock, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1) { close(rli->sock); return SOCK_BIND_ERR; }

	ret = listen(rli->sock, REQ_BACKLOG);
	if (ret == -1) { close(rli->sock); return SOCK_LISTEN_ERR; }
	
	return SUCCESS;
}


int close_req_listener(req_listener_info_t * rli) {

	int ret;
	ret = close(rli->sock);
	if (ret == -1) return SOCK_CLOSE_ERR;
	return SUCCESS;
}
