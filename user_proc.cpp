/*
    Shawn Brown

    user_proc.cpp

*/


#include <iostream>
#include <unistd.h>
#include "sysclock.h"
#include "bitvector.h"
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include "deadlock.h"
#include "semaphore.h"

using namespace std;

static void usage(std::string);


volatile sig_atomic_t sig_int_flag = 0;

void signal_handler(int sig)
{
	sig_int_flag = 1;
}

int main(int argc, char* argv[])
{

    if(argc < 3)
    {
        perror("ERROR: user_proc: invalid arguments");
        exit(EXIT_FAILURE);
    }

    const int next_proc = atoi(argv[1]); 

    string logfile = argv[2];

    const int max_time = atoi(argv[3]);

    vector<int> owned_resources;

    cout << "--- New: " << next_proc << " : " << logfile << endl;

    signal(SIGINT, signal_handler);

    const key_t mutex_key = ftok("sysclock.h",'R');
    Semaphore s(mutex_key, false);
    if(!s.is_init())
    {
        perror("ERROR: semaphore error");
        exit(EXIT_FAILURE);
    }

    const pid_t pid = getpid();

    srand(time(0) ^ pid);

    time_t sec_start = time(NULL);

    int msgid = msgget(message_queue_key, IPC_CREAT | 0666); 
    if (msgid == -1) {
        perror("user_proc: Error creating Message Queue");
        exit(EXIT_FAILURE);
    }

    struct shmid_ds shmid_ds;
    shmctl(shm_id, IPC_STAT, &shmid_ds);
    size_t size = shmid_ds.shm_segsz;

	shm_id = shmget( shm_key, size, IPC_CREAT | IPC_EXCL | 0660 );
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


	struct SysInfo* sys_info = ( struct SysInfo* ) ( shm_addr );

	struct UserProcesses* user_procs = ( struct UserProcesses* ) ( shm_addr + sizeof( struct SysInfo ) );

	struct ResourceDescriptors* res_des = ( struct ResourceDescriptors* ) ( user_procs ) + ( sizeof( struct UserProcesses ) * MAX_PROCESSES );

    s.Wait();
    write_log("USER_PROC: ", sys_info->clock_seconds, sys_info->clock_nanoseconds, " start success ", pid, next_proc, logfile);
    s.Signal();

    while(true)
    {
        bool req_resource = rand_prob((float)(max_time/100.0f));
        bool close_resource = rand_prob((float)(max_time/100.0f));
        bool shutdown = rand_prob(50.0f);

        s.Wait();
        sys_info->clock_nanoseconds += get_random(1000, 500000);
        s.Signal();

        if(sig_int_flag || (time(NULL) - sec_start > 1 && shutdown))
        {
            s.Wait();
            write_log("USER_PROC: ", sys_info->clock_seconds, sys_info->clock_nanoseconds, " shutting down ", pid, next_proc, logfile);
            s.Signal();

            msg.type = OSS_MQ_TYPE;
            msg.action = REQ_SHUTDOWN;
            msg.proc_index = next_proc;
            msg.proc_pid = pid;

            int n = msgsnd(msgid, (void *) &msg, sizeof(message), IPC_NOWAIT);

            msgrcv(msgid, (void *) &msg, sizeof(message), pid, 0); 

            return EXIT_SUCCESS;

        }

        if(req_resource)
        {
            int res = get_random(0, MAX_RESOURCES-1);

            s.Wait();
            write_log("USER_PROC: ", sys_info->clock_seconds, sys_info->clock_nanoseconds, " requesting resource " + int2str(res), pid, next_proc, logfile);
            s.Signal();

            msg.type = OSS_MQ_TYPE;
            msg.action = REQ_CREATE;
            msg.proc_index = next_proc;
            msg.proc_pid = pid;
            msg.res_index = res;

            int n = msgsnd(msgid, (void *) &msg, sizeof(message), IPC_NOWAIT);

            msgrcv(msgid, (void *) &msg, sizeof(message), pid, 0); 

            cout << "USER_PROC --- adding to owned group " << endl;
            if(msg.action == OK)
                owned_resources.push_back(res);
            continue;

        }

        if(owned_resources.size() > 0 && close_resource)
        {
            if(owned_resources.size() > 0)
            {
                int res_to_remove = get_random(0, owned_resources.size() - 1);

                s.Wait();
                write_log("USER_PROC: ", sys_info->clock_seconds, sys_info->clock_nanoseconds, " requesting resource " + int2str(res_to_remove), pid, next_proc, logfile);
                s.Signal();

                msg.type = OSS_MQ_TYPE;
                msg.action = REQ_DESTROY;
                msg.proc_index = next_proc;
                msg.proc_pid = pid;
                msg.res_index = owned_resources[res_to_remove];

                int n = msgsnd(msgid, (void *) &msg, sizeof(message), IPC_NOWAIT);

                msgrcv(msgid, (void *) &msg, sizeof(message), pid, 0); 
                
                if(msg.action == OK)
                {
                    owned_resources.erase(owned_resources.begin() + res_to_remove);
                }
            }
        }
    }
}



