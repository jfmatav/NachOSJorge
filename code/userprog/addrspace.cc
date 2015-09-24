// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "bitmap.h"
#include "noff.h"

#define binBegining 40 //constante utilizada para determinar el inicio del codigo dentro del ejecutable

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;
#ifndef VM 
    // use to copy form a file to memory
    int segmentSize,startVirtAddr,fileAddr,presentVirtAddr;
    int virtPage,physPage,phyAddr;
    int segmentRead,segmentLeft;
    int offset;  //variable that serves as a pointer to where the initialized data begins
#endif
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    {SwapHeader(&noffH);}

    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;	// we need to increase the size
												// to leave room for the stack

    stackBegining = noffH.code.size + noffH.initData.size + noffH.uninitData.size;

    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
#ifndef VM
    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
#endif						// at least until we have
						// virtual memory

    cout << "Este codigo tiene " << numPages << " paginas" << endl;

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);

    pageTable = new TranslationEntry[numPages];

    //////////////////////////COMIENZA LA COMPILACION CONDICIONAL SI SE UTILIZA MEMORIA VIRTUAL/////////////////////////////////////////

    #ifdef  VM

    //Se crea la ReverseTable, la cual asocia los marcos de memoria con punteros a las entradas del Page Table
    reverseTable = new TranslationEntry*[32];        

    //Ciclo que inicaliza la pageTable
    for (i = 0; i < numPages; i++) 
    {
	pageTable[i].virtualPage = i;
	pageTable[i].physicalPage = -1;
	pageTable[i].valid = false;
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = false;
    	pageTable[i].swapFrame = -1;
    }

    //se guarda el puntero del ejecutable, para cargar del ejecutable cuando sea necesario
    executablePtr = executable;

    //este indice se utiliza para saber cuales paginas no se cargan del ejecutable, sino que nada mas se les reserva espacio
    firstEmptyPage = ((noffH.code.size + noffH.initData.size) / PageSize)+1;

    //indice para el algoritmo del SecondChance
    SCidx = 0;
    
    /*char e;

	cout << "Numero de paginas virtuales: " << numPages << endl
	     << "Primera pagina vacia: " << firstEmptyPage << endl;
    cin.get(e);*/

    ///////////////////////////TERMINA LA COMPILACION CONDICIONAL SI SE UTILIZA MEMORIA VIRTUAL/////////////////////////////////////////
    ///////////////////////////COMPILACION SI SE ESTA TRABAJANDO CON MEMORIA FISICA UNICAMENTE//////////////////////////////////////////

    #else
	
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) 
    {
	pageTable[i].virtualPage = i;				// virtual page # = default compiler
	pageTable[i].physicalPage = BitMapMemory->Find(); 	// physical page # = BitMap responsability, give a clear page. 
	pageTable[i].valid = true;
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages 
    }  

    //Now, we copy the the code and data segments into memory    

    if (noffH.code.size > 0) 
    {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", noffH.code.virtualAddr, noffH.code.size);

        segmentSize = noffH.code.size; 			//Get size of code segment
        startVirtAddr = noffH.code.virtualAddr;		//Get virtual address page 
        fileAddr = noffH.code.inFileAddr;		//Get file address

        segmentRead = 0; 			// Position where to start to read
        phyAddr = 0;				// Physical address
        segmentLeft = segmentSize;		// size of segment left to load into memory, in the beginig would be all the size
        presentVirtAddr = startVirtAddr;	// Start load from here
        
	while(segmentLeft>0) //start load into memory until there isn't segment code left 
	{ 			
            virtPage = presentVirtAddr/PageSize;	   // Get virtual page div size of page
            physPage = pageTable[virtPage].physicalPage;   // Get physical address, located in pageTable modified by BitMapMemory->Find
 
            //now calculate physical address
            phyAddr = physPage*PageSize; // Physical address will be its value * 128 bytes (PageSize)to get the size of a page in memory

	    //So important, load to memory current page 
            executable->ReadAt(&(machine->mainMemory[phyAddr]),PageSize,fileAddr);

	    //Finally, update new values to load a new page
            segmentRead+=PageSize;  	//add to segment read the size of  page already loaded (128 bytes)
            presentVirtAddr+=PageSize; // add to presente virtual the size of  page already loaded (128 bytes) 
            fileAddr+=PageSize; 	//add to the file address the size of  page already loaded (128 bytes)
            segmentLeft-=PageSize;	//short the size of segment code left
        }
    }
    
    if (noffH.initData.size > 0) 
    {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", noffH.initData.virtualAddr, noffH.initData.size);
	
	//Get all information about the segment of initialized data, address, virtual and physical address 
        segmentSize = noffH.initData.size;
        startVirtAddr = noffH.initData.virtualAddr;
        fileAddr = noffH.initData.inFileAddr;
	
	//Set initial values
        segmentRead = 0;
        phyAddr = 0;
        segmentLeft = segmentSize; 		//segment of initialized data left
        presentVirtAddr = startVirtAddr;
        offset = startVirtAddr%PageSize;
        while(segmentLeft>0)
        { 		//start load into memory until there isn't segment of initialized data left 
            virtPage = presentVirtAddr/PageSize;	   // Get virtual page div size of page
            physPage = pageTable[virtPage].physicalPage;   // Get physical address, located in pageTable modified by BitMapMemory->Find

            //now calculate physical address
            phyAddr = (physPage*PageSize)+offset; // as we did before

            executable->ReadAt(&(machine->mainMemory[phyAddr]),PageSize,fileAddr); //So important, load to memory current page 

	    //Finally, update new values to load a new page
            segmentRead+=PageSize;  	//add to segment read the size of  page already loaded (128 bytes)
            presentVirtAddr+=PageSize; // add to presente virtual the size of  page already loaded (128 bytes) 
            fileAddr+=PageSize; 	//add to the file address the size of  page already loaded (128 bytes)
            segmentLeft-=PageSize;	//short the size of segment code left
        }

    }

    #endif


}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//----------------------------------------------------------------------

