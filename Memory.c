//****************   Memory Management   ************************
//Name: Alan Layfield and David Mudd		Class: OS II
//Date Due: 9/7/11		Professor: Yogi Dandass
//***************************************************************

#include "Memory.h"
#include "Semaphore.h"
#include <stdlib.h>
#include <stdio.h>

SEM memSem;
SEM listSem;



//***************************************************************
//************************** LL FUNCTIONS ***********************
//***************************************************************
void KernAddLL(PLINKEDLIST *ppLLHead, PLINKEDLIST pLLAdd) //Ready List PCBs
{
	//Next node points to old head node
	pLLAdd->pNext = *ppLLHead;

	//Set previous ptr of old head node
	pLLAdd->pNext->pPrev = pLLAdd;

	//Null out previous ptr
	pLLAdd->pPrev = NULL;

	//Head points to this node
	*ppLLHead = pLLAdd;


	return;
}

void KernUnlinkLL(PLINKEDLIST *ppLLHead, PLINKEDLIST pLLRem)
{
	//Next points to next node
	if(pLLRem->pPrev != NULL)
		pLLRem->pPrev->pNext = pLLRem->pNext;
	else
		*ppLLHead = pLLRem->pNext;

	//Rem is Head

	//Prev points to previous node
	if(pLLRem->pNext != NULL)
		pLLRem->pNext->pPrev = pLLRem->pPrev;

	return;
}

void KernInsertAfterLL(PLINKEDLIST *ppLLHead, PLINKEDLIST pLLAddAfterThis, PLINKEDLIST pLLAdd)
{
	//Set Next value of PLL to be added
	pLLAdd->pNext = pLLAddAfterThis->pNext;

	//Set Prev value of PLL to be added
	pLLAdd->pPrev = pLLAddAfterThis;

	//Set Next value of PLL added after
	pLLAddAfterThis->pNext = pLLAdd;

	return;
}

//***************************************************************
//********************** MEMORY FUNCTIONS ***********************
//***************************************************************
//Initialize LLs
void KernMemInit(void)
{	
	//Set Semaphore
	semWait(&memSem);	
	
	MEMSTARTLOC = (void *)DDR_BASE;

	#ifdef TESTING_MEMORY
	xil_printf("Memory Allocated.\r\nMemory Starting Location: 0x%x\r\n", (unsigned int)MEMSTARTLOC);
	#endif

	//Set pNext and pPrev for all Page Table Entries
	//Allocated LL
	unsigned int i;
	for(i = 0; i < NUM_PAGE_TABLE_PAGES; i = i + 1)
	{
		PMEMPAGE pMP = MEMSTARTLOC;
		(pMP + i)->data = 0;
		(pMP + i)->LLPages.pNext = &((pMP + i + 1)->LLPages);
		(pMP + i)->LLPages.pPrev = &((pMP + i - 1)->LLPages);

		if(i == 0)
			(pMP + i)->LLPages.pPrev = NULL;
		if(i == (NUM_PAGE_TABLE_PAGES-1))
			(pMP + i)->LLPages.pNext = NULL;
	}

	#ifdef TESTING_MEMORY
	xil_printf("Allocated Page List Completed. Free Page List Starting at: 0x%x\r\n", (unsigned int)(i * sizeof(MEMPAGE) + MEMSTARTLOC));
	#endif
	
	//Free LL
	for(i; i < NUM_PAGES; i = i + 1)
	{
		PMEMPAGE pMP = MEMSTARTLOC;
		(pMP + i)->data = 0;
		(pMP + i)->LLPages.pNext = &((pMP + i + 1)->LLPages);
		(pMP + i)->LLPages.pPrev = &((pMP + i - 1)->LLPages);

		if(i == NUM_PAGE_TABLE_PAGES)
		{
			(pMP + i)->LLPages.pPrev = NULL;
		}
		if(i == (NUM_PAGES-1))
			(pMP + i)->LLPages.pNext = NULL;
	}

	//Create Allocated List and Free List
	PMEMPAGE pMPAllocatedList = MEMSTARTLOC;
	pLLAllocatedList = &(pMPAllocatedList->LLPages);
	PMEMPAGE pMPFreeList = (MEMSTARTLOC + (NUM_PAGE_TABLE_PAGES * sizeof(MEMPAGE)));
	pLLFreeList = &(pMPFreeList->LLPages);

	#ifdef TESTING_MEMORY
	xil_printf("Free Page List Completed. End of Free Page List at: 0x%x\r\n", (unsigned int)(i * sizeof(MEMPAGE) + MEMSTARTLOC));
	#endif

	//Release Semaphore
	semSignal(&memSem);

	return;
}

