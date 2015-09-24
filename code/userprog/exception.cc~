// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "console.h"
#include "addrspace.h"
#include "nachostabla.h"
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <iostream>

using namespace std;

#define FALSE 0

int TLBIndex = 0; /*Initial position of TLB*/
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
//********************************************************************************************************

void NachosForkThread(int p) 
{

    AddrSpace *space;

    space = currentThread->space;
    space->InitRegisters();             // set the initial register values
    space->RestoreState();              // load page table register

// Set the return address for this thread to the same as the main thread
// This will lead this thread to call the exit system call and finish
    machine->WriteRegister( RetAddrReg, 4 );

    machine->WriteRegister( PCReg, p );
    machine->WriteRegister( NextPCReg, p + 4 );

    machine->Run();                     // jump to the user progam
    ASSERT(FALSE);

}




//********************************************************************************************************
void run_Thread(){
    //initialize registers, restore them, then run
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
}

//********************************************************************************************************

void returnFromSystemCall()  {

        int pc, npc;

        pc = machine->ReadRegister( PCReg );
        npc = machine->ReadRegister( NextPCReg );
        machine->WriteRegister( PrevPCReg, pc );        // PrevPC <- PC
        machine->WriteRegister( PCReg, npc );           // PC <- NextPC
        machine->WriteRegister( NextPCReg, npc + 4 );   // NextPC <- NextPC + 4

}

//********************************************************************************************************

void Nachos_Halt() {                    // System call 0

        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();

}       // Nachos_Halt

//********************************************************************************************************

void Nachos_Exit() {			// System call 1
/* System call definition described to user
	void Exit(
		int status	// Register 4
	);
*/
	int status= machine->ReadRegister(4);
/*	currentThread->openTable->delThread();
	currentThread->semTable->delThread();
	for (int i=0; i<=100; i++){
		if(myThreads->Test(i)){
			myThreads->Signal(myThreads->get_ID(currentThread));
		}
	}
	myThreads->Del(myThreads->get_ID(currentThread));

		
	machine->WriteRegister(0, status);*/

	cout << "Estatus del exit" <<status << endl;
	currentThread->	Finish ();	
        returnFromSystemCall();
	

}	//Nachos_Exit

//********************************************************************************************************

void Nachos_Exec() {			// System call 2
/* System call definition described to user
	int Exec(
		char* name	// Register 4
	);
*/
	int nameVrtAddr;	// Virtual address memory
	char * filename;	// File to open
	int fileLength = 0;	// Length of file 
        char c;			// every singel char read
	OpenFile * executable;	// Executable to link the memory to new thread
	AddrSpace * space;	// Space to assing to new thread
	char * file = new char[100];	// New file where is located our archive
	int pid = 0;		// Id from new thread created
	
	Thread * son = new Thread ("exec_thread");	// Create a new thread to execute
	son->openTable = new NachosOpenFilesTable();	// Create a new table of files
	son->semTable  =  new semaphoreTable();	// Create a new table of semaphores

	nameVrtAddr = machine->ReadRegister(4); // Read file	
	//traslate form Unix to NachOS
	do{
            machine->ReadMem(nameVrtAddr+fileLength, 1, (int*)(&c));
            file[fileLength] = c;
            ++fileLength;
        } while(c != '\0');
        file[fileLength] = 0;

	cout << file << endl;

	executable = fileSystem->Open(file);

	if (executable == NULL) {
		printf("Unable to open file %s\n", filename);		
	}else{
		space = new AddrSpace(executable);    
		son->space = space;
		pid = myThreads->Add(son); // add ID to new thread 			
		son->Fork ((VoidFunctionPtr)run_Thread, 0); // run new thread 
		delete executable; // close file

		machine->WriteRegister(2, pid);
		returnFromSystemCall();
	}

}	//Nachos_Exec

//********************************************************************************************************

void Nachos_Join() {			// System call 3
/* System call definition described to user
	int Join(
		int id  	// Register 4
	);
*/	
	int pid;
	pid = machine->ReadRegister(4); // 1st parameter

	if (myThreads->Test(pid)){ // if exist?
		myThreads->Wait(myThreads->get_ID(currentThread)); //make currentThread wait
		machine->WriteRegister(2, 0);   //we must return something , 0 denotes that the process waited on it's child
	} else {
		machine->WriteRegister(2, -1); // send error
	}

	returnFromSystemCall();

}	//Nachos_Join

//********************************************************************************************************

