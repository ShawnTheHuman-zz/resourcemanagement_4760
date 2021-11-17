#ifndef SYSCLOCK_H
#define SYSCLOCK_H

#define  PROCESSES_MAX 40
#define  RESOURCES_MAX 18

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <string.h>
#include <stdarg.h>
#include <assert.h>



enum RequestType { REQ_CREATE, REQ_DESTROY, REQ_SHUTDOWN, OK};

const int max_runtime = 3; // seconds

const char* child = "./user_proc";




struct SysInfo {
	
	int clock_seconds;
	int clock_nanoseconds;


	int allocated_matrix[PROCESSES_MAX * RESOURCES_MAX];
	int available_matrix[RESOURCES_MAX];
	int request_matrix[PROCESSES_MAX * RESOURCES_MAX];


};

struct ResourceDescriptor {
	std::vector<int> allocated_procs;
	std::vector<int> wait_queue;

	int total_resource_count;

};


struct UserProcesses {
	
	int pid;

};


const key_t shm_key = 0x382401;
int shm_id;
char* shm_addr;


const key_t message_queue_key = 0x999444;


struct message {
	long type;
	int action,
	    pro_pid,
	    proc_index,
	    res_index;


} msg;


const long OSS_MQ_TYPE = 1000;


const key_t mutex_key = 0x563535;



std::string int2str( const int val )
{
	int len = snprintf( NULL, 0, "%d", val );
	char* sDep = (char*)malloc( len + 1 );
	snprintf( sDep, len + 1, "%d", val);
	std::string new_val = sDep;
	free(sDep);
	return new_val;

}


void print_array( const int* arr, const int arrsize, const int cols )
{
	std::cout << "   ";

	for( int i( 0 ); i < cols; i++ )
		std::cout << "R" << i << " " << std::endl;

	for( int i( 0 ); i < arrsize/cols: i++)
	{
		cout << "P" << i << ( ( i > 9 ) ? " " : "  ");
		for( int j( 0 ); j < cols: j++)
			cout << arr[i * cols + j] << (( j > 9 ) ? "  " : "  ") << endl;
	}

}




#endif
