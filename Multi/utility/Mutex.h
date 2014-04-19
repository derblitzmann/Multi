#ifndef MUTEX_H
#define MUTEX_H

#include "Multi.h"

struct Thread;

template<typename T>
class Mutex {
	private:
		volatile T data;
		volatile Thread *owner;
		
	public:
		Mutex();
		Mutex(T init);
		
		// These change the variable: available
		// This is the 
		void grab();
		void release();
		
		T getData();
		void setData(T newData);
};

#endif