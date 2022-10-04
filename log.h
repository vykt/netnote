#ifndef LOG_H
#define LOG_H


#define LOG_LINE_SIZE 128
#define LOG_ERR_NUM 10
#define LOG_ACT_NUM 7


#define VECTOR_ERR_LOG 0
#define MEMORY_ERR_LOG 1
#define CRIT_ERR_LOG 2
#define CONF_ERR_LOG 3
#define UNIX_ERR_LOG 4
#define UDP_ERR_LOG 5
#define TCP_ERR_LOG 6		//Need id
#define REQ_ERR_LOG 7		//Need id
#define FILE_TX_ERR_LOG 8	//Need id, file
#define FILE_RX_ERR_LOG 9	//Need id, file


#define START_ACT 0
#define STOP_ACT 1
#define ERR_STOP_ACT 2
#define NEW_CONN_ACT 3		//Need id
#define DROP_CONN_ACT 4		//Need id
#define SEND_ACT 5			//Need id, file
#define RECV_ACT 6			//Need id, file


int log_err(int err_id, char * id, char * file);
int log_act(int act_id, char * id, char * file);


#endif
