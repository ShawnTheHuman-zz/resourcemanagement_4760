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
#include "deadlock.h"

#define MAX_PROCESSES 100

static void usage(std::string);



using namespace std;




volatile sig_atomic_t sig_int_flag = 0;

void signal_handler(int sig)
{
	sig_int_flag = 1;
}




int main(int argc, char* argv[])
{

	int opt;
	string logfile = "logfile";
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
	printf("Usage:\t %s [-h].\n", name.c_str());
	printf("Usage:\t %s [-v].\n\n", name.c_str());
	printf(" -h  help for invocation and termination\n-v for verbose mode\n\n\n");


}


int oss(string filename, bool verbose_mode){

	struct SysInfo* sys_info;
	struct UserProcesses* user_processes;
	struct ResourceDescriptors* res_desc;

	int wstatus;
	long next_start_time = 0;


	queue<int> ready_queue;
	list<int> blocked_list;


	const pid_t pid = getpid();


	srand( time(0) ^ pid );


	time_t sec_start = time(NULL);

	int deadlock_timer = 1;

	struct tm * cur_time = localtime( &sec_start );

	if(VerboseMode)
		print_log("Verbose Mode: ON", logfile);
	else
		print_log("Verbose Mode: OFF", logfile);	


	bitvector bv(PROCESSES_MAX);

	signal(SIGINT, signal_handler);

	bool killed = false;
	bool shutdown = false;

	int process_count = 0;
	int total_time = 0;
	int count_requested = 0;
	int count_allocated = 0;
	int count_released = 0;
	int count_wait = 0;
	int count_deadlocked = 0;
	int count_deadlock_runs = 0;
	int count_die_nat = 0;


	semaphore s(key_mutex, true, 1)

	if(!s.in_init())
	{
		perror("ERROR: OSS: unable to create semaphore");
		exit(EXIT_FAILURE);
	}

	int msgid = (key_message_queue, IPC_CREAT | 0666);
	if (msgid == -1)
	{
		perror("ERROR: OSS: unable to create message");
		exit(EXIT_FAILURE);
	}


}