AddrSpace::AddrSpace(AddrSpace* parent){
    numPages = parent->numPages;
    pageTable = new TranslationEntry[numPages];

    for(int i = 0; i < numPages-8; ++i)
    {
	 pageTable[i].virtualPage = i;
         pageTable[i].physicalPage = parent->pageTable[i].physicalPage;
         pageTable[i].valid = true;
	 pageTable[i].use = false;
	 pageTable[i].dirty = false;
	 pageTable[i].readOnly = false;
    }

    for(int i = numPages-8; i < numPages; ++i)
    {
	 pageTable[i].virtualPage = i;
         pageTable[i].physicalPage = BitMapMemory->Find();
         pageTable[i].valid = true;
	 pageTable[i].use = false;
	 pageTable[i].dirty = false;
	 pageTable[i].readOnly = false;
    }

}


//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace(){
   delete pageTable;
#ifdef VM
   delete executablePtr;	//se cierra el archivo ejecutable
   delete reverseTable;		//se borra la reverseTable
#endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters(){
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    #ifndef VM
         machine->pageTable = pageTable;
         machine->pageTableSize = numPages;
    #else
         machine->pageTable = NULL;
    #endif
}

//----------------------------------------------------------------------
// AddrSpace::copyParameters
// 	
//	
//
//	
//----------------------------------------------------------------------


void AddrSpace::copyParameters(char* parameters, int length) 
{
    for(int i = 0; i < length; ++i)
    {machine->WriteMem(stackBegining + i, 1, (int)(*parameters));}
}


////METODOS AUXILIARES PARA EL MANEJO DEL REVERSE TABLE

int AddrSpace::freeUp()
{
	bool found = false;
	
	//mientras no se encuentre el marco apropiado
	while (!found)
	{
		//si use esta encendido, lo apaga y se mueve		
		if (reverseTable[SCidx]->use)
		{
			reverseTable[SCidx]->use = false;			
			SCidx = (SCidx+1) % 32;
		}
		else//si use esta apagado, deja de buscar
		{found = true;}	
	}
	//regresa el indice que encontro
	return SCidx;
}

void AddrSpace::update (int PhysicalFrame, TranslationEntry* VirtualPage)
{
	//guarda una referencia a un Transalation Entry, dependiendo del physical frame que reciba	
	reverseTable[PhysicalFrame] = VirtualPage;
}

int AddrSpace::getVirtualPage (int PhysicalFrame)
{
	//regresa el numero de pagina virtual que se encuentra en un numero especifico
	return reverseTable[PhysicalFrame]->virtualPage;
}



