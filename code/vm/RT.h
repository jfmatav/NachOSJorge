#ifndef RT_H
#define RT_H

#include "copyright.h"
#include "filesys.h"
#include "translate.h"
#include "bitmap.h"
#include <unistd.h>
#include <assert.h> 




class ReverseTable{

  public:
	
    	ReverseTable();       // Initialize 
    	~ReverseTable();      // De-allocate
    
   	int freeUp(); 	      // allocate swap memory
    	void update (int PhysicalFrame, TranslationEntry * VirtualPage);         //  refresh data
	int getVirtualPage (int PhysicalFrame);	 	
    
  private:
	TranslationEntry  * tabla;
};
#endif // RT_H
