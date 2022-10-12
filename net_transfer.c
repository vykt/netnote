#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/mman.h>

#include "net_transfer.h"
#include "net_tcp.h"
#include "error.h"


void set_mmap_size(conn_info_t * ci) {

	//Get ci->mmap_size that doesn't leak memory
	if (((ci->mmap_iter+1) * DF_BLOCK) > ci->f_stat.st_size) {
		ci->mmap_size = ci->f_stat.st_size - (DF_BLOCK * ci->mmap_iter);
	} else {
		ci->mmap_size = DF_BLOCK;
	}
	ci->mmap_iter = ci->mmap_iter + 1;
}


size_t get_len(conn_info_t * ci) {

	//Get len that doesn't leak memory
	size_t len;
	size_t leftover;

	leftover = ci->mmap_size - ci->mmap_prog;
	if (leftover > DF_LEN) {
		len = DF_LEN;
	} else {
		len = leftover;
	}
	return len;
}


int conn_send(conn_info_t * ci) {

	int ret;
	ssize_t rd_wr;
	size_t len;

	//Check if end of last block reached
	if (ci->mmap_size == ci->mmap_prog) {
	
		//Check for end of file
		if (ci->mmap_size > 0 && ci->mmap_size != DF_BLOCK) {
			ci->status = CONN_STAT_SEND_COMPLETE;
			ret = munmap(ci->mmap_addr, ci->mmap_size);
			close(ci->sock);
			if (ret == -1 && errno != EINVAL) { return FILE_MMAP_ERR; }
			return SUCCESS;
		}

		ret = munmap(ci->mmap_addr, ci->mmap_size);
		if (ret == -1 && errno != EINVAL) {
			close(ci->sock);
			return FILE_MMAP_ERR;
		}

		set_mmap_size(ci);
		ci->mmap_prog = 0;

		//Assign new mmap
		ci->mmap_addr = mmap(NULL, ci->mmap_size, PROT_READ,
				             MAP_SHARED, ci->fd, (ci->mmap_iter-1) * DF_BLOCK);
	}

	len = get_len(ci);
	rd_wr = send(ci->sock, ci->mmap_addr + ci->mmap_prog, len, 0);
	if (rd_wr == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
		close(ci->sock);
		return SOCK_SEND_ERR;
	}

	ci->mmap_prog = ci->mmap_prog + rd_wr;

	return SUCCESS;
}


int conn_recv(conn_info_t * ci) {

	//int ret;
	ssize_t rd_wr;
	char recv_buf[DF_LEN] = {};

	rd_wr = recv(ci->sock, recv_buf, DF_LEN, 0);
	if (rd_wr == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
		close(ci->fd); return SOCK_RECV_ERR;
	} else if (rd_wr == 0) {
		close(ci->sock);
		close(ci->fd);
		ci->status = CONN_STAT_RECV_COMPLETE;
		return SUCCESS;
	}
	
	rd_wr = write(ci->fd, recv_buf, rd_wr);
	if (rd_wr == -1) {
		close(ci->sock);
		close(ci->fd);
		return FILE_WRITE_ERR;
	}

	return SUCCESS;
}
