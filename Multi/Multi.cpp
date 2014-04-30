#include "Multi.h"

extern "C" {

Thread mainThread;
Thread *currentThread = &mainThread;

void initMulti()
{
	// initialize our main thread
	mainThread.started = true;
	mainThread.stack_size = 0; // mainThread's stack is the reference for the other threads
	mainThread.saved_stack = NULL;
	mainThread.next = NULL;
	// start up our timer
	setupTimer();
}

void setupTimer()
{
	// timer.h has this VERY useful macro
	ConfigIntTimer3(T3_INT_ON | T3_INT_PRIOR_3 | T3_INT_SUB_PRIOR_2);
}

void startMulti()
{
	// timer.h has this VERY useful macro
	OpenTimer3(T3_ON | T3_PS_1_256, 3120);
}

void stopMulti()
{
	OpenTimer3(T3_OFF | T3_PS_1_256, 3120);
}

unsigned int addThread(void (*func)(void))
{
	Thread *newThread = (Thread *)malloc(sizeof(Thread));
	// Add thread to our linked list of threads
	newThread->next = mainThread.next;
	mainThread.next = newThread;
	
	// Setup some of the initial variables, the registers will be used in contextSwitch
	newThread->stack_size = 0;
	newThread->saved_stack = NULL;
	newThread->started = false;
	newThread->function = func;
}

// Here you can see the reason why it is inline and not 
// written in the functions that need it.
inline void saveRegisters()
{
	asm volatile("sw $at, %[at] \n"
				 : [at] "=m" (currentThread->at));
	asm volatile("sw $v0, %[v0] \n"
				 "sw $v0, %[v1] \n"
				 : [v0] "=m" (currentThread->v0), 
				   [v1] "=m" (currentThread->v1));
	asm volatile("sw $a0, %[a0] \n"
				 "sw $a1, %[a1] \n"
				 "sw $a2, %[a2] \n"
				 "sw $a3, %[a3] \n"
				 : [a0] "=m" (currentThread->a0), 
				   [a1] "=m" (currentThread->a1), 
				   [a2] "=m" (currentThread->a2), 
				   [a3] "=m" (currentThread->a3));
	asm volatile("sw $t0, %[t0] \n"
				 "sw $t1, %[t1] \n"
				 "sw $t2, %[t2] \n"
				 "sw $t3, %[t3] \n"
				 "sw $t4, %[t4] \n"
				 "sw $t5, %[t5] \n"
				 "sw $t6, %[t6] \n"
				 "sw $t7, %[t7] \n"
				 : [t0] "=m" (currentThread->t0),
				   [t1] "=m" (currentThread->t1),
				   [t2] "=m" (currentThread->t2),
				   [t3] "=m" (currentThread->t3),
				   [t4] "=m" (currentThread->t4),
				   [t5] "=m" (currentThread->t5),
				   [t6] "=m" (currentThread->t6),
				   [t7] "=m" (currentThread->t7));
	asm volatile("sw $s0, %[s0] \n"
				 "sw $s1, %[s1] \n"
				 "sw $s2, %[s2] \n"
				 "sw $s3, %[s3] \n"
				 "sw $s4, %[s4] \n"
				 "sw $s5, %[s5] \n"
				 "sw $s6, %[s6] \n"
				 "sw $s7, %[s7] \n"
				 : [s0] "=m" (currentThread->s0),
				   [s1] "=m" (currentThread->s1),
				   [s2] "=m" (currentThread->s2),
				   [s3] "=m" (currentThread->s3),
				   [s4] "=m" (currentThread->s4),
				   [s5] "=m" (currentThread->s5),
				   [s6] "=m" (currentThread->s6),
				   [s7] "=m" (currentThread->s7));
	asm volatile("sw $t8, %[t8] \n"
				 "sw $t9, %[t9] \n"
				 : [t8] "=m" (currentThread->t8),
				   [t9] "=m" (currentThread->t9));
	asm volatile("sw $k0, %[k0] \n"
				 "sw $k1, %[k1] \n"
				 : [k0] "=m" (currentThread->k0),
				   [k1] "=m" (currentThread->k1));
	asm volatile("sw $gp, %[gp] \n"
				 //"sw $sp, %[sp] \n" // The stack pointer has to be handled differently
				 "sw $s8, %[s8] \n"
				 "sw $ra, %[ra] \n"
				 : [gp] "=m" (currentThread->gp),
				   [sp] "=m" (currentThread->sp),
				   [s8] "=m" (currentThread->s8),
				   [ra] "=m" (currentThread->ra));
}

inline void loadRegisters()
{
	asm volatile("lw $at, %[at] \n"
				 : : [at] "m" (currentThread->at));
	asm volatile("lw $v0, %[v0] \n"
				 "lw $v0, %[v1] \n"
				 : : [v0] "m" (currentThread->v0), 
				     [v1] "m" (currentThread->v1));
	asm volatile("lw $a0, %[a0] \n"
				 "lw $a1, %[a1] \n"
				 "lw $a2, %[a2] \n"
				 "lw $a3, %[a3] \n"
				 : : [a0] "m" (currentThread->a0), 
				     [a1] "m" (currentThread->a1), 
				     [a2] "m" (currentThread->a2), 
				     [a3] "m" (currentThread->a3));
	asm volatile("lw $t0, %[t0] \n"
				 "lw $t1, %[t1] \n"
				 "lw $t2, %[t2] \n"
				 "lw $t3, %[t3] \n"
				 "lw $t4, %[t4] \n"
				 "lw $t5, %[t5] \n"
				 "lw $t6, %[t6] \n"
				 "lw $t7, %[t7] \n"
				 : : [t0] "m" (currentThread->t0),
				     [t1] "m" (currentThread->t1),
				     [t2] "m" (currentThread->t2),
				     [t3] "m" (currentThread->t3),
				     [t4] "m" (currentThread->t4),
				     [t5] "m" (currentThread->t5),
				     [t6] "m" (currentThread->t6),
				     [t7] "m" (currentThread->t7));
	asm volatile("lw $s0, %[s0] \n"
				 "lw $s1, %[s1] \n"
				 "lw $s2, %[s2] \n"
				 "lw $s3, %[s3] \n"
				 "lw $s4, %[s4] \n"
				 "lw $s5, %[s5] \n"
				 "lw $s6, %[s6] \n"
				 "lw $s7, %[s7] \n"
				 : : [s0] "m" (currentThread->s0),
				     [s1] "m" (currentThread->s1),
				     [s2] "m" (currentThread->s2),
				     [s3] "m" (currentThread->s3),
				     [s4] "m" (currentThread->s4),
				     [s5] "m" (currentThread->s5),
				     [s6] "m" (currentThread->s6),
				     [s7] "m" (currentThread->s7));
	asm volatile("lw $t8, %[t8] \n"
				 "lw $t9, %[t9] \n"
				 : : [t8] "m" (currentThread->t8),
				     [t9] "m" (currentThread->t9));
	asm volatile("lw $k0, %[k0] \n"
				 "lw $k1, %[k1] \n"
				 : : [k0] "m" (currentThread->k0),
				     [k1] "m" (currentThread->k1));
	asm volatile("lw $gp, %[gp] \n"
				 //"lw $at, %[sp] \n" // The stack pointer has to be handled carefully.
				 "lw $s8, %[s8] \n"
				 "lw $ra, %[ra] \n"
				 : : [gp] "m" (currentThread->gp),
				     [sp] "m" (currentThread->sp),
				     [s8] "m" (currentThread->s8),
				     [ra] "m" (currentThread->ra));
}

// This is called from the timer interrupt, thus it should be using EPC.
void __ISR(_TIMER_3_VECTOR, ipl3) T3Handler(void)
{
	// save the return point from the interrupt
	asm volatile("mfc0 $at, $14 \n"
				 "sw $at, %[PC] \n"
				 : [PC] "=m" (currentThread->pc));
	// clear the interrupt flag
	IFS0CLR = 1<<12;
	WriteTimer3(0);
	// The compiler puts EPC on the stack, we are going to have to use it. Hopefully it its
	// stack placement doesn't change in other compiles. [12(sp)]
	asm volatile("la $at, CSWITCH \n"
				 "sw $at, 12($sp) \n");
}

// CSWITCH:
void contextSwitch()
{
	// save the registers we got from the interruption.
	saveRegisters();
	// sp must be handled specially, add 40 due to this function's allocation of the stack
	asm volatile("addiu $at, $sp, 40 \n"
				 "sw $at, %[sp] \n"
				 : [sp] "=m" (currentThread->sp));
	// if we aren't the main thread, then we must store the stack.
	if(currentThread != &mainThread){
		// Since mainThread's stack is the reference, it will be higher in memory than 
		uint32_t sp_diff = mainThread.sp - currentThread->sp;
		// If things have been pushed onto the stack.
		if(sp_diff > 0){
			currentThread->stack_size = sp_diff;
			// since spdif is the difference in bytes already
			currentThread->saved_stack = (uint32_t *)malloc(sp_diff);
			memcpy(currentThread->saved_stack, (uint32_t *)currentThread->sp, sp_diff);
		}else{
			currentThread->stack_size = 0;
			currentThread->saved_stack = NULL;
		}
	}
	
	// Time to change threads
	if(currentThread->next == NULL){
		// If we are at the end, loop back to mainThread
		currentThread = &mainThread;
	}else{
		currentThread = currentThread->next;
	}
	
	// If we haven't started this thread yet, jump to it
	if(!currentThread->started){
		// we must deallocate the stack that this function allocated before jumping elsewhere
		asm volatile("addiu $sp, $sp, 40 \n"
					 "j START_THREAD \n");
	}
	// otherwise we are going to retrieve the state of the previous run.
	
	// here we load the stack back into memory, if there is anything to load
	if(currentThread->stack_size > 0){
		memcpy((uint32_t *)currentThread->sp, currentThread->saved_stack, currentThread->stack_size);
		// free the memory of the (soon to be) old memory
		free(currentThread->saved_stack);
		currentThread->saved_stack = NULL;
		currentThread->stack_size = 0;
	}
	
	// load the registers right before returning
	loadRegisters();
	// Jump to the place this thread was last.
	// and set the stack to where it was the 
	// last time this function ran.
	asm volatile("lw $sp, %[sp] \n"
				 "sw $at, -4($sp) \n" // temporarily store $at on top of the stack
				 "lw $at, %[pc] \n"
				 "jr $at \n"
				 "lw $at, -4($sp) \n" // grab $at back 
				 : : [pc] "m" (currentThread->pc),
					 [sp] "m" (currentThread->sp));
}

// START_THREAD:
void startThread()
{
	currentThread->started = true;
	// ensure the stack doesn't collide
	currentThread->function();
	// This kills the thread that dares to return.
	// Also, gotta keep that stack clean and in its proper spot.
	asm volatile("addiu $sp, $sp, 24 \n"
				 "j TDEATH \n");
}

// TDEATH:
void threadDeath()
{
	if(currentThread == &mainThread){
		// If this somehow happens, just return
		return;
	}
	// we need the thread before this one
	Thread *tmp = &mainThread;
	while(tmp->next != currentThread){
		tmp = tmp->next;
	}
	// relink the previous thread to point to
	// "our" next thread
	tmp->next = currentThread->next;
	// we don't need to free our stack stack, since it
	// was freed when this thread started.
	// but we will free the memory that this thread occupies
	free(currentThread);
	
	if(tmp->next == NULL){
		// If we are at the end, loop back to mainThread
		currentThread = &mainThread;
	}else{
		currentThread = tmp->next;
	}
	
	// If we haven't started this thread yet, jump to it
	if(!currentThread->started){
		// remember to clean the stack before jumping
		asm("addiu $sp, $sp, 32 \n"
			"j START_THREAD \n");
	}
	// otherwise we are going to retrieve the state of the previous run.
	
	// here we load the stack back into memory, if there is anything to load
	if(currentThread->stack_size > 0){
		memcpy((uint32_t *)currentThread->sp, currentThread->saved_stack, currentThread->stack_size);
		// free the memory of the (soon to be) old memory
		free(currentThread->saved_stack);
	}
	
	WriteTimer3(0);
	// load the registers right before returning
	loadRegisters();
	// Jump to the place this thread was last. And clean stack.
	asm volatile("addiu $sp, $sp, 32 \n"
				 "lw $at, %[pc] \n"
				 "jr $at \n"
				 : : [pc] "m" (currentThread->pc));
}

void yield()
{
	// reset timer3 and jump to contextSwitch
	WriteTimer3(0);
	// save pc as the return address of this function,
	// since in that case once this thread is resumed 
	// we jump to where this function would return to.
	// Also, jump to contextSwitch once that is done.
	asm volatile("sw $ra, %[pc] \n"
				 "j CSWITCH \n"
				 : [pc] "=m" (currentThread->pc));
}

} // extern "C"