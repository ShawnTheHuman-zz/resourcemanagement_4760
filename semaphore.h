
#ifndef SEMAPHORE
#define SEMAPHORE

#include <sys/sem.h>


class Semaphore
{
    private:
    
        bool _bCreator;
        int _semid;
        bool _initialized;
        struct sembuf sb;

    public:

    Semaphore(key_t, bool, int = 1);
    ~Semaphore();

    // Check if initialized properly
    bool is_init() { return _initialized; };

    // Semaphore Wait
    void Wait();

    // Semaphore Signal
    void Signal();    

};

#endif
