#ifndef ERROR_H
#define ERROR_H


//RETURNS
#define SUCCESS 0
#define FAIL 1

//vector.h
#define FULL_ERR 2
#define EMPTY_ERR 3
#define OUT_OF_BOUNDS_ERR 4
#define MEM_ERR 5
#define NULL_ERR 6

//net_udp.h, net_tcp.h, request.h
#define SOCK_OPEN_ERR 7
#define SOCK_CLOSE_ERR 8
#define SOCK_BIND_ERR 9
#define SOCK_LISTEN_ERR 10
#define SOCK_CONNECT_ERR 11
#define SOCK_OPT_ERR 12

#define SOCK_SEND_ERR 13
#define SOCK_SEND_NAME_ERR 14
#define SOCK_RECV_ERR 15
#define SOCK_RECV_NAME_ERR 16
#define SOCK_RECV_REQ_ERR 17

#define SOCK_CRED_ERR 18
#define SOCK_EXIT_UNKNOWN_ERR 19

//conn_udp.h - special cases
#define SOCK_RECV_EXIT_ERR 20

//conn_transfer.h
#define FILE_OPEN_ERR 21
#define FILE_STAT_ERR 22
#define FILE_MMAP_ERR 23
#define FILE_WRITE_ERR 24

//request.h
#define REQUEST_PERM_ERR 25
#define REQUEST_FILE_EXIST_ERR 26
#define REQUEST_CHGRP_ERR 27
#define REQUEST_GENERIC_ERR 28
#define REQUEST_LIST 29

//config.h
#define CONF_ADDR_ERR 30
#define CONF_READ_ERR 31
#define CONF_FORMAT_ERR 32
#define CONF_INCOMPLETE_ERR 33

//daemon.h, request.h
#define DAEMON_FORK_ERR 34
#define DAEMON_HANDLER_ERR 35
#define DAEMON_UN_SOCK_ERR 36
#define DAEMON_NAME_ERR 37
#define DAEMON_PID_WRITE_ERR 38
#define DAEMON_POLL_RMV_ERR 39
#define DAEMON_CLEANUP_ERR 40
#define DAEMON_GROUP_ERR 41
#define DAEMON_TERM_REQ 42

//log.h
#define LOG_PATH_ERR 43
#define LOG_ERR_ERR 44
#define LOG_ACT_ERR 45

//util.h
#define UTIL_OPENDIR_ERR 46

//critical errors
#define CRITICAL_ERR 47


//EXITS
#define EXIT_NORMAL 0
#define EXIT_ERR 1
#define EXIT_SIGTERM_NORMAL 0
#define EXIT_SIGTERM_ERR 1


#endif
