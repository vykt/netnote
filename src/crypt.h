#ifndef CRYPT_H
#define CRYPT_H

#include "net_tcp.h"

void init_crypt(char * options_arr, conn_info_t * ci);
unsigned char get_rand(conn_info_t * ci);
void cipher_byte(char * byte, conn_info_t * ci);
void cipher_buffer(char * buffer, int buffer_len, conn_info_t * ci);

#endif
