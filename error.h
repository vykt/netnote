#ifndef ERROR_H
#define ERROR_H


#define SUCCESS 0
#define FAIL 1

//vector.h
#define FULL_ERR 2
#define EMPTY_ERR 3
#define OUT_OF_BOUNDS_ERR 4
#define MEM_ERR 5
#define NULL_ERR 6

//net_udp.h, net_tcp.h
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

#define SOCK_EXIT_UNKNOWN_ERR 17

//conn_udp.h - special cases
#define SOCK_RECV_EXIT_ERR 18

//conn_transfer.h
#define FILE_OPEN_ERR 19
#define FILE_STAT_ERR 20
#define FILE_MMAP_ERR 21
#define FILE_WRITE_ERR 22

//config.h
#define CONF_ADDR_ERR 23

//critical errors
#define CRITICAL_ERR 24

#endif
