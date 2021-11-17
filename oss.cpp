/*
	Shawn Brown

	oss.cpp


*/
#include <iostream>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "sysclock.h"
#include "oss.h"


static void usage(std::string);



using namespace std;




int main(int argc, char* argv[])
{

	int opt;

	bool verbose_mode = false;


	while (( opt = getopt(argc, argv, "hv" )) != -1 )
	{

		switch(opt)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;

			case 'v':
				verbose_mode = true;
				break;

			case '?':
				if(isprint( optopt ))
				{
					errno = EINVAL;
					perror("ERROR: invalid option");
				}
				else
				{
					errno = EINVAL;
					perror("ERROR: unknown character");
	
				}
				return EXIT_FAILURE;	
			default:

				perror("OSS:ERROR: illegal option");
				usage( argv[0] );
				return EXIT_FAILURE;
		}
	}
	return oss(logfile, verbose_mode);
	
}


static void usage(std::string name)
{
	printf("Usage:\t %s [-h].\n", name);
	printf("Usage:\t %s [-v].\n\n", name);
	printf(" -h  help for invocation and termination\n-v for verbose mode\n\n\n");


}


