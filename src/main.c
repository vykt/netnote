#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>

#include <linux/limits.h>

#include "util.h"
#include "daemon.h"
#include "request.h"
#include "error.h"


int main(int argc, char ** argv, char ** envp) {

	// -d, --daemon						: start daemon
	// -s, --send <filename> <peer id>	: send filename
	// -l, --list						: list peers and their ids

	int ret;
	int opt;
	int opt_index;
	int peer_id = 0;
	char file[PATH_MAX] = {};
	uid_t check_root;
	FILE * check_pid;
	req_info_t ri;

	struct option long_opts[] = {
		{"list-files", no_argument, NULL, 'f'},
		{"env-clean", no_argument, NULL, 'e'},
		{"dl-clean", no_argument, NULL, 'c'},
		{"daemon", no_argument, NULL, 'd'},
		{"send", required_argument, NULL, 's'},
		{"list-peers", no_argument, NULL, 'l'},
		{"term", no_argument, NULL, 't'},
		{0,0,0,0}
	};

	while ((opt = getopt_long(argc, argv, "fecds:lt", long_opts, &opt_index)) != -1) {

		switch (opt) {

			//List files
			case 'f':
				//Check running as root
				check_root = getuid();
				if (check_root) {
					printf("Please clean downloads as root.\n");
					return FAIL;
				}
				ret = dl_action(DLDIR_TYPE_LIST);
				switch(ret) {
					case CRITICAL_ERR:
						printf("Critical error occurred.\n");
						return FAIL;
					case CONF_READ_ERR:
						printf("Config couldn't be read.\n");
						return FAIL;
					case UTIL_OPENDIR_ERR:
						printf("Couldn't open downloads directory. Are you a member of the netnote group?\n");
						return FAIL;
				}
				break;

			//Clean downloads
			case 'c':
				//Check running as root
				check_root = getuid();
				if (check_root) {
					printf("Please clean downloads as root.\n");
					return FAIL;
				}
				ret = dl_action(DLDIR_TYPE_RM);
				switch(ret) {
					case CRITICAL_ERR:
						printf("Critical error occurred.\n");
						return FAIL;
					case CONF_READ_ERR:
						printf("Config couldn't be read.\n");
						return FAIL;
					case UTIL_OPENDIR_ERR:
						printf("Couldn't open downloads directory.\n");
						return FAIL;
				}
				break;

			//Clean environment
			case 'e':
				//Check running as root
				check_root = getuid();
				if (check_root) {
					printf("Please clean environment as root.\n");
					return FAIL;
				}
				ret = env_clean();
				if (ret == UTIL_REMOVE_ERR) {
					printf("Not all files were removed. Permission misconfiguration?\n");
				}
				break;

			//Daemon option
			case 'd':
				//Check running as root
				check_root = getuid();
				if (check_root) {
					printf("Please run the daemon as root.\n");
					return FAIL;
				}
				//Check if daemon is running
				check_pid = fopen("/var/run/netnoted/netnoted.pid", "r");
				if (check_pid) {
					printf("/var/run/netnoted/netnoted.pid present, assuming daemon is already running.\n");
					return FAIL;
				}
				main_daemon();

			//Send option
			case 's':
				//int x = optind - 1;	
				if (argv[optind-1] == NULL) {
					printf("Use: netnote -s <filename> <peer id>\n");
					return FAIL;
				}
				realpath(argv[optind-1], file);
				if (argv[optind] == NULL) {
					printf("Use: netnote -s <filename> <peer id>\n");
					return FAIL;
				}
				peer_id = atoi(argv[optind]);
				if (peer_id < 0) {
					printf("Use: netnote -s <filename> <peer id>\n");
					return FAIL;
				}

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

			//List peers
			case 'l':
				strcpy(file, "LIST");
				peer_id = atoi("-1");

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

				break;

			//Send terminate request
			case 't':
				strcpy(file, "TERM");
				peer_id = atoi("-1");

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

				break;
		}
	}

	return FAIL;
}
