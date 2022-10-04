#ifndef CONFIG_H
#define CONFIG_H

#define CONF_OPTION_NUM 4
#define CONF_OPTION_SIZE 32	//Max length of option key.

#define M_ADDR 0
#define UDP_PORT 1
#define TCP_PORT 2
#define DL_PATH 3


int config_read(char * conf_path, char * options_arr);


#endif
