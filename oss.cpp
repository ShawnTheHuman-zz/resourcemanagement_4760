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

	if(VerboseMode)
		print_log("Verbose Mode: ON", logfile);
	else
		print_log("Verbose Mode: OFF", logfile);	


	bitvector bv(MAX_PROCESSES);

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
	
	int mem_size = sizeof(struct SysClock) + 
		      (sizeof(struct UserProcesses) * MAX_PROCESSES) + 
		      (sizeof(struct ResourceDescriptors) * MAX_RESOURCES);
	shmid_id = shmget( shm_key, mem_size, IPC_CREAT | IPC_EXCL | 0660 );
	if ( shm_id == -1 )
	{	
		perror("ERROR: OSS: unable to allocate memory");
		exit( EXIT_FAILURE );
	}

	
	shm_addr = ( char* )shmat( shmid, NULL, 0 );
	if ( !shm_addr )
	{
		perror( "ERROR: OSS: error attching memory" );
		exit(EXIT_FAILURE);
	}


	sys_info = ( struct SysInfo* ) ( shm_addr );

	user_procs = ( struct UserProcesses* ) ( shm_addr + sizeof( struct SysClock ) );

	res_des = ( struct ResourceDescriptors* ) ( user_procs ) + ( sizeof( struct UserProcesses ) * MAX_PROCESSES );

	int child_index = -1;

	sys_info->clock_seconds = 0;
	sys_info->clock_nanoseconds = 0;

	memset(sys_info->available_matrix, 0, sizeof(sys_info->available_matrix));
	memset(sys_info->request_matrix, 0, sizeof(sys_info->request_matrix));
	memset(sys_info->allocated_matrix, 0, sizeof(sys_info->allocated_matrix));
	

	for( int i = 0; i < MAX_RESOUR && !shutdown; i++ )
	{
	
		if( randprob( 0.20f ) )
		{
			sys_info->available_matrix[i] = res_des[i].total_resource_count = get_random(2, 10);

		}
		else
		{
			sys_info->available_matrix[i] = res_des[i].total_resource_count = 1;
			
		}

	}
	
	try
	{


		while(!shutdown)
		{
			s.Wait();
			sys_info->clock_nanosecondsseconds += get_random(10, 10000);
			
			if(sys_info->clock_nanosecondsseconds > 100000000 )
			{
				sys_info->clock_seconds += floor(sys_info->clock_nanosecondsseconds/1000000000);
				sys_info->clock_nanosecondsseconds -= 10000000;
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

						 user_processes[i].pid = newpid;


						bv.set_bit(i, true);
						
						process_count++;


						next_start_time += get_random(1, 500);


						s.Wait();

						write_log("printing", logfile);

						sys_info->clock_nanosecondsseconds += get_random(1000, 500000);

						s.Signal(); 

					}
				}
			}

			if (( sig_int_flag || (time(NULL) - sec_start) > max_seconds) && !killed)
			{
				killed = true;

				for( int i = 0; i < MAX_PROCESSES; i++)
				{
					if(bv.get_bits(i))
					{
						kill(UserProcesses[i].pid, SIGQUIT);	
						bv.set_bits( i, false );

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
			
			int wait_pid = waitpid( -1, &wstatus, WHOHANG | WUNTRACED | WCONTINUED );


			if( wait_pid == -1 )
			{
				shutdown = true;
				continue;
			} 

			if( WIFEXITED(wstatus) && wait_pid > 0 )
			{
				for( int i = 0; i < MAX_PROCESSES; i++ )
				{
					if( UserProcesses[i].pid == wait_pid )
					{
						UserProcesses[i].pid = 0;
						bv.set_bit(i, false)

						s.Wait();

						write_log("  ", logfile);
						
						s.Signal();
						break;

					}
				}
			}else if (WIFSIGNALED(wstatus) && wait_pid > 0) {
				cout << wait_pid << " killed by signal " << WTERMSIG(wstatus) << endl;
			}else if (WIFSTOPPED(wstatus) && wait_pid > 0) {
				cout << wait_pid << " stopped by signal " << WTERMSIG(wstatus) << endl;
			}else if (WIFSIGNALED(wstatus) && wait_pid > 0) {
			}
			

			if(!killed)
			{
				if( msgrcv(msgid, (void *) &msg, sizeof(message), OSS_MQ_TYPE, IPC_NOWAIT) > 0 )
				{
					s.Wait();
					log_message();

					s.Signal();

					if(msg.action == REQ_SHUTDOWN)
					{
						for( int i = 0, i < MAX_RESOURCES
	; i++)
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

						log_message(); // add later
						
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
								res_des[msg.res_index].allocated_procs.push_back(msg.proc_id);
								count_allocated++;

								int new_arr = get_array_value(sys_info->allocated_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES
			);
								set_array(sys_info->allocated_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES
			, new_arr + 1);

								s.Wait();
								
								log_message("OSS ", sys_info->clock_seconds, 
										    sys_info->clock_nanoseconds, 
										    " Process created " + 
										    (msg.proc_index).c_str() + ":" + 
										   (msg.action).c_str() + msg.proc_id, msg.proc_index, logfile);

								
								if( verbose_mode && count_allocated%20 == 0 )
									log_message(array_string(sys_info->allocated_matrix, MAX_RESOURCES
				 * MAX_PROCESSES, MAX_RESOURCES
				), logfile);


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
									
									log_message("OSS ", sys_info->clock_seconds, sys_info->clock_nanoseconds, "RESOURCE UNAVAILABLE: " 
									(msg.proc_index).c_string() + " putting process to sleep ", msg.proc_pid, msg.res_index, logfile);
				
									s.Signal();
								}
								
								res_des[msg.res_index].wait_queue.push_back(msg.proc_pid);

								count_wait++;

								int new_val = get_array(sys_info->request_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES);

								set_array_value(sys_info->request_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES, new_val + 1);
							}

						}
						else if(msg.action == REQ_DESTROY)
						{
							if(verbose_mode)
							{
								s.Wait();
								log_message("OSS ", sys_info->clock_seconds,
                                			sys_info->clock_nanoseconds,
                                			" Process released " +
                                            (msg.proc_index).c_str() + ":" +
                                            (msg.action).c_str() + msg.proc_id, msg.proc_index, logfile);
								s.Signal();

							}
                                                        for(vector<int>::iterator item = res_des[i].allocated_procs.begin(); item != res_des[i].allocated_procs.end(); ++item)
                                                        {
                                                                if( *item == msg.proc_pid )
                                                                {
                                                                    res_des[i].allocated_procs.erase(item);
                                                                    count_released++;

																	int new_val = get_array(sys_info->request_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES
												);

	                                                                set_array_value(sys_info->request_matrix, msg.proc_index, msg.res_index, MAX_RESOURCES
												, max(new_val - 1, 0 ));

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

						assert( !res_des[i].wait_queue.erase(res_des[i].wait_queue.begin())
						res_des[i].allocated_procs.push_back(waiting_proc);

						count_allocated++;

						int index = -1;
						for(int j(0);j < MAX_PROCESSES; j++)
						{
							if(user_processes[j].pid == waiting_proc)
								index = j;
						}
						if(index > -1)
						{
							if(verbose_mode)
							{
								s.Wait();
								log_message("OSS ", sys_info->clock_seconds,
                                			sys_info->clock_nanoseconds,
                                			" Process allocated after wait  " + int2str(waiting_proc), waiting_proc, waiting_proc, logfile);
								s.Signal();
							}
							int new_val = get_array(sys_info->allocated_matrix, index, i, MAX_RESOURCES);

							set_array_value(sys_info->allocated_matrix, index, i, MAX_RESOURCES, new_val + 1);
							new_val = get_array(sys_info->request_matrix, index, i, MAX_RESOURCES);
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

					int deadlocked = deadlock(sys_info->available_matrix, MAX_PROCESSES, sys_info->request_matrix, sys_info->allocated_matrix);

					if(deadlocked > -1)
					{
						s.Wait();
						log_message("OSS ", sys_info->clock_seconds, sys_info->clock_nanoseconds, "Deadlock detected " + int2str(deadlocked) + ": terminating process", msg.proc_id, msg.proc_index, logfile);

						s.Signal();

						count_deadlocked++;
						kill(user_processes[deadlocked].pid, SIGQUIT);
						bv.set_bit(deadlocked, false);
					}
				}

			}
		}
		catch(...){
			cout << "fatal error occured. aborting." << endl;
		}

		total_time = sys_info->clock_seconds;


		s.Wait();
		
		log_message("OSS: releasing shared memory.", logfile);

		if(shmdt(shm_addr) == -1)
		{
			perror("OSS: ERROR: unable to detach memory")
		}
		if (shmctl(shm_id, IPC_RMID, NULL) == -1) 
		{
        	perror("OSS: ERROR: deallocating shared memory ");
    	}
		log_message(": Memory de-allocated ", logfile);

		msgctl(msgid,IPC_RMID,NULL);

		log_message("________________________________\n", logfile);
    	log_message("OSS Statistics", logfile);
    	log_message("Total Requests Granted:\t\t\t" + int2str(count_allocated), logfile);
    	log_message("Requests Granted After Wait:\t\t" + int2str(count_wait), logfile);
    	log_message("Processes Killed By Deadlock:\t\t" + int2str(count_deadlocked), logfile);
    	log_message("Processes Die Naturally:\t\t" + int2str(count_died_nat), logfile);
    	log_message("Times Deadlock Ran:\t\t\t" + int2str(count_deadlock_runs), logfile);

		if(count_allocated > 0)
		{
			string deadlock_percent = (float)count_deadlocked/(float)count_allocated*100.0f;
			log_message("Average percent of deadlock:\t\t\t" + deadlock_percent, logfile);
		}

		s.Signal();
		cout << endl;


		return EXIT_SUCCESS;
	}
}

spawn_process(string proc, string file, int arr)
{
	pid_t pid = fork();

	if(pid < 0)
	{
		perror("OSS: ERROR: unable to fork")
		return EXIT_FAILURE;
	}

	if(pid == 0 )
	{
		if(arr < 0)
			execl(proc.c_str(), proc.c_str(), file.c_str(), "50", (char*)0);
		else
		{
			string array_string = int2str(array_string);
			execl(proc.c_str(), proc.c_str(), array_string.c_str(), file.c_str(), "50", (char*)0)
		}
		exit(EXIT_SUCCESS);
	}
	else 
		return pid;

}
