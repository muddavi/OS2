//****************   OS2 Main Management   **********************
//Name: Alan Layfield and David Mudd		Class: OS II
//Date Due: 9/7/11		Professor: Yogi Dandass
//***************************************************************


//#define TESTING_MEMORY 1
//#define TESTING_SHOW_MEM_PAGE_LIST_ADDR 1
//#define TESTING_THREAD_CREATION 1
//#define WIPE_MEMORY 1


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

//USER CREATED FILES
//------------------
#include "Memory.h"
#include "Thread.h"
#include "Interrupt.h"
#include "Semaphore.h"
#include "FileIO.h"

//***************************************************************
//************************* TESTING *****************************
//***************************************************************
int main()
{
	int testing = 0;
	void * tmp;

	semInit(&memSem, 1);
	semInit(&listSem, 1);
	semInit(&HDDSem, 1);
	
	xil_printf("HDDSem->count (MAIN) %d\r\n", HDDSem.count);
	
	/*
	xil_printf("memSem->count (MAIN) %d\r\n", memSem.count);
	xil_printf("listSem->count (MAIN) %d\r\n", listSem.count);
	*/
	
	//*********************************************************************
	//*************************  MEMORY TESTING  **************************
	//*********************************************************************
	//INITIALIZATION
	KernMemInit();
	
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	
	//TEST 1 PAGE ALLOCATION AND FREE
	#ifdef TESTING_MEMORY
	xil_printf("\r\nThe following test will attempt a 1 page allocation and free.\r\n");
	tmp = KernMemAllocPages(1);
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage(tmp);
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	
	//TEST 2 PAGE ALLOCATION AND FREE
	#ifdef TESTING_MEMORY
	xil_printf("\r\nThe following test will attempt a 2 page allocation and free.\r\n");
	tmp = KernMemAllocPages(2);
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp+SIZE_PAGE));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	xil_printf("\r\nTHIS NEXT TEST SHOULD INTENTIONALLY FAIL.\r\n");
	testing = KernMemFreePage((tmp));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	
	//TEST 3 PAGE ALLOCATION AND FREE
	#ifdef TESTING_MEMORY
	xil_printf("\r\nThe following test will attempt a 3 page allocation and free.\r\n");
	tmp = KernMemAllocPages(3);
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp+SIZE_PAGE+SIZE_PAGE));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp+SIZE_PAGE));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif

	//TEST 3 PAGE ALLOCATION AND FREE 1 IN MIDDLE
	#ifdef TESTING_MEMORY
	xil_printf("\r\nThe following test will attempt a 3 page allocation. Then it will free the middle page and attempt to reallocate the middle page.\r\n");
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif	
	#ifdef TESTING_MEMORY
	tmp = KernMemAllocPages(3);
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp+SIZE_PAGE));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	tmp = KernMemAllocPages(1);
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif

	//TEST VARIOUS PAGE ALLOCATION AND FREES
	#ifdef TESTING_MEMORY
	xil_printf("\r\nThe following test will attempt to provide various page allocations and frees.\r\n");
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	tmp = KernMemAllocPages(5);
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp+(2*SIZE_PAGE)));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	void* tmp2 = KernMemAllocPages(3);
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp+SIZE_PAGE));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp+(3*SIZE_PAGE)));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp+(4*SIZE_PAGE)));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp2));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp2+SIZE_PAGE));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	#ifdef TESTING_MEMORY
	testing = KernMemFreePage((tmp2+(2*SIZE_PAGE)));
	#endif
	#ifdef TESTING_SHOW_MEM_PAGE_LIST_ADDR
	xil_printf("pLLAllocatedList: 0x%x\r\n", (unsigned int)pLLAllocatedList);
	xil_printf("pLLFreeList: 0x%x\r\n", (unsigned int)pLLFreeList);
	#endif
	
	
	#ifdef TESTING_MEMORY
	xil_printf("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
	#endif
	
	
	
	//*********************************************************************
	//************************* FILE SYSTEM  ******************************
	//*********************************************************************
	SysAceInit();
	//extern void* HDDMetaStart;
	//HDDMetaStart = (void*)importFileSystem();
	
	#ifdef WIPE_MEMORY
	resetHDD(NUM_HDD_ARRAY_NODE_PAGES);
	#endif
	
	Xuint32 fileHandle;
	//fileHandle = createFile("david");
	testing = writeData(fileHandle, 5, "world");
	testing = readData(fileHandle, 5);
	
	
	//*********************************************************************
	//*************************  THREAD TESTING  **************************
	//*********************************************************************
	
	void thread1 (void *prm)
	{
		int i = 0;
		for(i = 0; i < 999999999; i++)
		{
			xil_printf("Thread 1\r\n");
			//Yield();
		}
	}
	
	void thread2 (void *prm)
	{
		int i = 0;
		for(i = 0; i < 999999999; i++)
		{
			xil_printf("Thread 2\r\n");
			//Yield();
		}
	}

	InitKernelProc();
	
	testing = CreateKernelThread(thread1,1,1);
	if(testing == 0)
		xil_printf("ERROR in Kernel Thread Creation.\r\n");
		
	//xil_printf("ReadyList: 0x%x\r\n", (unsigned int) pLLReadyList);
		
	testing = CreateKernelThread(thread2,1,1);
	if(testing == 0)
		xil_printf("ERROR in Kernel Thread Creation.\r\n");
		
	/*xil_printf("ReadyList: 0x%x\r\n", (unsigned int) pLLReadyList);
	xil_printf("ReadyList->pNext: 0x%x\r\n", (unsigned int) pLLReadyList->pNext);
	
	xil_printf("PThread0 LL: 0x%x\r\n", (unsigned int) &(pPCBKern->aTCB[0].llState));
	xil_printf("PThread1 LL: 0x%x\r\n", (unsigned int) &(pPCBKern->aTCB[1].llState));
	xil_printf("PThread2 LL: 0x%x\r\n", (unsigned int) &(pPCBKern->aTCB[2].llState));
	
	xil_printf("PThread1 TFN: 0x%x\r\n", (unsigned int) thread1);
	xil_printf("PThread2 TFN: 0x%x\r\n", (unsigned int) thread2);
	
	
	xil_printf("\r\n\r\nTHREAD0\r\n\r\n");
	DumpRegs(&(pPCBKern->aTCB[0].regs));
	xil_printf("ReadyList: 0x%x\r\n", (unsigned int) pLLReadyList);
	xil_printf("\r\n\r\nTHREAD1\r\n\r\n");
	DumpRegs(&(pPCBKern->aTCB[1].regs));
	xil_printf("ReadyList: 0x%x\r\n", (unsigned int) pLLReadyList);
	xil_printf("\r\n\r\nTHREAD2\r\n\r\n");
	DumpRegs(&(pPCBKern->aTCB[2].regs));
	xil_printf("ReadyList: 0x%x\r\n", (unsigned int) pLLReadyList);
	*/
	
	
	
	//*********************************************************************
	//*************** INFINITE LOOP FOR KERNEL THREAD  ********************
	//*********************************************************************
	for(;;) 
	{
		xil_printf("Thread 0\r\n");
		//Yield();
	}
	
	return 0;
}
