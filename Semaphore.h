//*********************   Semaphores   **************************
//Name: Alan Layfield and David Mudd		Class: OS II
//Date Due: 11/10/11		Professor: Yogi Dandass
//***************************************************************

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

//***************************************************************
//*********************** INCLUDE FILES *************************
//***************************************************************
//XILINX CREATED FILES
//--------------------
#include "xutil.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xexception_l.h"
#include "xintc.h"
#include "xsysace.h"

//USER CREATED FILES
//------------------
#include "Thread.h"



//***************************************************************
//**************************** STRUCTS **************************
//***************************************************************
typedef struct //SEMAPHORE
{
	int countLock;
	int fifoLock;
	unsigned int count;
	LINKEDLIST fifo;
}	SEM, *PSEM;



//***************************************************************
//******************* GLOBAL VARIABLES **************************
//***************************************************************
extern SEM memSem;
extern SEM listSem;
extern SEM HDDSem;



//***************************************************************
//********************* SEMAPHORE FUNCTIONS *********************
//***************************************************************
int TestAndSet(int*, int);
void Spinlock(int*);
void semInit(PSEM, int);
void semWait(PSEM);
void semSignal(PSEM);

#endif
