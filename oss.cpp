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
#include <list>
#include <assert.h>
#include "sysclock.h"
#include "bitvector.h"
#include "oss.h"
#include "deadlock.h"


static void usage(std::string);


using namespace std;




/* signal handler */
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

/* main simulation driver */
int oss(string logfile, bool verbose_mode){

	struct SysInfo* sys_info;
	struct UserProcesses* user_procs;
	struct ResourceDescriptors* res_des;

	int wstatus;
	long next_start_time = 0;


	queue<int> ready_queue;
	list<int> blocked_list;


	const pid_t pid = getpid();

	
	srand( time(0) ^ pid );


	time_t sec_start = time(NULL);

	int deadlock_timer = 1;

	struct tm * cur_time = localtime( &sec_start );

	if(verbose_mode)
		write_log("Verbose Mode: ON", logfile);
	else
		write_log("Verbose Mode: OFF", logfile);	

	bitvector bv(MAX_PROCESSES);

	signal(SIGINT, signal_handler);

	bool killed = false;
	bool shutdown = false;

	/* various counters for different processes and deadlock states  */
	int process_count = 0;
	int total_time = 0;
	int count_requested = 0;
	int count_allocated = 0;
	int count_released = 0;
	int count_wait = 0;
	int count_deadlocked = 0;
	int count_deadlock_runs = 0;
	int count_died_nat = 0;

	/* create semaphore */
	Semaphore s(mutex_key, true, 1);

	/* checking if semaphore is initialized */
	if(!s.is_init())
	{
		perror("ERROR: OSS: unable to create semaphore");
		exit(EXIT_FAILURE);
	}

	int msgid = (message_queue_key, IPC_CREAT | 0666);
	if (msgid == -1)
	{
		perror("ERROR: OSS: unable to create message");
		exit(EXIT_FAILURE);
	}
	
	/* get size necessary for shared memory */
	int mem_size = sizeof(struct SysInfo) + 
		      (sizeof(struct UserProcesses) * MAX_PROCESSES) + 
		      (sizeof(struct ResourceDescriptors) * MAX_RESOURCES);
			  
	shm_id = shmget( shm_key, mem_size, IPC_CREAT | IPC_EXCL | 0660 );
	if ( shm_id == -1 )
	{	
		perror("ERROR: OSS: unable to allocate memory");
		exit( EXIT_FAILURE );
	}

	
	shm_addr = ( char* )shmat( shm_id, NULL, 0 );
	if ( !shm_addr )
	{
		perror( "ERROR: OSS: error attching memory" );
		exit(EXIT_FAILURE);
	}


	sys_info = ( struct SysInfo* ) ( shm_addr );

	user_procs = ( struct UserProcesses* ) ( shm_addr + sizeof( struct SysInfo ) );

	res_des = (struct ResourceDescriptors*)(user_procs)+(sizeof( struct UserProcesses)*MAX_PROCESSES );

	int child_index = -1;

	sys_info->clock_seconds = 0;
	sys_info->clock_nanoseconds = 0;

	memset(sys_info->available_matrix, 0, sizeof(sys_info->available_matrix));
	memset(sys_info->request_matrix, 0, sizeof(sys_info->request_matrix));
	memset(sys_info->allocated_matrix, 0, sizeof(sys_info->allocated_matrix));
	

	for( int i = 0; i < MAX_RESOURCES && !shutdown; i++ )
	{
	
		if( rand_prob( 0.20f ) )
		{
			sys_info->available_matrix[i] = res_des[i].total_resource_count = get_random(2, 10);

		}
		else
		{
			sys_info->available_matrix[i] = res_des[i].total_resource_count = 1;
			
		}

	}
	
	// try{


		while(!shutdown)
		{
			s.Wait();
			sys_info->clock_nanoseconds += get_random(10, 10000);
			
			if(sys_info->clock_nanoseconds > 100000000 )
			{
				sys_info->clock_seconds += floor(sys_info->clock_nanoseconds/1000000000);
				sys_info->clock_nanoseconds -= 10000000;
			}
			s.Signal();
			
	
			if (process_count < MAX_PROCESSES && !killed && time(NULL) - sec_start < 3 )
			{
				int i( 0 );

				for( ; i < MAX_PROCESSES; i++)
				{
					if(!bv.get_bit(i))
					{
						cout << "\n New Process \n";

	
						int newpid = spawn_process(child, logfile, i);

						 user_procs[i].pid = newpid;


						bv.set_bit(i, true);
						
						process_count++;

						next_start_time += get_random(1, 500);


						s.Wait();

						write_log("printing", logfile);

						sys_info->clock_nanoseconds += get_random(1000, 500000);

						s.Signal(); 

					}
				}
			}

			if (( sig_int_flag || (time(NULL) - sec_start) > max_runtime) && !killed)
			{
				killed = true;

				for( int i = 0; i < MAX_PROCESSES; i++)
				{
					if(bv.get_bit(i))
					{
						kill( user_procs[i].pid, SIGQUIT);	
						bv.set_bit( i, false );

						count_died_nat++;

					}
				}
				if( sig_int_flag)
				{
					perror("Processes killed by ctrl-c. ");

				}
				else
				{	
					perror("Processes killed by timeout");


				}
			}
			
			int wait_pid = waitpid( -1, &wstatus, WNOHANG | WUNTRACED | WCONTINUED );


			if( wait_pid == -1 )
			{
				shutdown = true;
				continue;
			} 

			if( WIFEXITED(wstatus) && wait_pid > 0 )
			{
				for( int i = 0; i < MAX_PROCESSES; i++ )
				{
					if( user_procs[i].pid == wait_pid )
					{
						user_procs[i].pid = 0;
						bv.set_bit(i, false);

						s.Wait();

						write_log("  ", logfile);
						
						s.Signal();
						break;

					}
				}
			}
			else if (WIFSIGNALED(wstatus) && wait_pid > 0) {
				cout << wait_pid << " killed by signal " << WTERMSIG(wstatus) << endl;
			}
			else if (WIFSTOPPED(wstatus) && wait_pid > 0) {
				cout << wait_pid << " stopped by signal " << WTERMSIG(wstatus) << endl;
			}
			else if (WIFSIGNALED(wstatus) && wait_pid > 0) {
			}
			

			if(!killed)
			{
				if( msgrcv(msgid, (void *) &msg, sizeof(message), OSS_MQ_TYPE, IPC_NOWAIT) > 0 )
				{
					s.Wait();
					write_log("OSS: ",sys_info->clock_seconds, sys_info->clock_nanoseconds, " recieved message from " + int2str(msg.proc_index) + " : " + int2str(msg.action),msg.proc_pid, msg.proc_index, logfile);

					s.Signal();

					if(msg.action == REQ_SHUTDOWN)
					{
						for( int i = 0; i < MAX_RESOURCES; i++)
						{
							for(vector<int>::iterator item = res_des[i].allocated_procs.begin(); item != res_des[i].allocated_procs.end(); ++item)
							{
								if( *item == msg.proc_pid )
								{
									res_des[i].allocated_procs.erase(item);
									count_released++;
	
								}

							}

						}
						s.Wait();

						write_log("OSS: ",sys_info->clock_seconds, sys_info->clock_nanoseconds, " recieved shutdown message from " + int2str(msg.proc_index) + " : " + int2str(msg.action),msg.proc_pid, msg.proc_index, logfile); // add later
						
						s.Signal();


						msg.action = OK;
						msg.type = msg.proc_pid;
						
						int n = msgsnd( msgid, (void *) &msg, sizeof(message), IPC_NOWAIT);
					}
					
					else if( msg.action == REQ_CREATE)
					{
						count_requested++;
						
						if( res_des[msg.res_index].total_resource_count > res_des[msg.res_index].allocated_procs.size())
						{
							count_requested++;

							if( res_des[msg.res_index].total_resource_count > res_des[msg.res_index].allocated_procs.size())
							{	
								res_des[msg.res_index].allocated_procs.push_back(msg.proc_pid);
								count_allocated++;

								int new_arr = get_array_value(sys_info->allocated_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES);
								set_array_value(sys_info->allocated_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES, new_arr + 1);

								s.Wait();
								
								write_log("OSS ", sys_info->clock_seconds, 
										    sys_info->clock_nanoseconds, 
										    " Process created " + 
										    int2str(msg.proc_index) + ":" + 
										  	int2str(msg.action) + int2str(msg.proc_pid),msg.proc_id, msg.proc_index, logfile);

								
								if( verbose_mode && count_allocated%20 == 0 )
									write_log(array_string(sys_info->allocated_matrix, MAX_RESOURCES* MAX_PROCESSES, MAX_RESOURCES), logfile);


								s.Signal();
								
								msg.action = OK;
								msg.type = msg.proc_pid;

								int n = msgsnd(msgid, (void *) &msg, sizeof(message), IPC_NOWAIT);
								
							}
							else
							{
								if(verbose_mode)
								{
									s.Wait();
									
									write_log("OSS ", sys_info->clock_seconds, sys_info->clock_nanoseconds, "RESOURCE UNAVAILABLE: " + 
									int2str(msg.proc_index) + " putting process to sleep ", msg.proc_pid, msg.res_index, logfile);
				
									s.Signal();
								}
								
								res_des[msg.res_index].wait_queue.push_back(msg.proc_pid);

								count_wait++;

								int new_val = get_array_value(sys_info->request_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES);

								set_array_value(sys_info->request_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES, new_val + 1);
							}

						}
						else if(msg.action == REQ_DESTROY)
						{
							if(verbose_mode)
							{
								s.Wait();
								write_log("OSS ", sys_info->clock_seconds,
                                			sys_info->clock_nanoseconds,
                                			" Process released " +
                                        	int2str(msg.proc_index) + ":" +
                                        	 int2str(msg.action) + int2str(msg.proc_pid),msg.proc_pid, msg.proc_index, logfile);
								s.Signal();

							}
							

                                                        for(vector<int>::iterator item = res_des[msg.res_index].allocated_procs.begin(); item != res_des[msg.res_index].allocated_procs.end(); ++item)
                                                        {
                                                                if( *item == msg.proc_pid )
                                                                {
                                                                    res_des[msg.res_index].allocated_procs.erase(item);
                                                                    count_released++;

																	int new_val = get_array_value(sys_info->request_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES);

	                                                                set_array_value(sys_info->request_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES, max(new_val - 1, 0 ));

																	break;
                                                                }

                                                        }
														msg.action = OK;
                                                        msg.type = msg.proc_pid;

                                                        int n = msgsnd(msgid, (void *) &msg, sizeof(message), IPC_NOWAIT);
							
						
						}

					}
				}
				for(int i=0; i < MAX_RESOURCES; i++)
				{
					if(res_des[i].total_resource_count > res_des[i].allocated_procs.size() && res_des[i].wait_queue.size() > 0)
					{
						int waiting_proc = res_des[i].wait_queue.front();

						assert( !res_des[i].wait_queue.empty());
						res_des[i].wait_queue.erase(res_des[i].wait_queue.begin());


						res_des[i].allocated_procs.push_back(waiting_proc);
						count_allocated++;

						int index = -1;
						for(int j(0);j < MAX_PROCESSES; j++)
						{
							if(user_procs[j].pid == waiting_proc)
								index = j;
						}
						if(index > -1)
						{
							if(verbose_mode)
							{
								s.Wait();
								write_log("OSS ", sys_info->clock_seconds,
                                			sys_info->clock_nanoseconds,
                                			" Process allocated after wait  " + int2str(waiting_proc), waiting_proc, waiting_proc, logfile);
								s.Signal();
							}
							int new_val = get_array_value(sys_info->allocated_matrix, index, i, MAX_RESOURCES);

							set_array_value(sys_info->allocated_matrix, index, i, MAX_RESOURCES, new_val + 1);
							new_val = get_array_value(sys_info->request_matrix, index, i, MAX_RESOURCES);
							set_array_value(sys_info->allocated_matrix, index, i, MAX_RESOURCES, new_val - 1);
						}
						msg.action = OK;
                        msg.type = waiting_proc;
					    int n = msgsnd(msgid, (void *) &msg, sizeof(message), IPC_NOWAIT);
					}
				}
				if((time(NULL) - sec_start) > deadlock_timer)
				{
					deadlock_timer++;
					count_deadlock_runs++;

					int deadlocked = deadlock(sys_info->available_matrix, MAX_PROCESSES, MAX_RESOURCES, sys_info->request_matrix, sys_info->allocated_matrix);

					if(deadlocked > -1)
					{
						s.Wait();
						write_log("OSS ", sys_info->clock_seconds, sys_info->clock_nanoseconds, "Deadlock detected " + int2str(deadlocked) + ": terminating process", msg.proc_pid, msg.proc_index, logfile);

						s.Signal();

						count_deadlocked++;
						kill(user_procs[deadlocked].pid, SIGQUIT); // 
						bv.set_bit(deadlocked, false);
					}
				}
			}
		}
		
		// catch(...)
		// {
		// 	cout << "fatal error occured. aborting." << endl;
		// }

		total_time = sys_info->clock_seconds;


		s.Wait();
		
		write_log("OSS: releasing shared memory.", logfile);

		if(shmdt(shm_addr) == -1)
		{
			perror("OSS: ERROR: unable to detach memory");
		}
		if (shmctl(shm_id, IPC_RMID, NULL) == -1) 
		{
        	perror("OSS: ERROR: deallocating shared memory ");
    	}
		write_log(": Memory de-allocated ", logfile);

		msgctl(msgid,IPC_RMID,NULL);

		write_log("________________________________\n", logfile);
		write_log("OSS Statistics: ", logfile);
		write_log("Total Requests Granted: \t\t\t" + int2str(count_allocated), logfile);
    	write_log("Requests Granted After Wait: \t\t" + int2str(count_wait), logfile);
    	write_log("Processes Killed By Deadlock: \t\t" + int2str(count_deadlocked), logfile);
    	write_log("Processes Die Naturally:\t\t" + int2str(count_died_nat), logfile);
    	write_log("Times Deadlock Ran:\t\t\t" + int2str(count_deadlock_runs), logfile);

		if(count_allocated > 0)
		{
			string deadlock_percent = float2str((float)count_deadlocked/(float)count_allocated*100.0f);
			write_log("Average percent of deadlock:\t\t\t" + deadlock_percent, logfile);
		}

		s.Signal();
		cout << endl;


		return EXIT_SUCCESS;
	
	
	}

}

/* generate child processes */

int spawn_process(string proc, string file, int arr)
{
	pid_t pid = fork();

	if(pid < 0)
	{
		perror("OSS: ERROR: unable to fork");
		return EXIT_FAILURE;
	}

	if(pid == 0 )
	{
		if(arr < 0)
			execl(proc.c_str(), proc.c_str(), file.c_str(), "50", (char*)0);
		else
		{
			string array_string = int2str(arr);
			execl(proc.c_str(), proc.c_str(), array_string.c_str(), file.c_str(), "50", (char*)0);
		}
		exit(EXIT_SUCCESS);
	}

	else 
		return pid;

}