void Nachos_Create() {			// System call 4
/* System call definition described to user
	void Create(
		char* name  	// Register 4
	);
*/
	//Get the executable name

        int pointer = machine->ReadRegister(4);
	char* file = new char [100];
        int fileLength = 0;
        int c;

	do
        {
            machine->ReadMem((pointer+fileLength), 1, &c);
            //cout << "c: " << c << endl;
            file[fileLength] = c;
            ++fileLength;
        }
        while(c != '\0');
        file[fileLength] = 0;

	//cout << "El archivo es: " << file << endl;

	int unixHandle = open((const char*)file, O_RDWR|O_CREAT|O_EXCL, 00600);
	int err = errno;

        close(unixHandle);

        int nachosHandle;

	if(-1 == unixHandle)	//Check for errors;
        {
		cout << "No se pudo crear el archivo, error: " << err << endl;
		nachosHandle = -1;
   	}
        else			//no hubo errores
        {
		//meterlo en la table, coger el nachosHandle
        }

	machine->WriteRegister(2, nachosHandle);

        returnFromSystemCall();

}	//Nachos_Create

//********************************************************************************************************

void Nachos_Open() {                    // System call 5
/* System call definition described to user
	int Open(
		char *name	// Register 4
	);
*/
        
	int nachosHandle;

        //Get the file name
	int pointer = machine->ReadRegister(4);          // buffer = Read data from address given by user;
	char* file = new char [100];
        int fileLength = 0;
        char c;

	do
        {
            machine->ReadMem(pointer+fileLength, 1, (int*)(&c));
            file[fileLength] = c;
            ++fileLength;
        }
        while(c != '\0');
        file[fileLength] = 0;

        //Open file using Unix
        int uh = open((const char*)file, O_RDWR);
        int err = errno;

	if(-1 == uh)
	{
        	cout << "No pudo abrirse el archivo [Unix_Open], error: " << err << endl;
		nachosHandle = -1;
	}
	else
	{
		nachosHandle = currentThread->openTable->Open(uh);

        	// Verify for errors
        	if(nachosHandle == -1)
        	{cout << "No pudo abrirse el archivo [Nachos_Open]." << endl;}
	}
        
        // Return the result
        machine->WriteRegister(2, nachosHandle);

	delete file;

        returnFromSystemCall();

}       // Nachos_Open

//****************************************************************************************************

void Nachos_Read() {			// System call 6
/* System call definition described to user
        int Read(
		char *buffer,	// Register 4
		int size,	// Register 5
		OpenFileId id	// Register 6
	);
*/

////////////////////MANEJO DE EXCEPCIONES PARA LEER DE PANTALLA Y LEER DE TECLADO/////////////////////////

	int pointer = machine->ReadRegister(4);
        int tam = machine->ReadRegister(5);
        int nachosHandle = machine->ReadRegister(6);

	int sizeRead = 0;

	char buffer[tam];

	switch(nachosHandle)
	{
		case ConsoleInput:
			sizeRead = read(ConsoleInput, buffer, tam);
			break;

		case ConsoleOutput:
		case ConsoleError:
			cout << "Se intento leer de un medio no valido." << endl;
			break;


		default://all other archives
		if(currentThread-> openTable->isOpened(nachosHandle))
        	{
                	int uh = currentThread->openTable->getUnixHandle(nachosHandle);
                	sizeRead = read(uh, buffer, tam);
        	}	
		else //si el archivo no esta abierto
        	{cout << "Esta tratando de leer de un archivo no abierto." << endl;}

	}
	//Escribir el buffer de Unix en memoria de Nachos
	for(int i = 0; i < sizeRead; ++i)
	{machine->WriteMem(pointer+i, 1, (int)buffer[i]);}

	machine->WriteRegister(2, sizeRead);

       	returnFromSystemCall();

}	//Nachos_Read

//****************************************************************************************************

void Nachos_Write() {                   // System call 7
/* System call definition described to user
        void Write(
		char *buffer,	// Register 4
		int size,	// Register 5
		OpenFileId id	// Register 6
	);
*/
        int pointer = machine->ReadRegister(4);          // buffer = Read data from address given by user;
        int size = machine->ReadRegister(5);	         // Read size to write
	char buffer[size];

	for(int i = 0; i < size; ++i){
		machine->ReadMem(pointer+i, 1, (int*)(&buffer[i]));
	}
        buffer[size] = 0;

	//cout << "Se va a escribir: " << buffer;
        OpenFileId nachosHandle = machine->ReadRegister(6);	// Read file descriptor
	//cout << " en el archivo nh " << nachosHandle << endl;
	//Console->P();
	switch (nachosHandle) 
        {
		case  ConsoleInput:	// User could not write to standard input
			machine->WriteRegister( 2, -1 );
			break;
		case  ConsoleOutput:
			buffer[size] = 0;
			printf( "%s", buffer );
	                break;
                case  ConsoleError: // This trick permits to write integers to console 
                        printf( "%d\n", machine->ReadRegister( 4 ) ); 
                        break;
         
                default: // All other opened files 
		//cout << "Es un archivo regular" << endl;
                if(currentThread->openTable->isOpened(nachosHandle))// Verify if the file is open
                {
		    //cout << "El archivo esta abierto" << endl;
                    int unixHandle = currentThread->openTable->getUnixHandle(nachosHandle);
		    //cout << "El uh es " << unixHandle << endl;
                    int sizeWritten = write(unixHandle, buffer, size); 
                    //cout << "tamaño escrito: " << sizeWritten << endl;
                    machine->WriteRegister(2, sizeWritten);
                }else{
		    machine->WriteRegister(2, -1);
		    cout << "Se esta tratando de escribir a un archivo no abierto." << endl;
                }
       }
       //Console->V();
       returnFromSystemCall(); // Update the PC registers 
} // Nachos_Write

