//****************   Memory Management   ************************
//Name: Alan Layfield		Class: OS II
//Date Due: 9/7/11		Professor: Yogi Dandass
//***************************************************************

#ifndef MEMORY_H
#define MEMORY_H

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
//********** PREPROCESSOR CONSTANTS AND FUNCTIONS ***************
//***************************************************************
#define DDR_BASE 0x10000000
#define SIZE_DDR 536870912 //in Bytes (512 MB)
#define SIZE_PAGE 4096 //in Bytes (4KB) DO NOT SET LESS THAN 64 Bytes
#define NUM_PAGES ((SIZE_DDR) / (SIZE_PAGE))
#define NUM_PAGE_TABLE_PAGES (((NUM_PAGES * sizeof(MEMPAGE))) / SIZE_PAGE)
#define LIST_ENTRY_PTR(LLPTR, ENTRY_TYPE, LLMEMBER) ((ENTRY_TYPE *)((char*)LLPTR - (unsigned long)&(((ENTRY_TYPE *)0)->LLMEMBER)))
//****************** EXAMPLE USAGE OF LIST_ENTRY_PTR **************************
//******  pPCB pLLFirstPCB = LIST_ENTRY_PTR(pLLReadyPCBs, PCB, LLSched); ******
//*****************************************************************************

//***************************************************************
//**************************** STRUCTS **************************
//***************************************************************
typedef struct LLSTRUCT //LINKED LIST
{
	struct LLSTRUCT *pPrev;
	struct LLSTRUCT *pNext;
}	LINKEDLIST, *PLINKEDLIST;

typedef struct //Page Table Entries
{
	LINKEDLIST LLPages;
	unsigned int data;
}	MEMPAGE, *PMEMPAGE;



//***************************************************************
//******************* GLOBAL VARIABLES **************************
//***************************************************************
void* MEMSTARTLOC;
PLINKEDLIST pLLAllocatedList;
PLINKEDLIST pLLFreeList;



//***************************************************************
//********************* LINKEDLIST API **************************
//***************************************************************
void KernAddLL(PLINKEDLIST *ppLLHead, PLINKEDLIST pLLAdd); //Add PCB to front of Ready-State PCB LL
void KernUnlinkLL(PLINKEDLIST *ppLLHead, PLINKEDLIST pLLRem); //Remove PCB from Ready-State PCB LL
void KernInsertAfterLL(PLINKEDLIST *ppLLHead, PLINKEDLIST pLLAddAfterThis, PLINKEDLIST pLLAdd); //Insert PCB after SomePCB in PCB LL

//***************************************************************
//************************* MEMORY API **************************
//***************************************************************
void KernMemInit(void); //Initialize LLs
void * KernMemAllocPages(int n); //Allocate n pages, return base of alloc page block
int KernMemFreePage(void *ptr); //Free page at ptr - RC 1 = success, RC 0 = failure

//***************************************************************
//*********************** CALCULATION API ***********************
//***************************************************************
unsigned int CalcPageIndex(PLINKEDLIST); //Calculate Page Index from LL Pointer
void * CalcPagePointer(unsigned int); //Calculate Page Pointer
unsigned int CalcPageIndexPP(PMEMPAGE); //Calculate Page Index from Page Pointer
void * CalcLLPointer(unsigned int); //Calculate LL Pointer

#endif
