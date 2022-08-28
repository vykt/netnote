#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/mman.h>

#include "net_transfer.h"
#include "net_tcp.h"
#include "error.h"


int conn_send(conn_info_t * ci) {

	int ret;

	char * mem_block;
	size_t mem_block_size = 0;
	
	unsigned int buf_size = 0;

	ssize_t rd_wr;
	
	//Get mem_block_size that doesn't leak memory
	if ((ci->send_count+1) * DF_BLOCK > ci->f_stat.st_size) {
		mem_block_size = ci->f_stat.st_size - (DF_BLOCK * ci->send_count);
	} else {
		mem_block_size = DF_BLOCK;
	}

	//Get memory map
	mem_block = mmap(NULL, mem_block_size, PROT_READ, MAP_SHARED,
			         ci->fd, ci->send_count * DF_BLOCK);
	if (mem_block == MAP_FAILED) { close(ci->sock); return FILE_MMAP_ERR; }

	//Iterate through buffers, sending all of mem_block a buffer at a time
	for (unsigned int i = 0; i < (mem_block_size / DF_BUF); i++) {

		//If remainder of block is smaller than a buffer
		if ((mem_block_size - DF_BUF * (i+1)) < DF_BUF) {
			buf_size = mem_block_size - DF_BUF * i;
		} else {
			buf_size = DF_BUF;
		}

		//Ensure buffer is sent, retry if socket resource unavailable
		while (1) {
			rd_wr = send(ci->sock, mem_block + (i * DF_BUF), buf_size, 0);
			if (rd_wr == -1 && errno == EAGAIN) continue;
			if (rd_wr == -1) { close(ci->sock); return SOCK_SEND_ERR; }
		}
	}

	ret = munmap(mem_block, mem_block_size);
	if (ret == -1) { close(ci->sock); return FILE_MMAP_ERR; }

	//Increment send count for next call
	ci->send_count = ci->send_count + 1;

	//Check whether end of file has been reached
	if ((ci->send_count * DF_BLOCK) >= ci->f_stat.st_size) {
		close(ci->sock);
		ci->status = CONN_STAT_SEND_COMPLETE;
	}

	return SUCCESS;
}


int conn_recv(conn_info_t * ci) {

	char * recv_buf[DF_BUF] = {};
	ssize_t rd_wr;

	for (unsigned int i = 0; i < DF_BUF_IN_BLOCK; i++) {

		rd_wr = recv(ci->sock, recv_buf, DF_BUF, 0);
		
		//Check for recv errors
		if (rd_wr == -1 && errno == EAGAIN) { i--; continue; }
		if (rd_wr == -1) { close(ci->sock); close(ci->fd); return SOCK_RECV_ERR; }
		
		//Check for end of transmission
		if (rd_wr == 0) {
			close(ci->sock);
			close(ci->fd);
			ci->status = CONN_STAT_RECV_COMPLETE;
			return SUCCESS;
		}

		//Write buffer to file
		rd_wr = write(ci->fd, recv_buf, rd_wr);
		if (rd_wr == -1) { close(ci->sock); close(ci->fd); return FILE_WRITE_ERR; }
	
	}

	return SUCCESS;

}