//********************************************************************************************************

void Nachos_Close() {			// System call 8
/* System call definition described to user
	void Close(
		int OpenFileID  	// Register 4
	);
*/
	int nachosHandle = machine->ReadRegister(4);
	if(currentThread->openTable->isOpened(nachosHandle)){
		int uh = currentThread->openTable->getUnixHandle(nachosHandle);
                if(close(uh)){
			cout << "Hubo un error de Unix al cerrar el archivo." << endl;
                }else{
      			currentThread->openTable->Close(nachosHandle);
                }
        }else{
		cout << "Hubo un error de NachOS al cerrar el archivo - archivo no abierto." << endl;
        }
       returnFromSystemCall();

}	//Nachos_Close

//********************************************************************************************************

void Nachos_Fork() {			// System call 9
/* System call definition described to user
	void Fork(
		void (*func)() 	// Register 4
	);
*/

	// We need to create a new kernel thread to execute the user thread
	Thread * newT = new Thread( "child to execute Fork code" );

	// We need to share the Open File Table structure with this new child
	delete newT->openTable;
        newT->openTable = currentThread->openTable;
        newT->openTable->addThread();
        
        delete newT->semTable;
        newT->semTable = currentThread->semTable;
        newT->openTable->addThread();

        myThreads->Add(newT);

	newT->space = new AddrSpace(currentThread->space);

	// We (kernel)-Fork to a new method to execute the child code
	// Pass the user routine address, now in register 4, as a parameter
	newT->Fork( (VoidFunctionPtr)NachosForkThread, (void*)machine->ReadRegister( 4 ) );

	returnFromSystemCall();	// This adjust the PrevPC, PC, and NextPC registers

	DEBUG( 'u', "Exiting Fork System call\n" );

}	//Nachos_Fork

//********************************************************************************************************

void Nachos_Yield() {			// System call 10

	currentThread->Yield();
	returnFromSystemCall();

}	//Nachos_Yield

//********************************************************************************************************

void Nachos_SemCreate() {		// System call 11
/* System call definition described to user
	void SemCreate(
		int initval 	// Register 4
	);
*/
	union semun {
    		int              val;    /* Value for SETVAL */
    		struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    		unsigned short  *array;  /* Array for GETALL, SETALL */
    		struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
	};

 	int initval = machine->ReadRegister(4);          // buffer = Read data from address given by user;

	union semun x;
	x.val = initval;
 	int unixId =  semget(0xA11788, 1, IPC_CREAT|0600);                // Read semaphore id 
 	if(unixId == -1){
	   	cout << "Hubo un error al crear el semáforo" << endl;
 	}else{
		int st = semctl (unixId, 0, SETVAL, x);
		if(st == -1){
			cout << "Hubo un error al inicializar el semáforo" << endl;
		}else{
			int semid = currentThread->semTable->Create(unixId);  
  			machine->WriteRegister(2, semid);

		}
	}

	returnFromSystemCall();

}	//Nachos_SemCreate

//********************************************************************************************************
void Nachos_SemDestroy() {		// System call 12
/* System call definition described to user
	void SemDestroy(
		int SemId 	// Register 4
	);
*/
        int SemId = machine->ReadRegister(4);          // buffer = Read data from address given by user;
 	int unixId = currentThread->semTable->Delete(SemId);   // Read semaphore id 
        int st = semctl(unixId, 1, IPC_RMID);
	cout << "Han llamado al destructor" << endl;
        if(st == -1)
   	cout << "Hubo un error al destruir el semáforo." << endl;

	returnFromSystemCall();

}	//Nachos_Destroy

//********************************************************************************************************
void Nachos_SemSignal() {		// System call 13
/* System call definition described to user
	void SemSignal(
		int SemId 	// Register 4
	);
*/

	int SemId = machine->ReadRegister(4);          // buffer = Read data from address given by user;
 	int unixId = currentThread->semTable->getUnixHandle(SemId);   // Read semaphore id 
 	struct sembuf S;
   	S.sem_num = 0;
   	S.sem_op = 1;
   	int st = semop(unixId, &S, 1);
	if(st == -1)
   	  cout << "Hubo un error al hacer Signal." << endl;

	returnFromSystemCall();

}	//Nachos_Signal

