#include "swap.h"
#define BUFFER 64
#define Psize 128 //PageSize


Swap::Swap()      
{
	swapFilesMap = new BitMap(BUFFER);
	SwapFile=open("SwapFile.txt",O_RDWR|O_CREAT|O_EXCL,00600);
}

Swap::~Swap()     
{
	delete swapFilesMap;
	close(SwapFile);
	
}

int Swap::Allocate(char* buffer)
{
	int SwaPage = swapFilesMap->Find();
	int offset = SwaPage*Psize;
        lseek(SwapFile, offset, SEEK_SET);
	write(SwapFile, buffer, Psize);
        return SwaPage;	
}


void Swap::Unallocate(int page, char* buffer)      // Unallocate
{
	int leer;
	int offset = page*Psize;
	lseek(SwapFile, offset, SEEK_SET);
	leer = read(SwapFile, buffer, Psize);
	swapFilesMap->Clear(page);
}


