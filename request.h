#ifndef REQUEST_H
#define REQUEST_H

#include <linux/limits.h>


/*
 *	REQUESTS:
 *
 *		-g --get-peers (0)				: Show connected peers
 *		-s --send <filepath> <peer id>	: Send file at <filepath> to <peer id>
 *		
 */

#define REQ_BACKLOG 2
#define REQ_REPLY_SIZE 4096

typedef struct req_listener_info req_listener_info_t;
typedef struct req_info req_info_t;


struct req_listener_info {

	int sock;

};

struct req_info {

	int sock;
	int target_host_id; //-1 = give all hosts
	char file[PATH_MAX];
	char reply[REQ_REPLY_SIZE];
};


#endif
