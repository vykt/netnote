#ifndef CONFIG_H
#define CONFIG_H

#define CONF_OPTION_NUM 4
#define CONF_OPTION_SIZE 32	//Max length of option key.


int config_read(char * conf_path, char * options_arr);


#endif