//----------------------------------------------------------------------
// AddrSpace::loadPage
// 	
//	
//
//	
//----------------------------------------------------------------------
int AddrSpace::loadPage (int neededVirtualPage)
{
    int memoryFrameNum;			//cual es el marco de memoria en el cual se reemplazar la pagina virtual
    int exitingVirtualPage;		//el numero de pagina virtual que desocupa el marco de memoria
    char* pageBuffer = new char [128];	//buffer que guarda la pagina que entra del Swap
    char* swapBuffer = new char [128];	//buffer que guarda la pagina que sale al Swap
    //char p;				//utilizado para impresiones de control



    if(pageTable[neededVirtualPage].valid)//si la pagina esta en memoria...
    {
	memoryFrameNum = pageTable[neededVirtualPage].physicalPage;
	pageTable[neededVirtualPage].use = true; // SC
    }
    else //si la pagina no esta en memoria...
    {
	//busca un lugar libre donde meterla pagina virtual
	memoryFrameNum = BitMapMemory->Find();
		
	if(memoryFrameNum != -1) //se encontro una pagina libre
	{	
		//carga la pagina del ejecutable, si es que esta ahi (o sea, no es una pagina vacia)	
		if(neededVirtualPage < firstEmptyPage)
		{executablePtr->ReadAt(&(machine->mainMemory[memoryFrameNum*PageSize]), PageSize, (neededVirtualPage*PageSize)+binBegining);}

		//actualiza los registros de la pagina virtual que se acaba de crear		
		pageTable[neededVirtualPage].physicalPage = memoryFrameNum;
		pageTable[neededVirtualPage].valid = true;
	    	pageTable[neededVirtualPage].readOnly = false;            	

	    	//se actauliza la reverseTable con la referencia del transalation entry que se acaba de cargar
		update(memoryFrameNum, &pageTable[neededVirtualPage]);

	}
	else //no hay paginas disponibles
	{ 
		//se averigua cual es el marco que se va a liberar y cual pagina virtual lo ocupa
		memoryFrameNum = freeUp();
	    	exitingVirtualPage = getVirtualPage(memoryFrameNum);

                //cout << "Se le va a dar la pagina " << memoryFrameNum << ", ocupada por " << exitingVirtualPage << endl;
	
		//se averigua donde comienza en memoria el marco que se modificara	    	
		int frameAddress = memoryFrameNum*PageSize;

		if(pageTable[exitingVirtualPage].dirty) //Si la pagina esta dirty, se manda al swap
	    	{
			for(int i = 0; i < PageSize; ++i)
			{swapBuffer[i] = machine->mainMemory[frameAddress+i];}
		}

    		//determinar de donde sale la pagina que hay que meter en memoria, del ejecutable o del Swap
    		if(pageTable[neededVirtualPage].swapFrame != -1)
		{//si sale del Swap, pedirle la pagina al Swap
			SwapFile->Unallocate(pageTable[neededVirtualPage].swapFrame, pageBuffer);
			for(int i = 0; i < PageSize; ++i)
			{machine->mainMemory[frameAddress+i] = pageBuffer[i];}
		}
		else
 		{//si sale del ejecutable (no se ha ensuciado), determinar si es codigo/datos o no
	    		
			if(neededVirtualPage<firstEmptyPage)//si es codigo/datos, copiar del ejecutable
			{executablePtr->ReadAt(&(machine->mainMemory[frameAddress]), PageSize, neededVirtualPage*PageSize);}
	    	}

		//actualizacion del pageTable de la pagina que se acaba de poner en memoria
		pageTable[neededVirtualPage].physicalPage = memoryFrameNum;
		pageTable[neededVirtualPage].valid = true;
		pageTable[neededVirtualPage].swapFrame = -1;
                //fin de la actualizacion

		//actualizacion de la reverseTable
 		update(memoryFrameNum, &pageTable[neededVirtualPage]);
                //fin de actualizacion

        	//actualizacion del pageTable de la pagina que se acabe de sacar de memoria
		pageTable[exitingVirtualPage].valid = false;
		if(pageTable[exitingVirtualPage].dirty)
		{ //Si estaba dirty, se copia al Swap
        		pageTable[exitingVirtualPage].swapFrame = SwapFile->Allocate(swapBuffer);
		}
                pageTable[exitingVirtualPage].physicalPage = -1;
                //fin de la actualizacion

	}//fin de caso que hay que reemplazar

    }//fin de caso la pagina no esta en memoria

    //METODO PARA IMPRIMIR EL ESTADO DE LOS PAGE TABLE
    /*cout << "PT_ENTR\tVirtPag\tPhysPag\tdirty\tuse\tvalid\tSFrame" << endl;
    for(int i = 0; i < numPages; ++i)
    {
	cout << i << "\t" << pageTable[i].virtualPage
       	<< "\t" << pageTable[i].physicalPage
       	<< "\t" << pageTable[i].dirty
       	<< "\t" << pageTable[i].use
      	<< "\t" <<pageTable[i].valid
       	<< "\t" <<pageTable[i].swapFrame << endl;
    }
    cout << endl;

    cout << "RT_ENTR\tVirtPag\tPhysPag\tdirty\tuse\tvalid\tSFrame" << endl;
    for(int i = 0; i < 32; ++i)
    {
	cout << i << "\t";
	if(reverseTable[i]){cout << reverseTable[i]->virtualPage;}
        else{cout << -1;}
        cout << endl;
    }

    cout << endl;*/	
    delete pageBuffer;
    delete swapBuffer;
    return memoryFrameNum;
}














