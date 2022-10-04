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
#include "net_transfer.h"
#include "net_tcp.h"
#include "net_udp.h"
#include "vector.h"
#include "error.h"


//Looping condition
int terminate = 0;


//Respond to SIGTERM
int term_handler(int signum) {

	int ret;

	//Remove pid file
	ret = remove("/var/run/scarlet.pid");
	if (ret == -1) exit(EXIT_ERR);

	//Remove sock
	ret = remove("/var/run/scarlet/sock");
	if (ret == -1) exit(EXIT_ERR);

	return EXIT_SIGTERM_NORMAL;
}


//Called on unexpected exit to attempt some cleanup.
int term() {

	int ret;

	//Remove pid file
	ret = remove("/var/run/scarlet.pid");
	if (ret == -1) exit(EXIT_ERR);

	//Remove sock
	ret = remove("/var/run/scarlet/sock");
	if (ret == -1) exit(EXIT_ERR);

	return SUCCESS;
}


//Remove an entry in poll_fds to make it match contents of connection vector.
int poll_fds_remove(struct pollfd poll_fds[CONN_MAX], 
				    unsigned short poll_fds_count, int remove_index) {

	//Check for proper use, first 2 slots are reserved for listeners
	if (remove_index < POLL_ARR_MIN) return DAEMON_POLL_RMV_ERR;	

	//Move every element necessary
	for (int i = poll_fds_count - 1 - (poll_fds_count - 1 - remove_index);
		 i < poll_fds_count - 1; i++) {
		
		poll_fds[i] = poll_fds[i+1];
	}
	return SUCCESS;
}


/*
 *	This is the main function of the daemon.
 */
