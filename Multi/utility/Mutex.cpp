#include "Multi.h"
#include "Mutex.h"

template <typename T>
Mutex<T>::Mutex()
{
	owner = NULL;
}

template <typename T>
Mutex<T>::Mutex(T init)
{
	owner = NULL;
	data = init;
}

template <typename T>
void Mutex<T>::grab()
{
	if(owner == NULL){
		owner = currentThread;
	}else if(owner == currentThread){
	}else{
		// Wait until other threads are done using this
		while(owner != NULL){
			// and by wait, I mean let the other threads run until they are done
			yield();
		}
		owner = currentThread;
	}
}

template <typename T>
void Mutex<T>::release()
{
	// Only release if the currentThread owns this mutex
	if(owner == currentThread){
		owner = NULL;
	}
}

template <typename T>
T Mutex<T>::getData()
{
	T ret;
	// If we are the owner (already grabbed), just return the value
	if(owner == currentThread){
		ret = data;
	}else{// if this is free or being used by another thread, retrieve the value when it is free and release once done
		grab();
		T ret = data;
		release();
	}
	return ret;
}

template <typename T>
void Mutex<T>::setData(T newData)
{
	// same idea as in getData:
	// if already owner, just set
	// otherwise, grab, set, release to ensure atomicity
	if(owner == currentThread){
		data = newData;
	}else{
		grab();
		data = newData;
		release();
	}
}
