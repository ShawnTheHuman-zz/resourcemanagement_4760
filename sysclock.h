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

using namespace std;

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


std::string int2str( const int val );
void print_array( const int* arr, const int arrsize, const int cols );
std::string array_string( const int* arr, const int arrsize, const int cols );
int get_array_value(const int* array, const int row, const int col, const int total_cols);
int get_array_value(const int* array, const int row, const int col, const int total_cols);
void set_array_value( int* arr, const int row, const int col, const int total_cols, int new_val);
std::string print_time(const char* str);
std::string string_format(const std::string fmt, ...);
void write_log(std::string& log, std::string filename);
void write_log( std::string sys, int sec, int nano, std::string text, int pid, int index, std::string filename );

#endif