void main_daemon() {

	int ret;
	
	//Networking data
	send_ping_info_t si;
	recv_ping_info_t ri;
	conn_listener_info_t cli;
	conn_info_t * ci;
	conn_info_t * vci;
	addr_ping_info_t pi;
	struct sockaddr_in6 send_addr;
	struct sockaddr_in6 recv_addr;
	vector_t pings;
	vector_t conns;

	//Poll data
	struct pollfd poll_fds[CONN_MAX] = {};
	unsigned short poll_fds_count = 0;

	//Config data
	char * config_path = "opts.conf"; //TODO change to /etc/scarlet.conf
	char * options_arr;

	//Request data
	req_listener_info_t rli;
	req_cred_t rc;


	//Daemon initialiser
	ret = init_daemon();
	if (ret != SUCCESS) {
		log_err(CRIT_ERR_LOG, NULL, NULL);
		exit(EXIT_ERR);
	}
	ret = log_act(0, NULL, NULL);

	//Config initialiser
	options_arr = malloc((PATH_MAX + CONF_OPTION_SIZE) * CONF_OPTION_NUM);
	ret = config_read(config_path, options_arr); //Access: options_arr+(n * PATH_MAX)
	if (ret != SUCCESS) log_err(CONF_ERR_LOG, NULL, NULL);
	
	send_addr.sin6_family = AF_INET6;
	send_addr.sin6_port = htons(*(options_arr+(2 * PATH_MAX)));

	//Request initialiser
	ret = init_req_listener(&rli);
	if (ret != SUCCESS) log_err(UNIX_ERR_LOG, NULL, NULL);

	//Vector initialiser
	ret = vector_ini(&pings, sizeof(addr_ping_info_t));
	if (ret != SUCCESS) log_err(VECTOR_ERR_LOG, NULL, NULL);
	ret = vector_ini(&conns, sizeof(conn_info_t));
	if (ret != SUCCESS) log_err(VECTOR_ERR_LOG, NULL, NULL);

	//Network initialiser
	ret = init_send_ping_info(&si, (options_arr+(M_ADDR*PATH_MAX)), 
							  *(options_arr+(UDP_PORT*PATH_MAX)));
	if (ret != SUCCESS) log_err(CRIT_ERR_LOG, NULL, NULL);
	ret = init_recv_ping_info(&ri, (options_arr+(0*PATH_MAX)), 
							  *(options_arr+(UDP_PORT*PATH_MAX)));
	if (ret != SUCCESS) log_err(CRIT_ERR_LOG, NULL, NULL);

	ret = init_conn_listener_info(&cli, *(options_arr+(TCP_PORT*PATH_MAX)));
	if (ret != SUCCESS) log_err(CRIT_ERR_LOG, NULL, NULL);

	//Polling initialiser
	/*	poll_fds[UDP_LISTENER] = udp listener, POLLIN, index 0
	 *	poll_fds[TCP_LISTENER] = tcp listener, POLLIN, index 1
	 */
	poll_fds[UDP_LISTENER].fd = ri.sock;
	poll_fds[TCP_LISTENER].fd = cli.sock;
	poll_fds[UDP_LISTENER].events = POLLIN;
	poll_fds[TCP_LISTENER].events = POLLIN;
	poll_fds_count += 2;

	/*
	 *
	 *	MAIN LOGIC HERE
	 *
	 */

	while (!terminate) {

		//Get sockets awaiting action
		ret = poll(poll_fds, poll_fds_count, POLL_TIMEOUT * 100);
		if (ret == -1) log_err(CRIT_ERR_LOG, NULL, NULL);

		if (poll_fds[UDP_LISTENER].revents & POLLIN) {
			
			ret = recv_ping(&pings, &ri);
			if (ret != SUCCESS && ret != FAIL) log_err(UDP_ERR_LOG, NULL, NULL);

		}

		//Connection listener
		if (poll_fds[TCP_LISTENER].revents & POLLIN) {
			ret = conn_listener(&conns, cli, (options_arr+(DL_PATH*PATH_MAX)));
			if (ret != SUCCESS) log_err(TCP_ERR_LOG, "<connecting>", NULL);

			ret = vector_add(&conns, 0, NULL, VECTOR_APPEND_TRUE);
			if (ret != SUCCESS) log_err(VECTOR_ERR_LOG, NULL, NULL);

			ret = vector_get_ref(&conns, conns.length-1, (char **) &vci);
			if (ret != SUCCESS) log_err(VECTOR_ERR_LOG, NULL, NULL);

			vci->sock = ci->sock;
			vci->addr = ci->addr;

			poll_fds[poll_fds_count].fd = vci->sock;
			poll_fds[poll_fds_count].events = POLLIN;
			++poll_fds_count;

		} //End connection listener
		
		//Every ongoing connection (send & recv)
		for (int i = 3; i < poll_fds_count; i++) {

			//Network read
			if (poll_fds[i].revents & POLLIN) {

				ret = vector_get_ref(&conns, i-3, (char **) &vci);
				if (ret != SUCCESS) log_err(VECTOR_ERR_LOG, NULL, NULL);

				ret = conn_recv(vci);
				if (ret != SUCCESS) log_err(TCP_ERR_LOG, i, NULL);

				if (vci->status == CONN_STAT_RECV_COMPLETE) {

					ret = vector_rmv(&conns, i-3);
					if (ret != SUCCESS) log_err(VECTOR_ERR_LOG, NULL, NULL);

					ret = poll_fds_remove(poll_fds, poll_fds_count, i);
					if (ret != SUCCESS) log_err(CRIT_ERR_LOG, NULL, NULL);
					--poll_fds_count;
				}
			}

			//Network write
			if (poll_fds[i].revents & POLLOUT) {
				
				ret = vector_get_ref(&conns, i-3, (char **) &vci);
				if (ret != SUCCESS) log_err(VECTOR_ERR_LOG, NULL, NULL);

				ret = conn_send(vci);
				if (ret != SUCCESS) log_err(TCP_ERR_LOG, i, NULL);

				if (vci->status == CONN_STAT_SEND_COMPLETE) {
					
					ret = vector_rmv(&conns, i-3);
					if (ret != SUCCESS) log_err(VECTOR_ERR_LOG, NULL, NULL);

					ret = poll_fds_remove(poll_fds, poll_fds_count, i);
					if (ret != SUCCESS) log_err(CRIT_ERR_LOG);
					--poll_fds_count;
				}

			}
		}

		//Request listener
		if (poll_fds[REQ_LISTENER].revents & POLLIN) {
			ret = req_receive(&rli, &rc);
			if (ret != SUCCESS && ret != FAIL) log_err(UNIX_ERR_LOG);
			
			//If request was unsuccessful / unauthorised
			if (ret == FAIL) {
				log_err(REQ_ERR_LOG, rli.target_host_id, NULL);
			
			//If request was successful, initiate connection with target
			} else if (ret == SUCCESS) {
			
				ret = vector_get_ref(&pings, rli, &pi);
				if (ret != SUCCESS) log_err(VECTOR_ERR_LOG, NULL, NULL);
				ret = conn_initiate(&conns, pi.addr, rli.file);
				if (ret != SUCCESS) {
					log_err(TCP_ERR_LOG, rli.target_host_id, NULL);
				}
			//If request function encountered an error
			} else {
				log_err(CRIT_ERR_LOG, NULL, NULL);

			} //End request
		}

		//Check for outdated pings
		ret = check_ping_times(&pings);
		if (ret != SUCCESS) log_err(CRIT_ERR_LOG, NULL, NULL);

	}

	/*
	 *
	 *  END MAIN LOGIC
	 *
	 */
	
	//Request cleanup
	close(req_listener(&rli);
	ret = term();
	if (ret != SUCCESS) { free(options_arr); return DAEMON_CLEANUP_ERR; }

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
