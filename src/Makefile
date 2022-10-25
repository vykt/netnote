CC=gcc
CFLAGS= -Wall -ggdb -D_GNU_SOURCE=1 -D_POSIX_SAVED_IDS=1
CLEAN_TARGETS=../netnote main.o log.o log.h.gch config.o config.h.gch daemon.o daemon.h.gch request.o request.h.gch net_transfer.o net_transfer.h.gch net_tcp.o net_tcp.h.gch net_udp.o net_udp.h.gch vector.o vector.h.gch error.h.gch

netnote: main.o log.o log.h config.o config.h daemon.o daemon.h request.o request.h net_transfer.o net_transfer.h net_tcp.o net_tcp.h net_udp.o net_udp.h vector.o vector.h error.h
	${CC} ${CFLAGS} -o netnote main.o log.o log.h config.o config.h daemon.o daemon.h request.o request.h net_transfer.o net_transfer.h net_tcp.o net_tcp.h net_udp.o net_udp.h vector.o vector.h error.h
	mv netnote ..

main.o: main.c log.h config.h daemon.h request.h net_transfer.h net_tcp.h net_udp.h vector.h error.h
	${CC} ${CFLAGS} -c main.c log.h config.h daemon.h request.h net_transfer.h net_tcp.h net_udp.h vector.h error.h

log.o: log.c log.h error.h
	${CC} ${CFLAGS} -c log.c log.h error.h

config.o: config.c config.h error.h
	${CC} ${CFLAGS} -c config.c config.h error.h

daemon.o: daemon.c log.h config.h daemon.h request.h net_transfer.h net_tcp.h net_udp.h vector.h error.h
	${CC} ${CFLAGS} -c log.h config.h daemon.c daemon.h request.h net_transfer.h net_tcp.h net_udp.h vector.h error.h

request.o: request.c log.h request.h vector.h error.h
	${CC} ${CFLAGS} -c request.c log.h request.h vector.h error.h

net_transfer.o: net_transfer.c log.h net_transfer.h net_tcp.h error.h
	${CC} ${CFLAGS} -c net_transfer.c log.h net_transfer.h net_tcp.h error.h

net_tcp.o: net_tcp.c log.h net_tcp.h vector.h error.h
	${CC} ${CFLAGS} -c net_tcp.c log.h net_tcp.h vector.h error.h

net_udp.o: net_udp.c log.h net_udp.h vector.h error.h
	${CC} ${CFLAGS} -c net_udp.c log.h net_udp.h vector.h error.h

vector.o: vector.c vector.h error.h
	${CC} ${CFLAGS} -c vector.c vector.h error.h

clean:
	rm ${CLEAN_TARGETS}
