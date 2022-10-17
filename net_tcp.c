#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <linux/limits.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "net_tcp.h"
#include "vector.h"
#include "error.h"

//debug
#include <stdio.h>

/*
 *  conn_initiate() - Called when sending file to specified address.
 *
 *  conn_listener() - Periodically called to accept incoming files.
 */


int conn_initiate(vector_t * conns, struct sockaddr_in6 addr, char * file) {

	int ret;
	int sock_conn;
	ssize_t rd_wr;
	ssize_t rd_wr_total = 0;
	char * filename;
	//char filename_buf[NAME_MAX+1] = {};
	conn_info_t ci;

	//Create socket
	sock_conn = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (sock_conn == -1) return SOCK_OPEN_ERR;

	//Try to connect to conn
	ret = connect(sock_conn, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1 && errno != EINPROGRESS) { close(sock_conn); return SOCK_CONNECT_ERR; }

	//Build conn_info
	ci.sock = sock_conn;
	ci.status = CONN_STAT_SEND_INPROG;
	
	ci.mmap_addr = NULL;
	ci.mmap_size = 0;
	ci.mmap_prog = 0;
	ci.mmap_iter = 0;

	//Open file for sending
	ci.fd = open(file, O_RDONLY);
	if (ci.fd == -1) { close(ci.sock); return FILE_OPEN_ERR; }

	//Get file stat
	ret = fstat(ci.fd, &ci.f_stat);
	if (ret == -1) { close(ci.sock); return FILE_STAT_ERR; }

	//Send filename
	filename = strrchr(file, '/') + 1;
	//strcpy(filename_buf, file);
	//strcat(filename, "/");
	while (1) {
		rd_wr = send(ci.sock, filename+rd_wr_total,
				     strlen(filename)-rd_wr_total, 0);
		//rd_wr = send(ci.sock, filename_buf+rd_wr_total,
		//		     strlen(filename_buf)-rd_wr_total, 0);

		if (rd_wr == -1 && errno == EAGAIN ) { continue; }
		if (rd_wr == -1 ) { close(ci.sock); return SOCK_SEND_NAME_ERR; }
		rd_wr_total = rd_wr_total + rd_wr;
		break;
	}

	//Add to vector of ongoing connections
	ret = vector_add(conns, 0, (char *) &ci, VECTOR_APPEND_TRUE);
	if (ret != SUCCESS) return ret;

	return SUCCESS;	
}


int conn_listener(vector_t * conns, conn_listener_info_t cli, char * dir) {

	int ret;
	int sock_conn;
	ssize_t rd_wr;
	ssize_t rd_wr_total = 0;
	char recv_buf[NAME_MAX+1] = {};
	char recv_buf_total[4096+NAME_MAX+1] = {};
	char filename[NAME_MAX] = {};
	int filename_end;
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

	//Set socket of connection to not block
	ret = fcntl(sock_conn, F_SETFL, fcntl(sock_conn, F_GETFL, 0) | O_NONBLOCK);
	if (ret == -1) { close(sock_conn); return SOCK_OPT_ERR; }

	//Initialise conn
	ret = vector_add(conns, 0, NULL, VECTOR_APPEND_TRUE);
	if (ret != SUCCESS) return ret;
	ret = vector_get_ref(conns, conns->length - 1, (char **) &ci);
	if (ret != SUCCESS) return ret;
	ci->sock = sock_conn;
	ci->status = CONN_STAT_RECV_INPROG;

	//Wait to receive filename
	int recv_inprog = 1;
	while (recv_inprog) {
		
		rd_wr = recv(ci->sock, recv_buf, NAME_MAX+1, 0);
		
		//For every received character
		for (int i = 0; i < rd_wr; i++) {
			
			//If filename end found
			if (recv_buf[i] == '/') {

				recv_buf[i] = '\0';

				//if name length exceeds NAME_MAX
				if ((strlen(recv_buf_total) + i) > NAME_MAX) {
					close(ci->sock);
					ret = vector_rmv(conns, conns->length - 1);
					if (ret != SUCCESS) return CRITICAL_ERR;
					return SOCK_RECV_NAME_ERR;
				}

				strcat(filename, recv_buf_total);
				strncat(filename, recv_buf, i);
				filename_end = strlen(recv_buf_total) + i;
				recv_inprog = 0;
				break;

			} //End if filename end found
		} //End for every received character

		rd_wr_total = rd_wr_total + rd_wr;
		if (rd_wr_total <= NAME_MAX) {
			strcat(recv_buf_total, recv_buf);
		} else {
			close(ci->sock);
			ret = vector_rmv(conns, conns->length - 1);
			if (ret != SUCCESS) return CRITICAL_ERR;
			return SOCK_RECV_NAME_ERR;
		}
	} //End wait to receive filename

	//Now, create file at /dir/filename
	strcat(dir, filename);
	ci->fd = open(dir, O_WRONLY | O_CREAT, 0644);
	if (ci->fd == -1) { 
		close(ci->sock);
		ret = vector_rmv(conns, conns->length - 1);
		if (ret != SUCCESS) return CRITICAL_ERR;
		return FILE_OPEN_ERR;
	}

	//Finally, write the remainder of received buffer into the file if needed.
	int left_bytes = strlen(recv_buf_total) - filename_end - 1;
	if (left_bytes) {
		rd_wr_total = 0;
		while(1) {
			rd_wr = write(ci->fd, recv_buf_total+filename_end+1,
						strlen(recv_buf_total)-filename_end-1);
			if (rd_wr == -1) {
				close(ci->sock);
				ret = vector_rmv(conns, conns->length - 1);
				if (ret != SUCCESS) return CRITICAL_ERR;
				return FILE_WRITE_ERR;
			}
			rd_wr_total = rd_wr_total + rd_wr;
			if (rd_wr_total >= left_bytes) break;
		}
	} //End write remainder of received bytes

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
