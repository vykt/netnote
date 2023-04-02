#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "log.h"
#include "error.h"



int log_err(int err_id, char * id, char * file) {

	int ret;
	size_t rret;
	ssize_t rd_wr;
	int fd;
	time_t t;
	struct tm * lt;

	char log_time_buf[32] = {};
	char log_msg_buf[LOG_LINE_SIZE] = {};
	char log_buf[LOG_LINE_SIZE] = "[ERR] [";

	char * log_dir_path = "/var/log/netnoted";
	char * log_path = "/var/log/netnoted/netnoted.log";

	char err_messages[LOG_ERR_NUM][LOG_LINE_SIZE] = {
		"Internal vector error has occured.\n",
		"Internal memory error has occured.\n",
		"Daemon encountered a critical error, shutting down...\n",
		"Daemon failed to read config, shutting down...\n",
		"UNIX sock error has occured.\n",
		"UDP sock error has occured.\n",
		"TCP sock error has occured with peer with id %s.\n",
		"Request to peer with id %d did not complete successfully.\n",
		"File transmission error has occured with peer at %s while transferring file %s.\n",
		"File receiving error has occured with peer at %s while transferring file %s.\n"
	};

	//Create directory and file, if not present
	ret = mkdir(log_dir_path, 0770);
	if (ret == -1 && errno != EEXIST) return LOG_PATH_ERR;

	fd = open(log_path, O_WRONLY | O_APPEND | O_CREAT);
	if (fd == -1) return LOG_ERR_ERR;
	
	ret = chmod(log_path, 0664);
	if (ret == -1) {
		close(fd);
		return LOG_ERR_ERR;
	}

	//Get time
	t = time(NULL);
	lt = localtime(&t);
	rret = strftime(log_time_buf, sizeof(log_time_buf), "%c", lt);
	if (rret == 0) {
		close(fd);
		return LOG_ACT_ERR;
	}

	//Build log line
	strcat(log_buf, log_time_buf);
	strcat(log_buf, "] ");

	if (err_id <= 5) {
		sprintf(log_msg_buf, err_messages[err_id]);
	} else if (err_id <= 6) {
		sprintf(log_msg_buf, err_messages[err_id], id);
	} else {
		sprintf(log_msg_buf, err_messages[err_id], id, file);
	}

	strcat(log_buf, log_msg_buf);

	//Write to log
	rd_wr = dprintf(fd, "%s", log_buf);
	if (rd_wr == -1) {
		perror("uwu");
		close(fd);
		return LOG_ERR_ERR;
	}

	close(fd);
	return SUCCESS;

}


int log_act(int act_id, char * id, char * file) {

	int ret;
	size_t rret;
	ssize_t rd_wr;
	int fd;
	time_t t;
	struct tm * lt;

	char log_time_buf[32] = {};
	char log_msg_buf[LOG_LINE_SIZE] = {};
	char log_buf[LOG_LINE_SIZE] = "[ACT] [";

	char * log_dir_path = "/var/log/netnoted";
	char * log_path = "/var/log/netnoted/netnoted.log";

	char log_messages[LOG_ACT_NUM][LOG_LINE_SIZE] = {
		"Daemon started.\n",
		"Daemon stopped.\n",
		"New connection established with host at %s.\n",
		"Connection lost with host at %s.\n",
		"Finished sending file to host at %s.\n",
		"Finished receiving file from host at %s.\n",
		"Started sending file '%s' to host at %s.\n",
		"Started receiving file '%s' from host at %s.\n"
	};

	//Create directory and file, if not present
	ret = mkdir(log_dir_path, 0770);
	if (ret == -1 && errno != EEXIST) return LOG_PATH_ERR;

	fd = open(log_path, O_WRONLY | O_APPEND | O_CREAT);
	if (fd == -1) return LOG_ACT_ERR;
	
	ret = chmod(log_path, 0664);
	if (ret == -1) {
		close(fd);
		return LOG_ERR_ERR;
	}

	//Get time
	t = time(NULL);
	lt = localtime(&t);
	rret = strftime(log_time_buf, sizeof(log_time_buf), "%c", lt);
	if (rret == 0) {
		close(fd);
		return LOG_ACT_ERR;
	}

	//Build log line
	strcat(log_buf, log_time_buf);
	strcat(log_buf, "] ");

	if (act_id <= 1) {
		sprintf(log_msg_buf, log_messages[act_id]);
	} else if (act_id <= 5) {
		sprintf(log_msg_buf, log_messages[act_id], id);
	} else {
		sprintf(log_msg_buf, log_messages[act_id], file, id);
	}

	strcat(log_buf, log_msg_buf);

	//Write to log
	rd_wr = write(fd, log_buf, sizeof(log_buf));
	if (rd_wr == -1) {
		close(fd);
		return LOG_ACT_ERR;
	}

	close(fd);
	return SUCCESS;

}
