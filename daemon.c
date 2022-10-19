#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
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


//Cleanup files if error encountered before entering main loop
void early_term() {
	
	remove("/var/run/scarlet.pid");
	remove("/var/run/scarlet/sock");
	log_act(STOP_ACT, NULL, NULL);
	exit(EXIT_ERR);
}


//Respond to SIGTERM
void term_handler(int signum) {

	terminate = 1;
}


//Called on unexpected exit to attempt some cleanup.
int term(send_ping_info_t * si) {

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
	char err_buf[16] = {0};
	int port_buf;
	
	//Networking data
	send_ping_info_t si;
	recv_ping_info_t ri;
	conn_listener_info_t cli;
	conn_info_t * vci;
	addr_ping_info_t pi;
	addr_ping_info_t * ppi = &pi;
	vector_t pings;
	vector_t conns;

	//Poll data
	struct pollfd poll_fds[CONN_MAX] = {};
	unsigned short poll_fds_count = 0;

	//Config data
	char * config_path = "/home/vykt/programming/scarlet/opts.conf"; //TODO change to /etc/scarlet.conf
	char * options_arr;

	//Request data
	req_listener_info_t rli;
	req_cred_t rc;


	//Daemon initialiser
	ret = init_daemon();
	if (ret != SUCCESS) {
		log_err(CRIT_ERR_LOG, NULL, NULL);
		early_term();
	}
	ret = log_act(0, NULL, NULL);

	//Config initialiser
	options_arr = malloc((PATH_MAX + CONF_OPTION_SIZE) * CONF_OPTION_NUM);
	ret = config_read(config_path, options_arr); //Access: options_arr+(n * PATH_MAX)
	if (ret != SUCCESS) {
		log_err(CONF_ERR_LOG, NULL, NULL);
		early_term();
	}

	//Request initialiser
	ret = init_req_listener(&rli);
	if (ret != SUCCESS) {
		log_err(UNIX_ERR_LOG, NULL, NULL);
		early_term();
	}

	//Vector initialiser
	ret = vector_ini(&pings, sizeof(addr_ping_info_t));
	if (ret != SUCCESS) {
		log_err(VECTOR_ERR_LOG, NULL, NULL);
		early_term();
	}
	ret = vector_ini(&conns, sizeof(conn_info_t));
	if (ret != SUCCESS) {
		log_err(VECTOR_ERR_LOG, NULL, NULL);
		early_term();
	}

	//Network initialiser
	ret = init_send_ping_info(&si, (options_arr+(M_ADDR*PATH_MAX)), 
							  atoi((options_arr+(UDP_PORT*PATH_MAX))));
	if (ret != SUCCESS) {
		log_err(CRIT_ERR_LOG, NULL, NULL);
		early_term();
	}
	ret = init_recv_ping_info(&ri, (options_arr+(0*PATH_MAX)), 
							  atoi((options_arr+(UDP_PORT*PATH_MAX))));
	if (ret != SUCCESS) {
		log_err(CRIT_ERR_LOG, NULL, NULL);
		early_term();
	}
	ret = init_conn_listener_info(&cli, atoi((options_arr+(TCP_PORT*PATH_MAX))));
	if (ret != SUCCESS) {
		log_err(CRIT_ERR_LOG, NULL, NULL);
		early_term();
	}
	
	//Polling initialiser
	/*	poll_fds[UDP_LISTENER] = udp listener, POLLIN, index 0
	 *	poll_fds[TCP_LISTENER] = tcp listener, POLLIN, index 1
	 */
	poll_fds[UDP_LISTENER].fd = ri.sock;
	poll_fds[TCP_LISTENER].fd = cli.sock;
	poll_fds[REQ_LISTENER].fd = rli.sock;
	poll_fds[UDP_LISTENER].events = POLLIN;
	poll_fds[TCP_LISTENER].events = POLLIN;
	poll_fds[REQ_LISTENER].events = POLLIN;
	poll_fds_count += 3;

	/*
	 *
	 *	MAIN LOGIC HERE
	 *
	 */

	while (!terminate) {

		//Get sockets awaiting action
		ret = poll(poll_fds, poll_fds_count, POLL_TIMEOUT * 100);
		if (ret == -1) {
			log_err(CRIT_ERR_LOG, NULL, NULL);
			terminate = 1;
			printf("Poll exit.\n");
		}
		
		//Ping listener
		if (poll_fds[UDP_LISTENER].revents & POLLIN) {
			ret = recv_ping(&pings, &ri);
			if (ret != SUCCESS && ret != FAIL) {
				log_err(UDP_ERR_LOG, NULL, NULL);
				terminate = 1;
				printf("Recv ping exit.\n");
			}
			printf("Ping received, current number of unique hosts: %lu\n", pings.length);
		}

		//Connection listener
		if (poll_fds[TCP_LISTENER].revents & POLLIN) {
			ret = conn_listener(&conns, cli, (options_arr+(DL_PATH*PATH_MAX)));
			if (ret != SUCCESS && ret != CRITICAL_ERR) {
				log_err(TCP_ERR_LOG, "<connecting>", NULL);
				terminate = 1;
				printf("Conn listener exit.\n");
			} else if (ret == CRITICAL_ERR) {
				log_err(CRIT_ERR_LOG, NULL, NULL);
				terminate = 1;
				printf("Conn listener critical exit.\n");
			}

			ret = vector_get_ref(&conns, conns.length-1, (char **) &vci);
			if (ret != SUCCESS) {
				printf("VECTOR ERROR 1\n");
				log_err(VECTOR_ERR_LOG, NULL, NULL);
				terminate = 1;
				printf("Conn listener get vector ref exit.\n");
			}

			poll_fds[poll_fds_count].fd = vci->sock;
			poll_fds[poll_fds_count].events = POLLIN;
			++poll_fds_count;

		} //End connection listener
		
		//Every ongoing connection (send & recv)
		for (int i = 3; i < poll_fds_count; i++) {

			//Network read
			if (poll_fds[i].revents & POLLIN) {

				ret = vector_get_ref(&conns, i-3, (char **) &vci);
				if (ret != SUCCESS) {
					printf("VECTOR ERROR 2\n");
					log_err(VECTOR_ERR_LOG, NULL, NULL);
					terminate = 1;
					printf("Network read get vector ref exit.\n");
				}

				ret = conn_recv(vci);
				if (ret != SUCCESS) {
					sprintf(err_buf, "%d", i);
					log_err(TCP_ERR_LOG, err_buf, NULL);
					terminate = 1;
					printf("Network read conn recv exit.\n");
					//TODO in future, if fail to receive, handle it gracefully
					//instead of quitting.
				}

				if (vci->status == CONN_STAT_RECV_COMPLETE) {

					ret = vector_rmv(&conns, i-3);
					if (ret != SUCCESS) {
						printf("VECTOR ERROR 3\n");
						log_err(VECTOR_ERR_LOG, NULL, NULL);
						terminate = 1;
					}

					ret = poll_fds_remove(poll_fds, poll_fds_count, i);
					if (ret != SUCCESS) {
						log_err(CRIT_ERR_LOG, NULL, NULL);
						terminate = 1;
						printf("Network read poll fds remove exit.\n");
					}
					--poll_fds_count;
				}
			}

			//Network write
			if (poll_fds[i].revents & POLLOUT) {
				
				ret = vector_get_ref(&conns, i-3, (char **) &vci);
				if (ret != SUCCESS) {
					printf("VECTOR ERROR 4\n");
					log_err(VECTOR_ERR_LOG, NULL, NULL);
					terminate = 1;
					printf("Network write get vector ref exit.\n");
				}

				ret = conn_send(vci);
				if (ret != SUCCESS) {
					sprintf(err_buf, "%d", i);
					log_err(TCP_ERR_LOG, err_buf, NULL);
					terminate = 1;
					printf("Network write conn send exit.\n");
				}

				if (vci->status == CONN_STAT_SEND_COMPLETE) {
					
					ret = vector_rmv(&conns, i-3);
					if (ret != SUCCESS) {
						printf("VECTOR ERROR 5\n");
						log_err(VECTOR_ERR_LOG, NULL, NULL);
						terminate = 1;
						printf("Network write vector rmv exit.\n");
					}
					ret = poll_fds_remove(poll_fds, poll_fds_count, i);
					if (ret != SUCCESS) {
						log_err(CRIT_ERR_LOG, NULL, NULL);
						terminate = 1;
						printf("Network write poll fds remove exit.\n");
					}
					--poll_fds_count;
				}

			}
		}

		//Request listener
		if (poll_fds[REQ_LISTENER].revents & POLLIN) {
			ret = req_receive(&rli, &rc, &pings);
			if (ret != SUCCESS && ret != FAIL && ret != REQUEST_LIST) {
				log_err(UNIX_ERR_LOG, NULL, NULL);
				terminate = 1;
				printf("Req listener receive exit.\n");
			}

			//If request was unsuccessful / unauthorised
			if (ret == FAIL) {
				sprintf(err_buf, "%d", rli.target_host_id);
				log_err(REQ_ERR_LOG, err_buf, NULL);
				

			//If request was successful, initiate connection with target
			} else if (ret == SUCCESS) {
			
				ret = vector_get_ref(&pings, (unsigned long) rli.target_host_id, 
									 (char **) &ppi);
				if (ret != SUCCESS) {
					printf("VECTOR ERROR 6\n");
					log_err(VECTOR_ERR_LOG, NULL, NULL);
					terminate = 1;
					printf("Req listener get vector ref exit.\n");
				}
				port_buf = strtol(options_arr+(TCP_PORT * PATH_MAX), NULL, 10);
				ppi->addr.sin6_port = htons(port_buf);
				ret = conn_initiate(&conns, ppi->addr, rli.file);
				if (ret != SUCCESS) {
					sprintf(err_buf, "%d", rli.target_host_id);
					log_err(TCP_ERR_LOG, err_buf, NULL);
					terminate = 1; //TODO in future, handle gracefully
					printf("Req listener conn initiate exit.\n");
				}

				ret = vector_get_ref(&conns, conns.length-1, (char **) &vci);
				if (ret != SUCCESS) {
					printf("VECTOR ERROR 7\n");
					log_err(VECTOR_ERR_LOG, NULL, NULL);
					terminate = 1;
					printf("Req listener get vector ref second exit.\n");
				}

				poll_fds[poll_fds_count].fd = vci->sock;
				poll_fds[poll_fds_count].events = POLLOUT;
				++poll_fds_count;

			//If request function encountered an error
			} else if (ret != REQUEST_LIST) {
				log_err(CRIT_ERR_LOG, NULL, NULL);
				terminate = 1;
				printf("Strange and seemingly unnecessary exit?\n");

			} //End request
		}

		//Check for outdated pings
		ret = check_ping_times(&pings);
		if (ret != SUCCESS) {
			log_err(CRIT_ERR_LOG, NULL, NULL);
			terminate = 1;
			printf("Ping time exit.\n");
		}

		//Check for need to send out ping
		if (si.last_ping + PING_INTERVAL < time(NULL)) {
			ret = send_ping(&si, MSG_PING);
			if (ret != SUCCESS) {
				log_err(UDP_ERR_LOG, NULL, NULL);
				terminate = 1;
				printf("Ping sent exit.\n");
			}
			printf("Ping sent.\n");
		}
	}

	/*
	 *
	 *  END MAIN LOGIC
	 *
	 */
	
	//Request cleanup
	close(rli.sock);
	ret = term(&si);
	if (ret != SUCCESS) { free(options_arr); exit(DAEMON_CLEANUP_ERR); }

	//Config cleanup
	free(options_arr);

	log_act(STOP_ACT, NULL, NULL);
	exit(SUCCESS);
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
