#include "nachostabla.h"
#define BUFFER 23


NachosOpenFilesTable::NachosOpenFilesTable()       // Initialize 
{
	openFiles = new int[BUFFER];
        openFilesMap = new BitMap(BUFFER);
	openFilesMap->Mark(0);
	openFilesMap->Mark(1);
	openFilesMap->Mark(2);
	
}

NachosOpenFilesTable::~NachosOpenFilesTable()      // De-allocate
{
	delete openFiles;
	delete openFilesMap;
}
    
int NachosOpenFilesTable:: Open( int UnixHandle ) // Register the file handle
{
	int NachosHandle = openFilesMap->Find();
	openFiles[NachosHandle] = UnixHandle;
	return NachosHandle;
}
int NachosOpenFilesTable:: Close( int NachosHandle )      // Unregister the file handle
{
	if (isOpened(NachosHandle)){
		openFilesMap->Clear(NachosHandle);
		return  openFiles[NachosHandle];
	}else{
		return -1;
	}
}

bool NachosOpenFilesTable:: isOpened( int NachosHandle )
{	
	if (NachosHandle <= BUFFER && 3 <= NachosHandle)
	{return openFilesMap->Test(NachosHandle);}
	else
	{return false;}
}

int NachosOpenFilesTable:: getUnixHandle( int NachosHandle )
{
	if (isOpened(NachosHandle)){
		return openFiles[NachosHandle];
	}else{
		return -1; 
	}
}
void NachosOpenFilesTable:: addThread()
{
	usage++;
}
void NachosOpenFilesTable:: delThread()
{
	usage--;
	if (usage == 0){
		for(int x=3; x<=BUFFER; x++){
			if (isOpened(x)){
				close(openFiles[x]);// cerramos el archivo con el UnixHandle almacenado en file[NachosHandle] 
				openFilesMap->Clear(x); // bitmap disponible
			}
		}
		delete this; //
	}
}
void NachosOpenFilesTable:: Print()  
{
	for (int x=3; x<= BUFFER; x++){
		if (isOpened(x)){
			printf("NachosHandle is : %d, and UnixHandle id is % d \n", x,  openFiles[x]);
		}		
	}
}  


