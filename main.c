#include <stdio.h>
#include <unistd.h>
#include <getopt.h>


int main(int argc, char ** argv) {

	// -d, --daemon						: start daemon
	// -s, --send <filename> <peer id>	: send filename
	// -l, --list						: list peers and their ids

	int opt;
	int opt_index;
	struct option long_opts[] = {
		{"daemon", no_argument, NULL, 'd'},
		{"send", required_argument, NULL, 's'},
		{"list", no_argument, NULL, 'l'},
		{0,0,0,0}
	};

	while ((opt = getopt_long(argc, argv, "ds:l", long_opts, &opt_index)) != -1) {

		switch (opt) {

			case 'd':
				printf("daemon called\n");
				break;
			case 's':
				printf("send called\n");
				break;
			case 'l':
				printf("list called\n");
				break;


		}

	}




	//Parse command line options
	

}
