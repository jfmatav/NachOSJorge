#include "RT.h"

ReverseTable::ReverseTable(){
	tabla = new TranslationEntry [32];	
}
 ReverseTable::~ReverseTable(){
	delete tabla;
}
    
int ReverseTable::freeUp(){
	int pos=0;
	bool found = false
	while (!found){
		if (tabla[pos]->use){
			tabla[pos]->use = false;			
			pos = (pos+1) % 32;
		}else{
			found = true;
		}	
	}
	return pos;
}

void ReverseTable::update (int PhysicalFrame, TranslationEntry* VirtualPage){
	tabla[PhysicalFrame] = VirtualPage;
}

int ReverseTable::getVirtualPage (int PhysicalFrame){
	return tabla[PhysicalFrame]->virtualPage;
}
