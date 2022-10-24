#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>

#include "config.h"
#include "error.h"


/*
 *	This is a config reader from a the SNES emulator menu written a while back.
 *	It's a bit nasty with the pointer arithmetic for the options array but otherwise
 *	should be sufficient.
 */


int config_populate_options(char * options_arr, int options_arr_offset) {

	char * line_segment;
	if ((line_segment = strtok(NULL, "=")) == NULL) {
		return CONF_FORMAT_ERR;
	}

	if (line_segment[strlen(line_segment)-1] == '\n') {
		line_segment[strlen(line_segment)-1] = '\0';
	}

	strcpy(options_arr+(options_arr_offset*PATH_MAX), line_segment);
	return SUCCESS;
}


int config_read(char * conf_path, char * options_arr) {

	int ret;

	//Options defined here, change here if needed.
	char * options_match[CONF_OPTION_NUM] = {
		"multicast_addr",
		"shared_udp_port",
		"shared_tcp_port",
		"downloads_path"
	};

	char * conf_line = malloc((PATH_MAX + CONF_OPTION_SIZE) * sizeof(char));
	char * conf_line_segment;
	size_t conf_line_size = 0;

	FILE * fd;
	if ((fd = fopen(conf_path, "r")) == NULL) {
		free(conf_line);
		return CONF_READ_ERR;
	}

	//Reading config file
	while ((getline(&conf_line, &conf_line_size, fd)) > 0) {

		//Getting identifier value from string.
		if(conf_line[0] == '#' || conf_line[0] == '\n') continue;
		conf_line_segment = strtok(conf_line, "=");
		if (conf_line_segment == NULL) {
			free(conf_line);
			fclose(fd);
			return CONF_READ_ERR;
		}

		//Checking identifier against options_arr
		for (int i = 0; i < CONF_OPTION_NUM; ++i) {
			if (strcmp(options_match[i], conf_line_segment) == 0) {		
				//Assigning values from config to memory
				ret = config_populate_options(options_arr, i);
				if (ret != SUCCESS) {
					free(conf_line);
					fclose(fd);
					return ret;
				}
				break;
			}
		}
	}

	//TODO check all options get read
	for (int i = 0; i < CONF_OPTION_NUM; i++) {
		
		//Theoretically (lol): if option has not been read
		if (*(options_arr+(i * PATH_MAX)) == 0) {
			free(conf_line);
			fclose(fd);
			return CONF_INCOMPLETE_ERR;
		}
	}

	free(conf_line);
	fclose(fd);

	return 0;
}
