#include "nachosthreads.h"
#define BUFFER 100


NachosThreads::NachosThreads(){

	mythreads.usage=1;	
 	ThreadMap = new BitMap(BUFFER);
}    

NachosThreads::~NachosThreads(){	
	delete ThreadMap;
}


int NachosThreads::Add(Thread * t){
	int thread_id = ThreadMap->Find();
	mythreads.usage+=1;
        mythreads.threads[thread_id].thread_ptr =  new Thread ("new_thread");
	mythreads.threads[thread_id].thread_ptr = t;
	return thread_id;


}
int NachosThreads::get_ID(Thread * t){
	bool found= false;
	int pos=0;
	while(!found && pos <=100){
		if (mythreads.threads[pos].thread_ptr == t){
			return pos;
		}else{
			pos++;
		}
	}
}
int NachosThreads::Del(int thread_id){
	if(BUFFER>=0 && BUFFER<100){
		if(ThreadMap->Test(thread_id)>=0){
			ThreadMap->Clear(thread_id);
			return 0;
		}
		else
			return -1;
	}

}

int NachosThreads::Cant(){
	return mythreads.usage;
}


void NachosThreads::Wait(int thread_id){
	mythreads.threads[thread_id].sem_ptr->P();
}

void NachosThreads::Signal(int thread_id){
	mythreads.threads[thread_id].sem_ptr->V();
}

bool NachosThreads::Test(int thread_id){
	bool found = false;
	if(BUFFER>=0 && BUFFER<100){
		if(ThreadMap->Test(thread_id)>=0){
			found=false;
			return found;
		}else{
			return found;
		}
	}else{ 
	return found;
	}
}
