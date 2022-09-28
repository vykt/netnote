#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/limits.h>

#include "log.h"
#include "config.h"
#include "daemon.h"
#include "request.h"
#include "net_tcp.h"
#include "net_udp.h"
#include "vector.h"
#include "error.h"


//Respond to SIGTERM
void term_handler(int signum) {

	int ret;

	//Remove pid file
	ret = remove("/var/run/scarlet.pid");
	if (ret == -1) exit(EXIT_SIGTERM_ERR);
	
	//Remove sock
	ret = remove("/var/run/scarlet/sock");
	if (ret == -1) exit(EXIT_SIGTERM_ERR);

	return EXIT_SIGTERM_NORMAL;
}


/*
 *	This is the main function of the daemon.
 */
void main_daemon() {

	int ret;
	
	//Networking data
	send_ping_info_t si;
	recv_ping_info_t ri;
	struct sockaddr_in6 send_addr = {AF_INET6, htons(port_tcp_};
	struct sockaddr_in6 recv_addr;
	vector_t pings;
	vector_t conns;

	//Poll data
	struct pollfd poll_fds[CONN_MAX] = {};
	int poll_fds_availability[CONN_MAX] = {ARR_AVAILABLE};
	unsigned short poll_fds_count = 0;

	//Config data
	char * config_path = "opts.conf" //TODO change to /etc/scarlet.conf
	char * options_arr;	


	//Daemon initialiser
	ret = init_daemon();
	if (ret != SUCCESS) {
		log_err(2);
		exit(EXIT_ERR);
	}
	ret = log_act(0, NULL, NULL);

	//Config initialiser
	*options_arr = malloc((PATH_MAX + CONF_OPTION_SIZE) * CONF_OPTION_NUM);
	ret = config_read(config_path, options_arr); //Access: options_arr+(n * PATH_MAX)


	/*
	 *
	 *	MAIN LOGIC HERE
	 *
	 */

	//Config cleanup
	free(options_arr);


	return SUCCESS;
}


//Fork & get adopted by init. Close stdio, write PID & clean up possible loose files.
int init_daemon() {

	int ret;
	int fd;
	pid_t proc_id;
	struct sigaction action;

	//Fork process
	//proc_id = fork();
	//if (proc_id == -1) { return DAEMON_FORK_ERR; }

	//Exit if parent
	//if (proc_id > 0) {
	//	exit(EXIT_NORMAL);
	//}

	//Unmask file mode
	umask(0);

	//Change working directory to root of filesystem
	chdir("/");

	//Close standard input/output streams
	//close(STDIN_FILENO);
	//close(STDOUT_FILENO);
	//close(STDERR_FILENO);
	
	//Register term_handler as handler for SIGTERM
	memset(&action, 0, sizeof(action));
	action.sa_handler = term_handler;
	ret = sigaction(SIGTERM, &action, NULL);
	if (ret == -1) return DAEMON_HANDLER_ERR;

	//Remove previous PID if present
	ret = remove("/var/run/scarlet.pid");
	if (ret == -1 && errno != ENOENT) return DAEMON_PID_WRITE_ERR;

	//Write PID to /var/run/scarlet.pid
	fd = open("/var/run/scarlet.pid", O_WRONLY | O_CREAT);
	if (fd == -1) return DAEMON_PID_WRITE_ERR;

	ret = chmod("/var/run/scarlet.pid", 0644);
	if (ret == -1) return DAEMON_PID_WRITE_ERR;

	proc_id = getpid();
	dprintf(fd, "%d\n", proc_id);
	close(fd);

	//Create unix socket for communicating with client
	ret = mkdir("/var/run/scarlet", 0755);
	if (ret == -1 && errno != EEXIST) return DAEMON_UN_SOCK_ERR;

	//If present due to improper exit, remove previous socket
	ret = remove("/var/run/scarlet/sock");
	if (ret == -1 && errno != ENOENT) return DAEMON_UN_SOCK_ERR;

	return SUCCESS;
}
