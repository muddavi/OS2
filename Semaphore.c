//*********************   Semaphores   **************************
//Name: Alan Layfield		Class: OS II
//Date Due: 11/10/11		Professor: Yogi Dandass
//***************************************************************

#include "Semaphore.h"
#include "Thread.h"

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



//***************************************************************
//******************* SEMAPHORE FUNCTIONS ***********************
//***************************************************************
int TestAndSet(int *piLock, int iVal)
{
	asm("loop:\tlwarx 5,0,3\r\n\tcmpwi 5,0\r\n\tbne fail\r\n\tstwcx. 4,0,3\r\n\tbne loop\r\n\tli 3,1\r\n\tblr\r\nfail:");
	return 0;
}

void Spinlock(int *piLock)
{
	while(!TestAndSet(piLock,1));
}

void semInit(PSEM pSem, int iVal)
{
	pSem->count = iVal;
	//xil_printf("pSem->count (INIT) %d\r\n", pSem->count);
	pSem->countLock = 0;
	pSem->fifoLock = 0;
}

void semWait(PSEM pSem)
{
	//Edit Count
	
	Spinlock(&(pSem->countLock));
	pSem->count--;
	//xil_printf("pSem->count: %d\r\n", pSem->count);
	if(pSem->count >= 0)
	{
		pSem->countLock = 0;
		return;
	}
	pSem->countLock = 0;
	
	//Edit FIFO
	
	Spinlock(&(pSem->fifoLock));
	//Select Ready Thread To Be Next Running Thread
	PLINKEDLIST pLLOldRunningThread = pLLRunningList;
	KernUnlinkLL(&(pLLRunningList), pLLRunningList);
	KernAddLL(&(pLLRunningList), pLLReadyList);
	KernUnlinkLL(&(pLLReadyList), pLLReadyList);
	
	//Add This Thread To The Blocked List In This Sem's FIFO
	PLINKEDLIST fifoPtr = &(pSem->fifo);
	KernAddLL(&fifoPtr, pLLOldRunningThread);
	
	pSem->fifoLock = 0;
	SwitchAndGo(LIST_ENTRY_PTR(pLLOldRunningThread, TCB, llState), LIST_ENTRY_PTR(pLLRunningList, TCB, llState));
}

void semSignal(PSEM pSem)
{
	Spinlock(&(pSem->countLock));
	pSem->count++;
	if(pSem->count > 0)
	{
		pSem->countLock = 0;
		return;
	}
	pSem->countLock = 0;
	
	//Edit FIFO
	
	Spinlock(&(pSem->fifoLock));
	//Select Thread From FIFO
	PLINKEDLIST pLLOldBlockedThread = &(pSem->fifo);
	PLINKEDLIST fifoPtr = &(pSem->fifo);
	KernUnlinkLL(&(fifoPtr), &(pSem->fifo));
	
	//Add This Thread To Ready List
	KernAddLL(&(pLLReadyList), pLLOldBlockedThread);
	
	pSem->fifoLock = 0;
}
