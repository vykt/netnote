#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>

#include <linux/limits.h>

#include "daemon.h"
#include "request.h"
#include "error.h"

int main(int argc, char ** argv) {

	// -d, --daemon						: start daemon
	// -s, --send <filename> <peer id>	: send filename
	// -l, --list						: list peers and their ids

	int ret;
	int opt;
	int opt_index;
	int peer_id = 0;
	char file[PATH_MAX] = {};
	char resolved_path[PATH_MAX] = {};
	uid_t check_root;
	FILE * check_pid;
	req_info_t ri;

	struct option long_opts[] = {
		{"daemon", no_argument, NULL, 'd'},
		{"send", required_argument, NULL, 's'},
		{"list", no_argument, NULL, 'l'},
		{0,0,0,0}
	};

	while ((opt = getopt_long(argc, argv, "ds:l", long_opts, &opt_index)) != -1) {

		switch (opt) {

			//Daemon option
			case 'd':
				//Check running as root
				check_root = getuid();
				if (check_root) {
					printf("Please run the daemon as root.\n");
					return FAIL;
				}
				//Check if daemon is running
				check_pid = fopen("/var/run/scarlet.pid", "r");
				if (check_pid) {
					printf("/var/run/scarlet.pid present, assuming daemon already running.\n");
					return FAIL;
				}
				main_daemon();

			//Send option
			case 's':
				//int x = optind - 1;	
				if (argv[optind-1] == NULL) {
					printf("Use: scarlet -s <filename> <peer id>\n");
					return FAIL;
				}
				realpath(argv[optind-1], file);
				if (argv[optind] == NULL) {
					printf("Use: scarlet -s <filename> <peer id>\n");
					return FAIL;
				}
				peer_id = atoi(argv[optind]);

				ret = init_req(&ri, peer_id, file);
				if (ret != SUCCESS) {
					printf("An error occured. Check error log for details.\n");
					return FAIL;
				}

				ret = req_send(&ri);
				if (ret != SUCCESS) {
					printf("An error occured. Check error log for details.\n");
					return FAIL;
				}

				return SUCCESS;

			//List function
			case 'l':
				strcpy(file, "LIST");
				peer_id = atoi("0");

				ret = init_req(&ri, peer_id, file);
				if (ret != SUCCESS) {
					printf("An error occured. Check error log for details.\n");
					return FAIL;
				}

				ret = req_send(&ri);
				if (ret != SUCCESS) {
					printf("An error occured. Check error log for details.\n");
					return FAIL;
				}

				return SUCCESS;
		}
	}

	return FAIL;
}
