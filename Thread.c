//****************   Thread Management   ************************
//Name: Alan Layfield and David Mudd		Class: OS II
//Date Due: 10/18/11		Professor: Yogi Dandass
//***************************************************************

#include "Thread.h"
#include "Memory.h"
#include "Semaphore.h"

//CAPTURE REGISTERS
#define CaptureRegs(r) asm ("stmw 0,%[a]\r\n\tmfmsr 0\r\n\tstw 0,%[b]\r\n\tmfcr 0\r\n\tstw 0,%[c]\r\n\tmflr 0\r\n\tstw 0,%[d]\r\n\tmfctr 0\r\n\tstw 0,%[e]\r\n\tlwz 0,%[f]\r\n\t" : [a] "=m" (r.auiGPR[0]), [b] "=m" (r.uiMSR), [c] "=m" (r.uiCR), [d] "=m" (r.uiLR), [e] "=m" (r.uiCTR) : [f] "m" (r.auiGPR[0]));

//SWITCH REGISTERS
#define SwitchRegs(n,o) asm ("stmw 0,%[a]\n\tmfmsr 0\n\tstw 0,%[b]\n\tmfcr 0\n\tstw 0,%[c]\n\tmflr 0\n\tstw 0,%[d]\n\tmfctr 0\n\tstw 0,%[e]\n\tlwz 0,%[f]\n\tlmw 5,%[g]\n\tlwz 0,%[h]\n\tmtmsr 0\n\tlwz 0,%[i]\n\tmtcr 0\n\tlwz 0,%[j]\n\tmtlr 0\n\tlwz 0,%[k]\n\tmtctr 0\n\tlwz 0,%[l]\n\tlwz 1,%[m]\n\tlwz 2,%[p]\n\tlwz 3,%[q]\n\tlwz 4,%[r]\n\t" : [a] "=m" (o.auiGPR[0]), [b] "=m" (o.uiMSR), [c] "=m" (o.uiCR), [d] "=m" (o.uiLR), [e] "=m" (o.uiCTR) : [f] "m" (o.auiGPR[0]), [g] "m" (n.auiGPR[5]), [h] "m" (n.uiMSR), [i] "m" (n.uiCR), [j] "m" (n.uiLR), [k] "m" (n.uiCTR), [l] "m" (n.auiGPR[0]), [m] "m" (n.auiGPR[1]), [p] "m" (n.auiGPR[2]), [q] "m" (n.auiGPR[3]), [r] "m" (n.auiGPR[4]));

void InitKernelProc(void)
{		
	//Allocate the page for the pcb
	void* PCBPageLoc = KernMemAllocPages(1);
	pPCBKern = (PPCB)PCBPageLoc;

	//Memset the pcb page to 0
	unsigned int i;
	for(i = (unsigned int)PCBPageLoc; i < (unsigned int)PCBPageLoc + SIZE_PAGE; i++)
	{
		*((unsigned char*)i) = 0x0;
	}

	//Assign a pid
	pPCBKern->pid = 1000 + pidCounter;
	pidCounter++;

	//Set the first thread as the currently running kernel thread
	
	#ifdef TESTING_THREAD_CREATION
	xil_printf("\r\nInitializing Kernel Thread...\r\n");
	#endif
	
	pPCBKern->aTCB[0].fUsed = 1;
	
	//Set Semaphore
	semWait(&listSem);

	KernAddLL(&(pLLRunningList), &(pPCBKern->aTCB[0].llState));
	
	//Release Semaphore
	semSignal(&listSem);
	
	#ifdef TESTING_THREAD_CREATION
	xil_printf("\r\nKernel Thread Initialized\r\n");
	xil_printf("\r\nKernel Thread PTR = 0x%x\r\n", (unsigned int) &(pPCBKern->aTCB[0]));
	#endif

	Interrupt_Init();
	//INTERRUPTS
	//set programmable timer interrupt

	//set timer interrupt handler

	//clear timer interrupt flag

	//enable timer interrupt
}

