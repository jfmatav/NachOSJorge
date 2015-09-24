#include "semaforoTabla.h"
#define BUFFER 100


semaphoreTable::semaphoreTable()       // Initialize 
{
	opensem = new int [BUFFER];
    	semaforoMap = new BitMap(BUFFER);
	usage = 1;
}

semaphoreTable::~semaphoreTable()      // De-allocate
{
	delete opensem;
	delete semaforoMap;
}
    

int semaphoreTable::Create( int UnixHandle ) // Register the semaphore handle
{
	int NachosHandle = semaforoMap->Find();
	opensem[NachosHandle] = UnixHandle;
	return NachosHandle;
}


bool semaphoreTable::Exist( int NachosHandle )
{
	if(NachosHandle <= BUFFER && 0 <= NachosHandle)
	{return semaforoMap->Test(NachosHandle);}
	else
	{return false;}	
}


int semaphoreTable::Delete( int NachosHandle )      // Unregister the semaphore handle
{
	if (Exist(NachosHandle))
	{
		semaforoMap->Clear(NachosHandle);
		return  opensem[NachosHandle];
	}
	else
	{return -1;}
}


void semaphoreTable:: addThread()
{
	usage++;
}


void semaphoreTable:: delThread()
{
	usage--;
	if (usage == 0)
	{
	    for(int x=0; x<=BUFFER; x++)
	    {
		if (Exist(x))
		{
		    //Se llama al destructor del semaforo de Unix
		int unixId = opensem[x];   // Read semaphore id 
        	int st = semctl(unixId, 1, IPC_RMID);
		cout << "Han llamado al destructor" << endl;
        	if(st == -1)
   			cout << "Hubo un error al destruir el semáforo." << endl;
		    	semaforoMap->Clear(x); // bitmap disponible
		}
	    }
	    delete this;
	}
}



int semaphoreTable::getUnixHandle( int NachosHandle )
{
	if (Exist(NachosHandle))
	{return opensem[NachosHandle];}
	else
	{return -1;}
}

// método de prueba para el cónclave
void semaphoreTable:: Print()  
{
	for (int x=0; x<= BUFFER; x++)
	{
	   if (Exist(x))
	   {printf("NachosHandle is : %d, and UnixHandle id %d \n", x,  opensem[x]);}
	}
}  



