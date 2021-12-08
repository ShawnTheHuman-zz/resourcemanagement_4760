
#ifndef SEMAPHORE
#define SEMAPHORE

#include <sys/sem.h>


class productSemaphores
{
    private:
    
        bool _bCreator;
        int _semid;
        bool _initialized;
        struct sembuf sb;

    public:

    productSemaphores(key_t, bool, int = 1);
    ~productSemaphores();

    // Check if properly setup
    bool is_init() { return _initialized; };

    // Semaphore Wait
    void Wait();

    // Semaphore Signal
    void Signal();    

};

#endif
