//****************   Thread Management   ************************
//Name: Alan Layfield and David Mudd		Class: OS II
//Date Due: 10/18/11		Professor: Yogi Dandass
//***************************************************************

#ifndef THREAD_H
#define THREAD_H

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
#include "Memory.h"

//***************************************************************
//********** PREPROCESSOR CONSTANTS AND FUNCTIONS ***************
//***************************************************************
#define THREADS_PER_PROCESS 5

//***************************************************************
//**************************** STRUCTS **************************
//***************************************************************
typedef struct //REGISTER LIST
{
	Xuint32 volatile auiGPR[32];
	Xuint32 volatile uiMSR;
	Xuint32 volatile uiCR;
	Xuint32 volatile uiLR;
	Xuint32 volatile uiCTR;
}	REGS, *PREGS;

typedef struct //THREAD CONTEXT BLOCK
{
	unsigned char fUsed;
	unsigned char* pStackMemBase;
	int iNumStackPages;
	LINKEDLIST llState;
	REGS regs;
}	TCB, *PTCB;

typedef struct //PROCESS CONTROL BLOCK
{
	unsigned int pid;
	TCB aTCB[THREADS_PER_PROCESS];
}	PCB, *PPCB;

typedef void (*PTHREADFN) (void*);



//***************************************************************
//******************* GLOBAL VARIABLES **************************
//***************************************************************
PLINKEDLIST pLLReadyList; //Create Thread Ready List
PLINKEDLIST pLLRunningList; //Create Running Thread Pointer
PPCB pPCBKern;
int pidCounter;



//***************************************************************
//******************* THREAD FUNCTIONS **************************
//***************************************************************
void InitKernelProc(void);
void ThreadCleanup(void);
int CreateKernelThread(PTHREADFN, Xuint32, int);
void Yield(void);
void ThreadManager(Xuint32, PTHREADFN);
void DumpRegs(PREGS);
void SwitchAndGo(PTCB, PTCB);

#endif
