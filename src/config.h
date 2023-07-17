#ifndef CONFIG_H
#define CONFIG_H

#define CONF_OPTION_NUM 5
#define CONF_OPTION_SIZE 32 //Max length of option key.

#define M_ADDR 0
#define UDP_PORT 1
#define TCP_PORT 2
#define DL_PATH 3
#define STREAM_KEY 4

int config_read(char * conf_path, char * options_arr);


#endif
