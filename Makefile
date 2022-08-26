CC=gcc
CFLAGS= -Wall -ggdb
CLEAN_TARGETS=scarlet main.o net_transfer.o net_transfer.h.gch net_tcp.o net_tcp.h.gch net_udp.o net_udp.h.gch vector.o vector.h.gch error.h.gch


scarlet: main.o net_transfer.o net_transfer.h net_tcp.o net_tcp.h net_udp.o net_udp.h vector.o vector.h error.h
	${CC} ${CFLAGS} -o scarlet main.o net_transfer.o net_transfer.h net_tcp.o net_tcp.h net_udp.o net_udp.h vector.o vector.h error.h

main.o: main.c net_transfer.h net_tcp.h net_udp.h vector.h error.h
	${CC} ${CFLAGS} -c main.c net_transfer.h net_tcp.h net_udp.h vector.h error.h

net_transfer.o: net_transfer.c net_transfer.h net_tcp.h error.h
	${CC} ${CFLAGS} -c net_transfer.c net_transfer.h net_tcp.h error.h

net_tcp.o: net_tcp.c net_tcp.h vector.h error.h
	${CC} ${CFLAGS} -c net_tcp.c net_tcp.h vector.h error.h

net_udp.o: net_udp.c net_udp.h vector.h error.h
	${CC} ${CFLAGS} -c net_udp.c net_udp.h vector.h error.h

vector.o: vector.c vector.h error.h
	${CC} ${CFLAGS} -c vector.c vector.h error.h

clean:
	rm ${CLEAN_TARGETS}
