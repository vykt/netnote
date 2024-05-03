#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <grp.h>
#include <errno.h>

#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>

#include "log.h"
#include "net_tcp.h"
#include "vector.h"
#include "error.h"


/*
 *  conn_initiate() - Called when sending file to specified address.
 *
 *  conn_listener() - Periodically called to accept incoming files.
 */


int conn_initiate(vector_t * conns, struct sockaddr_in6 addr, char * file) {

	int ret;
	int conn_val;
	//socklen_t conn_val_len;
	ssize_t rd_wr;
	ssize_t rd_wr_total = 0;
	//time_t conn_timeout;
	//time_t cur_time;
	char * filename;
	char filename_buf[512] = {};
	//char filename_buf[NAME_MAX+1] = {};
	conn_info_t ci;

	//Create socket
	//ci.sock = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
	ci.sock = socket(AF_INET6, SOCK_STREAM, 0);
	if (ci.sock == -1) return SOCK_OPEN_ERR;

	//Set socket to be reused
	ret = setsockopt(ci.sock, SOL_SOCKET, SO_REUSEADDR, &conn_val, sizeof(int));
	if (ret == -1) { close(ci.sock); return SOCK_OPT_ERR; }

	//Set up select for checking connect status
	//struct pollfd conn_status[1] = {};
	//conn_status[0].fd = ci.sock;
	//conn_status[0].events = POLLOUT;

	//Try to connect to conn
	ret = connect(ci.sock, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1 && errno != EINPROGRESS) { close(ci.sock); return SOCK_CONNECT_ERR; }

	//Check connection status
	/*conn_timeout = time(NULL);
	while (1) {

		//Check if connection has completed
		ret = poll(conn_status, 1, POLL_CONN_TIMEOUT);
		if (ret == -1) { close(ci.sock); return SOCK_CONNECT_ERR; }

		if (conn_status[0].revents & POLLOUT) {

			//Check the outcome of completion
			conn_val_len = sizeof(conn_val);
			ret = getsockopt(ci.sock, SOL_SOCKET, SO_ERROR, 
					         &conn_val, &conn_val_len);
			if (ret == -1) { close(ci.sock); return SOCK_CONNECT_ERR; }

			if (conn_val == 0) { break; }
			else {
				close(ci.sock);
				return SOCK_CONNECT_ERR;
			}
		}
		//If taking too long, abort
		cur_time = time(NULL);
		if (cur_time > (conn_timeout + CONN_TIMEOUT)) {
			close(ci.sock);
			return SOCK_CONNECT_ERR;
		}
	}*/

	//Build conn_info
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
	strcat(filename_buf, filename);
	strcat(filename_buf, "/");
	while (1) {
		rd_wr = send(ci.sock, filename_buf+rd_wr_total,
				     strlen(filename_buf)-rd_wr_total, 0);

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
	ssize_t rd_wr;
	ssize_t rd_wr_total = 0;
	char recv_buf[NAME_MAX+1] = {};
	char recv_buf_total[4096+NAME_MAX+1] = {};
	char id_buf[16] = {};
	char filename[NAME_MAX] = {};
	int filename_end;
	char dir_copy[PATH_MAX] = {};
	conn_info_t ci;
	socklen_t addr_len = sizeof(cli.addr);
	struct group * grp_netnote;

	//Accept incoming connection
	ci.sock = accept(cli.sock, (struct sockaddr *) &cli.addr, &addr_len);
	if (ci.sock == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		return FAIL;
	} else if (ci.sock == -1) {
		close (cli.sock);
		return SOCK_RECV_ERR;
	}

	//Set socket of connection to not block
	//ret = fcntl(ci.sock, F_SETFL, fcntl(ci.sock, F_GETFL, 0) | O_NONBLOCK);
	//if (ret == -1) { close(ci.sock); return SOCK_OPT_ERR; }
	ci.status = CONN_STAT_RECV_INPROG;

	//Wait to receive filename
	int recv_inprog = 1;
	while (recv_inprog) {
		
		rd_wr = recv(ci.sock, recv_buf, NAME_MAX+1, 0);
		
		//For every received character
		for (int i = 0; i < rd_wr; i++) {
			
			//If filename end found
			if (recv_buf[i] == '/') {

				//recv_buf[i] = '\0';

				//if name length exceeds NAME_MAX
				if ((strlen(recv_buf_total) + i) > NAME_MAX) {
					close(ci.sock);
					return FAIL;
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
			close(ci.sock);
			return FAIL;
		}
	} //End wait to receive filename

	//Now, create file at /dir/filename
	strcat(dir_copy, dir);
	strcat(dir_copy, "/");
	strcat(dir_copy, filename);
	ci.fd = open(dir_copy, O_WRONLY | O_CREAT, 0640);
	if (ci.fd == -1) { 
		close(ci.sock);
		return FILE_OPEN_ERR;
	}
	ret = ftruncate(ci.fd, 0);
	if (ret == -1) {
		close(ci.sock);
		return FILE_OPEN_ERR;
	}

	//Change group ownership
	grp_netnote = getgrnam("netnote");
	if (grp_netnote == NULL) {
		close(ci.sock);
		return DAEMON_GROUP_ERR;
	}

	ret = fchown(ci.fd, 0, grp_netnote->gr_gid);
	if (ret == -1) {
		close(ci.sock);
		return DAEMON_GROUP_ERR;
	}

	//Finally, write the remainder of received buffer into the file if needed.
	int left_bytes = strlen(recv_buf_total) - (filename_end + 1);
	if (left_bytes > 0) {
		rd_wr_total = 0;
		while(1) {
			rd_wr = write(ci.fd, recv_buf_total+filename_end+1,
						strlen(recv_buf_total)-filename_end-1);
			if (rd_wr == -1) {
				close(ci.sock);
				return FAIL;
			}
			rd_wr_total = rd_wr_total + rd_wr;
			if (rd_wr_total >= left_bytes) break;
		}
	} //End write remainder of received bytes

	//Add to vector of ongoing connections
	ret = vector_add(conns, 0, (char *) &ci, VECTOR_APPEND_TRUE);
	if (ret != SUCCESS) return CRITICAL_ERR;

	log_act(RECV_ACT, id_buf, filename);

	return SUCCESS;
}


int init_conn_listener_info(conn_listener_info_t * cli, unsigned short port) {

	int ret;
	int reuse = 1;

	//Create socket
	cli->sock = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (cli->sock == -1) return SOCK_OPEN_ERR;
	
	//Set socket to be reused
	ret = setsockopt(cli->sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	if (ret == -1) { close(cli->sock); return SOCK_OPT_ERR; }

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
