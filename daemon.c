#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "daemon.h"
#include "error.h"


int main_daemon() {


	return SUCCESS;
}


int init_daemon() {

	int ret;
	pid_t proc_id;

	//Fork process
	//proc_id = fork();
	//if (proc_id == -1) { return DAEMON_FORK_ERR; }

	//Return success if parent
	//if (proc_id > 0) {
	//	return SUCCESS;
	//}

	//Unmask file mode
	umask(0);

	//Change working directory to root of filesystem
	chdir("/");

	//Close standard input/output streams
	//close(STDIN_FILENO);
	//close(STDOUT_FILENO);
	//close(STDERR_FILENO);

	//TODO perhaps open logging file here?
	
	//Create unix socket for communicating with client
	ret = mkdir("/var/run/scarlet", 0755);
	if (ret == -1 && errno != EEXIST) printf("errno mkdir: %d\n", errno);

	//If present due to improper exit, remove previous socket
	ret = remove("/var/run/scarlet/sock");
	if (ret == -1 && errno != ENOENT) return DAEMON_UN_SOCK_ERR;

	//TODO call another function here

	return SUCCESS;
}


int close_daemon() {

	//Close socket
	ret = remove("/var/run/scarlet/sock");
	if (ret == -1) exit(CRITICAL_ERR);
	exit(SUCCESS);
	
}
