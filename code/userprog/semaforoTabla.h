#ifndef SEMAFOROTABLA_H
#define SEMAFOROTABLA_H

#include "copyright.h"
#include "filesys.h"
#include "bitmap.h"
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <iostream>
using namespace std;



class semaphoreTable {

  public:
	
    	semaphoreTable();       // Initialize 
    	~semaphoreTable();      // De-allocate
    
   	int Create( int UnixHandle ); // Register the semaphore handle
    	int Delete( int NachosHandle );      // Unregister the semaphore handle
    	bool Exist( int NachosHandle );
    	int getUnixHandle( int NachosHandle );
    	void addThread();
	void delThread();

    	void Print();               // Print contents
    
  private:
    	int * opensem;		// A vector with user opened semaphore
    	BitMap * semaforoMap;	// A bitmap to control our vector
	int usage;
    				

};
#endif // SEMAFOROTABLA_H
