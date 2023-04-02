#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#include <linux/limits.h>

#include "util.h"
#include "config.h"
#include "error.h"


void env_clean() {

	char * env_arr[ENV_LEN] = {
		"/var/run/netnoted/netnoted.pid",
		"/var/run/netnoted/sock"
	};

	for (int i = 0; i < ENV_LEN; ++i) {
		remove(env_arr[i]);
	}
}


int dl_clean() {

	int ret;

	char * config_path = "/etc/netnote.conf";
	char * options_arr;

	DIR * dl_dir;
	struct dirent * dl_file;
	char dl_file_path[PATH_MAX] = {0};

	//Config initialiser
	options_arr = malloc((PATH_MAX + CONF_OPTION_SIZE) * CONF_OPTION_NUM);
	if (options_arr == NULL) {
		return CRITICAL_ERR;
	}

	ret = config_read(config_path, options_arr);
	if (ret != SUCCESS) {
		return CONF_READ_ERR;
	}

	//Get handle on downloads directory
	dl_dir = opendir(options_arr+(DL_PATH * PATH_MAX));
	if (dl_dir == NULL) {
		return UTIL_OPENDIR_ERR;
	}

	//While there are files in downloads directory left to process
	while ((dl_file = readdir(dl_dir)) != NULL) {
		
		if (!strcmp(dl_file->d_name, ".") || !strcmp(dl_file->d_name, "..")) continue;

		sprintf(dl_file_path, "%s/%s", options_arr+(DL_PATH * PATH_MAX), dl_file->d_name);
		printf("file: %s\n", dl_file_path);
	}
	
	closedir(dl_dir);
	free(options_arr);

	return SUCCESS;
}