void ThreadCleanup(void) 
{
	/*xil_printf("ReadyList: 0x%x\r\n", (unsigned int) pLLReadyList);
	xil_printf("RunningList: 0x%x\r\n", (unsigned int) pLLRunningList);
	xil_printf("ReadyList->pNext: 0x%x\r\n", (unsigned int) pLLReadyList->pNext);*/
	
	PTCB pTCB = LIST_ENTRY_PTR(pLLRunningList, TCB, llState);	

	//Mark the tcb space as free
	pTCB->fUsed = 0;

	//Free stack space
	unsigned int firstMemPage = 0;
	firstMemPage = (unsigned int)(pTCB->pStackMemBase) - (SIZE_PAGE - 64) - (SIZE_PAGE) * (pTCB->iNumStackPages);
	int i;
	for(i = 0; i < pTCB->iNumStackPages; i++)
	{
		KernMemFreePage((void*)(firstMemPage + i * (SIZE_PAGE)));
	}

	//Set Semaphore
	semWait(&listSem);

	//xil_printf("Hello\r\n");
	
	PLINKEDLIST pLLOldThread = pLLRunningList;
	PLINKEDLIST pLLNewThread = pLLReadyList;
	
	//Remove currently running tcb fom the running list
	KernUnlinkLL(&pLLRunningList, pLLRunningList);

	//Select the next thread to run (without placing current thread back into ready list)
	KernUnlinkLL(&pLLReadyList, pLLReadyList);
	KernAddLL(&pLLRunningList, pLLNewThread);
	/*xil_printf("ReadyList: 0x%x\r\n", (unsigned int) pLLReadyList);
	xil_printf("RunningList: 0x%x\r\n", (unsigned int) pLLRunningList);
	xil_printf("ReadyList->pNext: 0x%x\r\n", (unsigned int) pLLReadyList->pNext);*/
	
	//Release Semaphore
	semSignal(&listSem);

	//xil_printf("Hello\r\n");
	
	SwitchAndGo(LIST_ENTRY_PTR(pLLOldThread, TCB, llState), LIST_ENTRY_PTR(pLLRunningList, TCB, llState));
}

int CreateKernelThread(PTHREADFN pThreadFn, Xuint32 uiThreadParam, int iPagesInStack)
{
	//Search for a free thread context in the kernel PCB
	int i = 0;
	for(i = 0; i < THREADS_PER_PROCESS; i++)
	{
		if(pPCBKern->aTCB[i].fUsed == 0)
			break;
	}

	//If we search and can't find an empty TCB, return 0
	if(i == THREADS_PER_PROCESS)
		return 0;

	//Mark the tcb as used
	pPCBKern->aTCB[i].fUsed = 1;

	//Remember the stack size
	pPCBKern->aTCB[i].iNumStackPages = iPagesInStack;

	//Capture all the addresses to setup the C context for the thread
	CaptureRegs(pPCBKern->aTCB[i].regs);

	//Allocate memory for stack
	void* memLoc = KernMemAllocPages(iPagesInStack);
	pPCBKern->aTCB[i].pStackMemBase = (unsigned char*)((unsigned int)memLoc + (SIZE_PAGE - 64) + (SIZE_PAGE) * (iPagesInStack - 1));

	//Setup the stack pointer register in the context
	pPCBKern->aTCB[i].regs.auiGPR[31] = (Xuint32)pPCBKern->aTCB[i].pStackMemBase;

	//If we didn't get memory, return 0
	if(memLoc == NULL)
		return 0;
	
	//The stack grows down, so be sure to set it up correctly
	//Remember stack pointer (record it in the context)
	pPCBKern->aTCB[i].regs.auiGPR[1] = (Xuint32)pPCBKern->aTCB[i].pStackMemBase;	

	//Set Semaphore
	semWait(&listSem);

	//Add the tcb to the ready list
	PLINKEDLIST pLLTCB = &(pPCBKern->aTCB[i].llState);
	KernAddLL(&pLLReadyList, pLLTCB);

	//Release Semaphore
	semSignal(&listSem);

	//The following two will be arguments passed to the ThreadManager function
	//that will call the the thread function and will also terminate the
	//thread once the thread function returns

	//Setup the thread parameter
	pPCBKern->aTCB[i].regs.auiGPR[3] = (Xuint32)uiThreadParam;
	// Setup the thread function address
	pPCBKern->aTCB[i].regs.auiGPR[4] = (Xuint32)pThreadFn;
	// Setup the address of the thread manager function. Execution will be
	// returned to the address in the LR in SwitchAndGo
	pPCBKern->aTCB[i].regs.uiLR = (Xuint32)ThreadManager;
	
	xil_printf("\r\nThread %d PTR = 0x%x\r\n", i, (unsigned int) &(pPCBKern->aTCB[i]));
	
	return 1;
}

