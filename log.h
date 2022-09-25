#ifndef LOG_H
#define LOG_H


#define LOG_LINE_SIZE 128
#define LOG_ERR_NUM 7
#define LOG_ACT_NUM 7


#define VECTOR_ERR_LOG 0
#define MEMORY_ERR_LOG 1
#define CRIT_ERR_LOG 2
#define UDP_ERR_LOG 3
#define TCP_ERR_LOG 4		//Need addr
#define FILE_TX_ERR_LOG 5	//Need addr, file
#define FILE_RX_ERR_LOG 6	//Need addr, file


#define START_ACT 0
#define STOP_ACT 1
#define ERR_STOP_ACT 2
#define NEW_CONN_ACT 3		//Need addr
#define DROP_CONN_ACT 4		//Need addr
#define SEND_ACT 5			//Need addr, file
#define RECV_ACT 6			//Need addr, file


int log_err(int err_id, char * addr, char * file);
int log_act(int act_id, char * addr, char * file);


#endif
