#include <stdlib.h>
#include <stdint.h>

#include <linux/limits.h>

#include "crypt.h"
#include "config.h"
#include "net_tcp.h"
#include "error.h"


//set seed from config
void init_crypt(char * options_arr, conn_info_t * ci) {

    ci->seed = (unsigned int) atoi(options_arr+(STREAM_KEY*PATH_MAX));

}


//linear congruential generator, 8 bit
unsigned char get_rand(conn_info_t * ci) {

    const unsigned int a = 166;
    const unsigned int c = 101;
    const unsigned int m = UINT8_MAX;
    
    unsigned int num;
    num = (a * ci->seed + c) % m;

    ci->seed = num;
    return num;
}


//XOR byte with next random number
void cipher_byte(char * byte, conn_info_t * ci) {

    *byte = *byte ^ get_rand(ci);
}


//Encrypt buffer
void cipher_buffer(char * buffer, int buffer_len, conn_info_t * ci) {

    //cipher each byte
    for (int i = 0; i < buffer_len; ++i) {
        buffer[i] = buffer[i] ^ get_rand(ci);
    }
}