void ThreadManager(Xuint32 uiThreadParam, PTHREADFN pThreadFn)
{
	xil_printf("Thread Manager is starting the thread %08x\r\n", (Xuint32)pThreadFn);

	(*pThreadFn)((void*)uiThreadParam);

	xil_printf("Thread Manager is terminating the thread %08x\r\n", (Xuint32)pThreadFn);
	
	   /*xil_printf("****************BtC*************\r\n");
		DumpRegs(&(pPCBKern->aTCB[0].regs));
		DumpRegs(&(pPCBKern->aTCB[1].regs));
		DumpRegs(&(pPCBKern->aTCB[2].regs));*/
	ThreadCleanup();
	   /*xil_printf("****************AtC*************\r\n");
		DumpRegs(&(pPCBKern->aTCB[0].regs));
		DumpRegs(&(pPCBKern->aTCB[1].regs));
		DumpRegs(&(pPCBKern->aTCB[2].regs));*/

}

//Print registers to the screen
void DumpRegs(PREGS pRegs)
{
	xil_printf("R0-7: %08x %08x %08x %08x %08x %08x %08x %08x\r\r\n",
	pRegs->auiGPR[0], pRegs->auiGPR[1], pRegs->auiGPR[2], pRegs->auiGPR[3],
	pRegs->auiGPR[4], pRegs->auiGPR[5], pRegs->auiGPR[6], pRegs->auiGPR[7]);
	xil_printf("G8-15: %08x %08x %08x %08x %08x %08x %08x %08x\r\r\n",
	pRegs->auiGPR[8], pRegs->auiGPR[9], pRegs->auiGPR[10], pRegs->auiGPR[11],
	pRegs->auiGPR[12], pRegs->auiGPR[13], pRegs->auiGPR[14], pRegs->auiGPR[15]);
	xil_printf("G16-23: %08x %08x %08x %08x %08x %08x %08x %08x\r\r\n",
	pRegs->auiGPR[16], pRegs->auiGPR[17], pRegs->auiGPR[18], pRegs->auiGPR[19],
	pRegs->auiGPR[20], pRegs->auiGPR[21], pRegs->auiGPR[22], pRegs->auiGPR[23]);
	xil_printf("R24-31: %08x %08x %08x %08x %08x %08x %08x %08x\r\r\n",
	pRegs->auiGPR[24], pRegs->auiGPR[25], pRegs->auiGPR[26], pRegs->auiGPR[27],
	pRegs->auiGPR[28], pRegs->auiGPR[29], pRegs->auiGPR[30], pRegs->auiGPR[31]);
	xil_printf("MSR: %08x, LR: %08x, CR: %08x, CTR: %08x\r\r\n",
	pRegs->uiMSR, pRegs->uiLR, pRegs->uiCR, pRegs->uiCTR);
}

void Yield()
{
	//Set Semaphore	
	semWait(&listSem);
	
	//xil_printf("Hello\r\n");
	
	PLINKEDLIST pLLOldThread;
	PLINKEDLIST pLLNewThread;
	
	pLLOldThread = pLLRunningList;
	pLLNewThread = pLLReadyList;

	PTCB pTCBOld = LIST_ENTRY_PTR(pLLRunningList, TCB, llState);
	PTCB pTCBNew = LIST_ENTRY_PTR(pLLReadyList, TCB, llState);

	// if the ready list isn't empty, switch threads
	if(pLLReadyList != NULL)
	{
		// Select the next thread
		// remove new thread from ready list
		KernUnlinkLL(&pLLReadyList, pLLNewThread);

		// remove old thread from the running list
		KernUnlinkLL(&pLLRunningList, pLLOldThread);

		// add the new thread to running list
		//xil_printf("pLLNewThread: 0x%x\r\n", (unsigned int) pLLNewThread);
		KernAddLL(&pLLRunningList, pLLNewThread);

		// add the old thread to the ready list
		//xil_printf("pLLOldThread: 0x%x\r\n", (unsigned int) pLLOldThread);
		KernAddLL(&pLLReadyList, pLLOldThread);
	}
	else 
	{
		//xil_printf("ReadyList: 0x%x\r\n", (unsigned int) pLLReadyList);
		
		//Release Semaphore
		semSignal(&listSem);
		
		//xil_printf("Hello\r\n");
		
		return;
	}
	
	// Save the registers for the current thread
	// The LR contains the address we want to return back to for
	// this current thread to continue
	
	//Release Semaphore
	semSignal(&listSem);
	
	//xil_printf("Hello\r\n");
	
	SwitchAndGo(pTCBOld, pTCBNew);
}


void SwitchAndGo(PTCB pOldTCB, PTCB pNewTCB)
{	
	// There should be no prologue/epilogue code as long as we don't call
	// functions or do much other processing here.
	// Following is a call to an asm macro:
	SwitchRegs(pNewTCB->regs, pOldTCB->regs);
}
