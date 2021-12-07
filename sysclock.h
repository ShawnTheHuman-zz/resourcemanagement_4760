#ifndef SYSCLOCK_H
#define SYSCLOCK_H

#define  MAX_PROCESSES 40
#define  MAX_RESOURCES 18

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
#include <sys/sem.h>
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


	int allocated_matrix[MAX_PROCESSES * MAX_RESOURCES];
	int available_matrix[MAX_RESOURCES];
	int request_matrix[MAX_PROCESSES * MAX_RESOURCES];


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
	    proc_pid,
	    proc_index,
	    res_index;


} msg;

class Semaphore
{
    private:
    
        bool _bCreator;
        int _semid;
        bool _initialized;
        struct sembuf structSemaBuf;

    public:

    Semaphore(key_t, bool, int = 1);
    ~Semaphore();

    // Check if properly setup
    bool is_init() { return _initialized; };

    // Semaphore Wait
    void Wait();

    // Semaphore Signal
    void Signal();    

};


const long OSS_MQ_TYPE = 1000;


const key_t mutex_key = 0x563535;


std::string int2str( const int val );
void print_array( const int* arr, const int arrsize, const int cols );
std::string array_string( const int* arr, const int arrsize, const int cols );
int get_array_value(const int* array, const int row, const int col, const int total_cols);
void set_array_value( int* arr, const int row, const int col, const int total_cols, int new_val);
std::string print_time(const char* str);
std::string string_format(const std::string fmt, ...);
void write_log(std::string& log, std::string filename);
void write_log( std::string sys, int sec, int nano, std::string text, int pid, int index, std::string filename );
int get_random(int MinVal, int MaxVal);
bool rand_prob(float);


#endif
