#ifndef SWAP_H
#define SWAP_H

#include "copyright.h"
#include "filesys.h"
#include "bitmap.h"
#include <unistd.h>
#include <assert.h> 
#include <fcntl.h>      /* Modos de apertura */
#include <stdlib.h>     /* Funciones de ficheros */



class Swap{

  public:
	
    	Swap();       // Initialize 
    	~Swap();      // De-allocate
    
   	int Allocate(char* buffer ); // allocate swap memory
    	void Unallocate(int page, char* buffer);         //    unallocate swap memory
    	
    	

    	
    
  private:
    	BitMap * swapFilesMap;	// A bitmap to control our vector
        int SwapFile;
    	

};
#endif // Swap_H
