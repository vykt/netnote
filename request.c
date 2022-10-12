#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/un.h>

#include "request.h"
#include "net_udp.h"
#include "vector.h"
#include "error.h"

/* TODO
 *
 *	When user requests to send some file, check their UID has read permission to said
 *	file.
 *
 *	Request format:
 *		"destination ID\file path"
 *				   OR
 *		"list"
 *
 *  Check user credentials using SO_PEERCRED, seteuid(2) to their UID, perform
 *  command, return to root permissions.
 */


//Check user credentials for file, refuse send if authorisation not met
int req_authorise(char * request_path, req_cred_t * rc) {

	int ret;
	int fd;
	int fail = 0;
	uid_t def_uid;
	gid_t def_gid;

	//Get & save current uid
	def_uid = getuid();
	def_gid = getgid();

	//Set euid & egid	
	ret = setegid(rc->gid);
	if (ret == -1) return REQUEST_PERM_ERR;
	
	ret = seteuid(rc->uid);
	if (ret == -1) return REQUEST_PERM_ERR;

	//Attempt to open file for reading
	fd = open(request_path, O_RDONLY);
	if (fd == -1 && errno == EEXIST) fail = 1; //return REQUEST_FILE_EXIST_ERR;
	if (fd == -1 && errno == EACCES) fail = 2; //return REQUEST_PERM_ERR;
	close(fd);

	//Set euid back to default
	ret = seteuid(def_uid);
	if (ret == -1) return CRITICAL_ERR;
	ret = setegid(def_gid);
	if (ret == -1) return CRITICAL_ERR;

	if (fail == 1) {
		return REQUEST_FILE_EXIST_ERR;
	} else if (fail == 2) {
		return REQUEST_PERM_ERR;
	}

	return SUCCESS;
}


//Used by client, socket blocks
int req_send(req_info_t * ri) {

	int ret;
	ssize_t rd_wr;
	char itoa_buf[4] = {};
	char sock_path[108] = {};	
	char request[PATH_MAX+4] = {};
	
	const char * req_root = "/var/run/scarlet/";
	const char * req_sock = "sock";

	ri->sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (ri->sock == -1) return SOCK_OPEN_ERR;

	//Build socket location	
	strcat(sock_path, req_root);
	strcat(sock_path, req_sock);

	//Build address
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, sock_path);

	//Check request type
	sprintf(itoa_buf, "%d", ri->target_host_id + 1);

	//Build request
	if(itoa_buf == 0) {
		strcat(request, "list");
	} else {
		strcat(request, itoa_buf);
		strcat(request, "\\");
		strcat(request, ri->file);
	}

	//Connect
	ret = connect(ri->sock, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1) { close(ri->sock); return SOCK_CONNECT_ERR; }

	//Write to socket
	rd_wr = send(ri->sock, request, strlen(request), 0);
	if (rd_wr == -1) { close(ri->sock); return SOCK_SEND_ERR; }

	//Receive response
	rd_wr = recv(ri->sock, ri->reply, REQ_REPLY_SIZE, 0);
	if (rd_wr <= 0) { close(ri->sock); return SOCK_RECV_ERR; }

	close(ri->sock);
	return SUCCESS;
}


