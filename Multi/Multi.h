#ifndef MULTI_H
#define MULTI_H

// needed for the wonderful interrupt macros
#include <plib.h>
// needed for memcpy, didn't want to roll my own
#include <string.h>

// This is so people can use the simple Mutex class we have.
#include "utility/Mutex.h"

/*
This is a library to implement basic multi-threading.
Initially for the pic32, but should be ported to the Arduino not too long after.
*/

extern "C" {

struct Thread {
	// Our registers
	uint32_t at;
	
	uint32_t v0;
	uint32_t v1;
	
	uint32_t a0;
	uint32_t a1;
	uint32_t a2;
	uint32_t a3;
	
	uint32_t t0;
	uint32_t t1;
	uint32_t t2;
	uint32_t t3;
	uint32_t t4;
	uint32_t t5;
	uint32_t t6;
	uint32_t t7;
	
	uint32_t s0;
	uint32_t s1;
	uint32_t s2;
	uint32_t s3;
	uint32_t s4;
	uint32_t s5;
	uint32_t s6;
	uint32_t s7;
	
	uint32_t t8;
	uint32_t t9;
	
	uint32_t k0;
	uint32_t k1;
	
	uint32_t gp;
	uint32_t sp;
	uint32_t s8;
	uint32_t ra;
	
	// where in the code it is
	uint32_t pc;
	
	// This is to determine if we must initialize the thread
	bool started;
	
	// This is the function pointer to the function to run as a thread
	void (*function)(void);
	
	uint32_t stack_size;
	uint32_t *saved_stack;
	
	Thread *next;
};

// Our globals
// this always exists, thus it be in "permanent" program memory rather than the heap
extern Thread mainThread;
// The initially current thread is of course, main
extern Thread *currentThread;

// This function sets everything up for multi-threading
void initMulti();
// Sets up our timer interrupt, called in initMulti.
void setupTimer();
// Start the multithreading
void startMulti();
// and stop it
void stopMulti();

// Add a thread to our thread list
unsigned int addThread(void (*func)(void));

// These functions are simply to make other functions
// easier to read, and they are inlined since we need
// unmodified registers, otherwise weird things can happen.
inline void saveRegisters();
inline void loadRegisters();

// This is the principle function of this library.
// This is the function that is called every X ticks 
// and stores the current context before switching to the next thread.
void contextSwitch() asm("CSWITCH");

// This function is called to start up a new thread.
void startThread() asm("START_THREAD");

// This function is called when a thread exits
void threadDeath() asm("TDEATH");

// This function yields the current execution of a thread to other threads.
void yield();
}

#endif