//********************************************************************************************************

void Nachos_SemWait() {			// System call 14
/* System call definition described to user
	void SemWait(
		int SemId 	// Register 4
	);
*/
	int SemId = machine->ReadRegister(4);          // buffer = Read data from address given by user;
 	int unixId = currentThread->semTable->getUnixHandle(SemId);   // Read semaphore id 
	struct sembuf W;
	W.sem_num = 0;
   	W.sem_op = -1;
   	int st = semop(unixId, &W, 1);
	if(st == -1){
	   	cout << "Hubo un error al hacer Wait." << endl;
		perror("Semaforo::wait");
	}
	returnFromSystemCall();

}	//Nachos_SemWait

//********************************************************************************************************

void Nachos_PageFaultManagement(){

	//se averigua cual es la pagina que no esta en la TLB	
	int badPage = machine->ReadRegister(39)/PageSize;   //BadVAddrReg 

	//se pide al Addrspace que cargue la pagina en memoria
	int memoryFrame = currentThread->space->loadPage(badPage);

	//Se averigua cual es la pagina virtual que va a salir de la TLB
	int tlbExitingPage = machine->tlb[TLBIndex].virtualPage;

	//Se guardan los bits dirty y use del TranslationEntry que se va a escribir en la TLB
	currentThread->space->pageTable[tlbExitingPage].dirty = machine->tlb[TLBIndex].dirty;
	currentThread->space->pageTable[tlbExitingPage].use = machine->tlb[TLBIndex].use;

	//Se actualiza el registro de la TLB		
    	machine->tlb[TLBIndex].virtualPage = currentThread->space->pageTable[badPage].virtualPage;
    	machine->tlb[TLBIndex].physicalPage = currentThread->space->pageTable[badPage].physicalPage;
    	machine->tlb[TLBIndex].dirty = currentThread->space->pageTable[badPage].dirty;
    	machine->tlb[TLBIndex].use = currentThread->space->pageTable[badPage].use;
    	machine->tlb[TLBIndex].valid = currentThread->space->pageTable[badPage].valid;


	//Metodo para imprimir los contenidos de la TLB
        /*cout << "TLBENTR\tVirtPag\tPhysPag\tdirty\tuse\tvalid" << endl;
        for(int i = 0; i < 4; ++i)
        {
	cout << i << "\t" << machine->tlb[ i ].virtualPage
                  << "\t" << machine->tlb[ i ].physicalPage
                  << "\t" << machine->tlb[ i ].dirty
                  << "\t" << machine->tlb[ i ].use
                  << "\t" << machine->tlb[ i ].valid << endl;
	} 
        cin.get(c);*/
	
	//Se avanza el indice del FIFO en la TLB
	TLBIndex = (TLBIndex+1) % 4;
}

//********************************************************************************************************
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which) 
    {	
       case SyscallException:
          switch (type) 
          {

             case SC_Halt:           	   // System call # 0
                Nachos_Halt(); 
                break;
             case SC_Exit:		   // System call # 1
		Nachos_Exit();
                break;
             case SC_Exec:		   // System call # 2
		Nachos_Exec();
                break;
             case SC_Join:		   // System call # 3
		Nachos_Join();
                break;
             case SC_Create:		   // System call # 4
		Nachos_Create();
                break;
             case SC_Open:		   // System call # 5
                Nachos_Open();
                break;
             case SC_Read:		   // System call # 6
		Nachos_Read();
                break;
             case SC_Write:		   // System call # 7
                Nachos_Write();
                break;
             case SC_Close:		   // System call # 8
		Nachos_Close();
                break;
             case SC_Fork:		   // System call # 9
		Nachos_Fork();
                break;
             case SC_Yield:		   // System call # 10
		Nachos_Yield();
                break;
	     case SC_SemCreate:		   // System call # 11
		Nachos_SemCreate();
		break;	
	     case SC_SemDestroy:	   // System call # 12
		Nachos_SemDestroy();
		break;
	     case SC_SemSignal:		   // System call # 13
		Nachos_SemSignal();
		break;
	     case SC_SemWait:		   // System call # 14
		Nachos_SemWait();
		break;	     
    	     default:
                printf("Unexpected syscall exception %d\n", type );
		Nachos_Halt();
                ASSERT(FALSE);
        	break;
  	}
       break;

       case PageFaultException:
		Nachos_PageFaultManagement();
      		 break;
       
       	

       default:
          printf( "Unexpected exception %d, machine halting.\n", which );
          Nachos_Halt();
          break;
    }
}