//Used by daemon. Listening socket doesn't block, communicating socket does block
int req_receive(req_listener_info_t * rli, req_cred_t * rc, vector_t * pings) {

	int ret;
	int sock_conn;
	ssize_t rd_wr;
	struct ucred cred;
	addr_ping_info_t * pi;
	socklen_t len = sizeof(cred);

	char request[PATH_MAX+4] = {};
	char reply[PATH_MAX+4] = {};
	char * addr_buf;
	char num_buf[16] = {};
	char * request_id;
	char * request_path;

	char * perm_err_response = "perm_err";
	char * file_exist_err_response = "file_exist_err";
	char * successful_response = "success";

	//Try listen
	sock_conn = accept(rli->sock, NULL, NULL);
	if (sock_conn == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		return FAIL;
	} else if (sock_conn == -1) {
		close (rli->sock);
		return SOCK_RECV_ERR;
	}

	//Get UID of unix socket peer
	ret = getsockopt(sock_conn, SOL_SOCKET, SO_PEERCRED, &cred, &len);
	if (ret == -1) {
		close(sock_conn);
		return SOCK_CRED_ERR;
	}

	rc->uid = cred.uid;
	rc->gid = cred.gid;

	//Receive request
	rd_wr = recv(sock_conn, request, PATH_MAX+4, 0);
	if (rd_wr <= 0) { close(sock_conn); return SOCK_RECV_ERR; }

	//Process request
	request_id = strtok(request, "\\");
	request_path = strtok(NULL, "\\");

	//If asking for list
	ret = strcmp(request_path, "LIST");	
	if (!ret) {

		//For every ping, build line of response message
		for (int i = 0; i < pings->length; i++) {

			ret = vector_get_ref(pings, i, (char **) &pi);
			if (ret != SUCCESS) { close(sock_conn); return ret; }

			sprintf(num_buf, "%d : ", i);
			strcat(num_buf, reply);
			addr_buf = inet_ntop(AF_INET6, (void *restrict) &pi->addr, 
											addr_buf, strlen(addr_buf));
			if (addr_buf == NULL) { close(sock_conn); return REQUEST_GENERIC_ERR; }
			strcat(addr_buf, reply);
			strcat("\n", reply);

		}
		
		//Send reply to caller
		rd_wr = send(sock_conn, reply, strlen(reply), 0);
		if (rd_wr == -1) { close(sock_conn); return SOCK_SEND_ERR; }

		close(sock_conn);
		return SUCCESS;
	}
	
	//If asking for send

	//Test permissions
	ret = req_authorise(request_path, rc);
	
	//If permissions insufficient
	if (ret == REQUEST_PERM_ERR) {
		rd_wr = send(sock_conn, perm_err_response,
				     strlen(perm_err_response), 0);
		if (rd_wr == -1) { close(sock_conn); return SOCK_SEND_ERR; }
		close(sock_conn);
		return FAIL;
	//if file doesn't exist
	} else if (ret == REQUEST_FILE_EXIST_ERR) {
		rd_wr = send(sock_conn, file_exist_err_response,
				     strlen(file_exist_err_response), 0);
		if (rd_wr == -1) { close(sock_conn); return SOCK_SEND_ERR; }
		close(sock_conn);
		return FAIL;
	}

	//Set listener info to received request
	strcpy(rli->file, request_path);
	ret = atoi(request_id);
	if (ret == 0) return SOCK_RECV_REQ_ERR;
	
	rli->target_host_id = ret - 1;

	//Notify sender request is successful, is being processed
	rd_wr = send(sock_conn, successful_response, strlen(successful_response), 0);
	if (rd_wr == -1) { close(sock_conn); return SOCK_SEND_ERR; }

	ret = close(sock_conn);
	return SUCCESS;
}


//Initialise request struct for daemon listener
int init_req(req_info_t * ri, int target_host_id, char * file) {

	ri->target_host_id = target_host_id;
	strcpy(ri->file, file);

	return SUCCESS;
}


//Start daemon listener for requests
int init_req_listener(req_listener_info_t * rli) {

	int ret;
	char sock_path[108] = {}; //Max path of sockets, see man unix(7).

	const char * req_root = "/var/run/scarlet/";
	const char * req_sock = "sock";

	//Create socket
	rli->sock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (rli->sock == -1) return SOCK_OPEN_ERR;

	//Create path for socket
	strcat(sock_path, req_root);
	strcat(sock_path, req_sock);

	//Create address
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, sock_path);

	//Bind
	ret = bind(rli->sock, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1) { close(rli->sock); return SOCK_BIND_ERR; }

	//Change socket permissions
	ret = chmod("/var/run/scarlet/sock", 0666); //TODO TODO TODO change to 0660
	if (ret == -1) return SOCK_BIND_ERR;

	//Listen
	ret = listen(rli->sock, REQ_BACKLOG);
	if (ret == -1) { close(rli->sock); return SOCK_LISTEN_ERR; }
	
	return SUCCESS;
}


//Close daemon listener for requests
int close_req_listener(req_listener_info_t * rli) {

	int ret;
	ret = close(rli->sock);
	if (ret == -1) return SOCK_CLOSE_ERR;
	ret = remove("/var/run/scarlet/sock");
	if (ret == -1) return SOCK_CLOSE_ERR;
	return SUCCESS;
}
