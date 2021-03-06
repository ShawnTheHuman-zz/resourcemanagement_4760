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
#include <sys/msg.h> 
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/sem.h>
#include <sys/stat.h>

using namespace std;




std::string int2str( const int val )
{
	int len = snprintf( NULL, 0, "%d", val );
	char* sDep = (char*)malloc( len + 1 );
	snprintf( sDep, len + 1, "%d", val);
	std::string new_val = sDep;
	free(sDep);
	return new_val;
}

std::string float2str( const float val )
{
	int len = snprintf( NULL, 0, "%f", val );
	char* sDep = (char*)malloc( len + 1 );
	snprintf( sDep, len + 1, "%0.4f", val);
	std::string new_val = sDep;
	free(sDep);
	return new_val;
}


void print_array( const int* arr, const int arrsize, const int cols )
{
	std::cout << "   ";

	for( int i( 0 ); i < cols; i++ )
		std::cout << "R" << i << " " << std::endl;

	for( int i( 0 ); i < arrsize/cols; i++)
	{
		cout << "P" << i << ( ( i > 9 ) ? " " : "  ");
		for( int j( 0 ); j < cols; j++)
			cout << arr[i * cols + j] << (( j > 9 ) ? "  " : "  ") << endl;
	}

}

std::string array_string( const int* arr, const int arrsize, const int cols )
{
	std::string output = "   ";

	for( int i( 0 ); i < cols; i++ )
		output.append("R" + int2str(i) + " \n");

	for( int i( 0 ); i < arrsize/cols; i++)
	{
		output.append("P" + int2str(i) + (( i > 9) ? " " : "   "));
		for( int j( 0 ); j < cols; j++)
			output.append(int2str(arr[i * cols + j]) + (( j > 9 ) ? "   " : "  "));
		output.append("\n");
	}
	return output;
}


int get_array_value(const int* array, const int row, const int col, const int total_cols)
{
	return array[ row * total_cols + col];

}


void set_array_value( int* arr, const int row, const int col, const int total_cols, int new_val)
{
	arr[ row * total_cols + col] = new_val;

}

std::string print_time(const char* str)
{
	time_t t;
	struct tm * _time;
	char buffer[10];

	time (&t);
	_time = localtime ( &t );


	strftime( buffer, 80, "%T", _time);

	std::string new_time = str;
	new_time.append(buffer);
	
	return new_time;
}




/* formats a string regardless of size.  */
std::string string_format(const std::string fmt, ...)
{
	int size = ((int)fmt.size()) * 2 + 50;
	std::string str;
	va_list ap;
	while (1) {    
		str.resize(size);
		va_start(ap, fmt);
		int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
		va_end(ap);
		if (n > -1 && n < size) 
		{ 
			str.resize(n);
			return str;
		}
		if (n > -1)  
			size = n + 1; 
		else
			size *= 2;
	}
	return str;
}




void write_log(std::string input, std::string filename)
{
	std::cout << input.c_str() << std::endl;


    std::ofstream logFile (filename.c_str(), 
            std::ofstream::out | std::ofstream::app);
    if (logFile.is_open())
    {
        logFile << input.c_str() << std::endl;
        logFile.close();
    }
    else
    {
        perror("Unable to write to log file");
    }
}

/* second writer for verbose mode  */
void write_log( std::string sys, int sec, int nano, std::string text, int pid, int index, std::string filename )
{
	std::cout << string_format("%s%.2d %.6d:%.10d\t%s PID %d", sys.c_str(), index, sec, nano, text.c_str(), pid) << std::endl;

	std::ofstream logfile ( filename.c_str(), std::ofstream::out | std::ofstream::app );

	if( logfile.is_open())
	{
		logfile << string_format("%s%.2d %.6d:%.10d\t%s PID %d", sys.c_str(), index, sec, nano, text.c_str(), pid) << std::endl;

		logfile.close();
	}
	else
	{
		perror("ERROR writing to file");
	}
}

int get_random(int MinVal, int MaxVal)
{
    int range = MaxVal-MinVal+1 ;
    return rand()%range + MinVal ;
}


bool rand_prob(float prob)
{
    float fVal = (rand()%1000)/10.0f;
    return fVal < (prob * 100.0f);
}




