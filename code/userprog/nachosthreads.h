#ifndef NACHOSTHREADS_H
#define NACHOSTHREADS_H

#include "copyright.h"
#include "filesys.h"
#include "bitmap.h"
#include "synch.h"
#include <unistd.h>
#include "syscall.h"
#include "thread.h"


struct Dato{
	int usage ;
	struct thread{
        	Thread *thread_ptr;
		Semaphore* sem_ptr;
	}threads[100];
};


class NachosThreads{
         
  public:
	
    	NachosThreads();       // Initialize 
    	~NachosThreads();      // De-allocate
    
   	int Add (Thread *); // Register the ID Tread    	
        int Del (int);
	bool Test (int);   // exist?
	int get_ID(Thread *);
        int Cant ();
	void Wait(int);
	void Signal(int);

    	
    
  private:
    	BitMap * ThreadMap;	// A bitmap to control our vector
	Dato mythreads;
};
#endif // NACHOSTHREADS_H
