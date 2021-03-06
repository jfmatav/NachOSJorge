// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "bitmap.h"
#include "machine.h"

#define UserStackSize		1024 	// increase this as necessary!

//----------------------------------------------------------------------
// ReverseTableEntry
// 	 Estructura que registra la relacion entre marcos de memoria
//	y paginas virtuales de un ejecutable;	
//----------------------------------------------------------------------

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"

    AddrSpace(AddrSpace *parent);       // Create the address space for the fork
					// system call, copies the physical pages
					// of the parent's process' code and data
					// segment, creates a new stack.

    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 

    void copyParameters(char*, int);

    int loadPage(int);

    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
     

  private:
    
    unsigned int numPages;		// Number of pages in the virtual 
					// address space

    int stackBegining;

#ifdef VM
    int SCidx;				//utilizado para la busqueda SecondChance

    TranslationEntry  **reverseTable;    // Reverse Table	

    int freeUp(); 	      		// allocate swap memory
    
    void update (int, TranslationEntry*);     //  refresh data
    
    int getVirtualPage (int);		//Get virtual page from reverse table

    //void loadBufferFormMemory(char* swapBuffer, int frameAddress, int exitingVirtualPage);

    
    OpenFile* executablePtr;  //puntero al ejecutable que se recibe como parametro, utilizado para guardar la referencia
    int firstDataPage;	      //indicador de la primera pagina que se puede poner dirty
    int firstEmptyPage;       //atributo utilizado para saber en que pagina debe crearse una pagina nueva, sin cargar del ejecutable
    int swapIndex;	      //utilizado para saber cual pagina es la siguiente a desplazar de memoria al Swap

    int binBegining;

#endif

};

#endif // ADDRSPACE_H
