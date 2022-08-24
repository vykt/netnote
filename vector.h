#ifndef VECTOR_H
#define VECTOR_H

#include <sys/types.h>

#include "error.h"

#define VECTOR_APPEND_TRUE 1
#define VECTOR_APPEND_FALSE 0


typedef struct vector vector_t;

struct vector {

	char * vector;
	size_t data_size;
	unsigned long length;

};


int vector_set(vector_t * v, unsigned long pos, char * data);
int vector_add(vector_t * v, unsigned long pos, char * data, unsigned short append);
int vector_rmv(vector_t * v, unsigned long pos);
int vector_get(vector_t * v, unsigned long pos, char * data);
int vector_get_ref(vector_t * v, unsigned long pos, char ** data);
int vector_get_pos_by_dat(vector_t * v, char * data, unsigned long * pos);
int vector_mov(vector_t * v, unsigned long pos, unsigned long pos_new);

int vector_ini(vector_t * v, size_t data_size);
int vector_end(vector_t * v);


#endif