//Allocate n pages, return base of alloc page block
void *KernMemAllocPages(int n)
{
	//Set Semaphore
	semWait(&memSem);
	
	PMEMPAGE pMP = LIST_ENTRY_PTR(pLLFreeList, MEMPAGE, LLPages);
	PMEMPAGE pFirstPage = pMP;
	unsigned int currentConsecPages = 1;

	while(pMP->LLPages.pNext != NULL)
	{	
		
		if(pMP->LLPages.pPrev == &((pMP - 1)->LLPages))
		{
			currentConsecPages = currentConsecPages + 1;
		}
		else 
		{
			currentConsecPages = 1;
			pFirstPage = pMP;
		}

		if(currentConsecPages == n)
		{
			PMEMPAGE i;
			for(i = pFirstPage; i <= pMP; i = i + 1)
			{
				#ifdef TESTING
				xil_printf("Page Allocated.\r\n");
				#endif				
				KernUnlinkLL(&pLLFreeList, &(i->LLPages));
			}

			PMEMPAGE j;			
			for(j = pFirstPage; j <= pMP; j = j + 1)
			{				
				KernAddLL(&pLLAllocatedList, &(j->LLPages));
			}	

			void* pP = CalcPagePointer(CalcPageIndex(&(pFirstPage->LLPages)));

			#ifdef TESTING
			xil_printf("%d Page(s) of Memory Allocated. Initial address: 0x%x\r\n", n, (unsigned int)pP);
			#endif

			//Release Semaphore
			semSignal(&memSem);
			
			return pP;	
		}
		pMP = LIST_ENTRY_PTR(pMP->LLPages.pNext, MEMPAGE, LLPages);
	}
	
	//Release Semaphore
	semSignal(&memSem);
	
	return NULL;
}

//Free page at ptr - RC 1 = success, RC 0 = failure
int KernMemFreePage(void *ptr)
{
	//Set Semaphore
	semWait(&memSem);

	PLINKEDLIST pLL = CalcLLPointer(CalcPageIndexPP(ptr));

	PMEMPAGE pMP = LIST_ENTRY_PTR(pLLAllocatedList, MEMPAGE, LLPages);
	while(pMP->LLPages.pNext != NULL)
	{
		if(&(pMP->LLPages) == pLL)
		{			
			KernUnlinkLL(&(pLLAllocatedList), &(pMP->LLPages));

			KernAddLL(&(pLLFreeList), &(pMP->LLPages));

			#ifdef TESTING
			xil_printf("1 Page of Memory Freed. Address: 0x%x\r\n", (unsigned int)ptr);
			#endif
	
			//Release Semaphore
			semSignal(&memSem);
	
			return 1;
		}
		pMP = LIST_ENTRY_PTR(pMP->LLPages.pNext, MEMPAGE, LLPages);
	}

	#ifdef TESTING
	xil_printf("Failure freeing 1 Page of Memory. Address: 0x%x\r\n", (unsigned int)ptr);
	#endif

	//Release Semaphore
	semSignal(&memSem);

	return 0;
}

//***************************************************************
//******************* CALCULATION FUNCTIONS *********************
//***************************************************************
//Calculate Page Index from LL Pointer
unsigned int CalcPageIndex(PLINKEDLIST pLL)
{
	unsigned int PAGE_INDEX = (void*)pLL - MEMSTARTLOC;
	PAGE_INDEX = PAGE_INDEX / sizeof(MEMPAGE);

	return PAGE_INDEX;
}

//Calculate Page Pointer
void *CalcPagePointer(unsigned int PAGE_INDEX)
{	
	unsigned int pP;
	pP = (PAGE_INDEX * SIZE_PAGE) + (unsigned int)MEMSTARTLOC;

	return (void*)pP;
}

//Calculate Page Index from Page Pointer
unsigned int CalcPageIndexPP(PMEMPAGE pP)
{
        unsigned int PAGE_INDEX = (void*)pP - MEMSTARTLOC;
        PAGE_INDEX = PAGE_INDEX / SIZE_PAGE;

        return PAGE_INDEX;
}

//Calculate LL Pointer
void *CalcLLPointer(unsigned int PAGE_INDEX)
{
	PLINKEDLIST pLL = (PAGE_INDEX * sizeof(MEMPAGE)) + MEMSTARTLOC;

	return (void*)pLL;
}
