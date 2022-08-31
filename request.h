#ifndef REQUEST_H
#define REQUEST_H

#include <linux/limits.h>

typedef struct req_listener_info req_listener_info_t;
typedef struct req_info req_info_t;


struct req_listener_info {

	int sock;
	unsigned int target_host_id;
	char file[PATH_MAX];

};

struct req_info {

	int sock;

};


#endif
