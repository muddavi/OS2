//****************   FileIO Management   ************************
//Name: Alan Layfield		Class: OS II
//Date Due: 12/03/11		Professor: Yogi Dandass
//***************************************************************

#include "Thread.h"
#include "Memory.h"
#include "Semaphore.h"
#include "FileIO.h"
#include <string.h>

SEM HDDSem;
XStatus Status;
XSysAce SysAce;
PMETA PMeta;

void SysAceInit(void)
{
	Status = XSysAce_Initialize(&SysAce, XPAR_SYSACE_COMPACTFLASH_DEVICE_ID );

	if (Status != XST_SUCCESS)
	{
       xil_printf( "Error setting up Compact Flash.\r\n" );
       return;
	}

	XSysAce_ResetCfg(&SysAce);
	Status = LockSysAce(&SysAce);
	if (Status != XST_SUCCESS)
	{
       xil_printf( "Error locking CF.\r\n" );
       return;
	}
}

XStatus LockSysAce(XSysAce *InstancePtr)
{
  XASSERT_NONVOID(InstancePtr != NULL);
  XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);
 
  // Force the MPU lock. The lock will occur immediately.
  XSysAce_mOrControlReg(InstancePtr->BaseAddress, XSA_CR_LOCKREQ_MASK | XSA_CR_FORCELOCK_MASK);
 
  //See if the lock was granted.
  while (!XSysAce_mIsMpuLocked(InstancePtr->BaseAddress));
  return XST_SUCCESS;
}

Xuint32 importFileSystem(void)
{
	xil_printf("Initializing File System...\r\n\r\n");
	
	semWait(&HDDSem);
	
	void* location = KernMemAllocPages(NUM_HDD_ARRAY_NODE_PAGES);
	int i;
	//Write x amount of data to RAM
	for(i = 0; i < NUM_HDD_ARRAY_NODE_PAGES; i++)
	{
		while(!XSysAce_IsCFReady(&SysAce));

		Status = XSysAce_SectorRead(&SysAce, 0, 8, (char*)(((Xuint32)location) + (512*i)));
		if (Status != XST_SUCCESS)
			return -1;
		if(i%100 == 0)
			xil_printf("Loading File System: %d / %d\r\n", i, NUM_HDD_ARRAY_NODE_PAGES);
	}	
		
	semSignal(&HDDSem);
	
	xil_printf("\r\nFile System Initialized, location in RAM = 0x%x\r\n", location);
	
	if(location != 0)
		return (Xuint32)location;
	else
	{
		xil_printf("Error in initializing file system metadata in RAM.\r\n\r\n");
		return -1;
	}
}

int exportFileSystem(int fileHandle)
{
	xil_printf("Export FS\r\n");
	int page = (fileHandle*META_SIZE)/SIZE_PAGE;
	
	semWait(&HDDSem);
	
	//Write x amount of data to SysAce
	while(!XSysAce_IsCFReady(&SysAce));

	Status = XSysAce_SectorWrite(&SysAce, (page*8), 8, (char*)(((Xuint32)HDDMetaStart) + (page*8*512)));
	if (Status != XST_SUCCESS)
		return -1;
	
	semSignal(&HDDSem);
	
	return 0;
}

int bufferData(Xuint32 fileHandle)
{
	xil_printf("Buffer Data\r\n");
	PMETA PMeta;
	PMeta = (PMETA)((Xuint32)HDDMetaStart + fileHandle*8*512);
	
	if((fileHandle < 0) || (fileHandle > NUM_HDD_PAGES))
	{
		//FAILURE TO FIND FILE
		xil_printf("\r\n---In function: bufferData:\r\n");
		xil_printf("File not found: %s\r\n\r\n", fileHandle);
		return -1;
	}
	if(PMeta->deleted == 1)
	{
		//FILE ALREADY DELETED
		xil_printf("\r\n---In function: bufferData:\r\n");
		xil_printf("File is already deleted: %s\r\n\r\n", fileHandle);
		return -1;
	}
	
	char* bufferLocation;

	if(PMeta->bufferLoc == 0)
	{
		bufferLocation = (char*)KernMemAllocPages(1);
		PMeta->bufferLoc = bufferLocation;
		int success = exportFileSystem(fileHandle);
		if(success != 0)
		{
			xil_printf("Failure exporting metadata.");
			return -1;
		}
	}
	else bufferLocation = PMeta->bufferLoc;
	
	Xuint32 fileLocation = PMeta->fileLoc;
	
	semWait(&HDDSem);
	
	//Buffer data from HDD to RAM
	while(!XSysAce_IsCFReady(&SysAce));

	Status = XSysAce_SectorRead(&SysAce, fileLocation, 8, bufferLocation);
	if (Status != XST_SUCCESS)
		return -1;
		
	semSignal(&HDDSem);

	//Success!
	return 0;
}

int writeToDisk(Xuint32 fileHandle)
{
	xil_printf("Write to disk\r\n");
	PMETA PMeta;
	PMeta = (PMETA)((Xuint32)HDDMetaStart + fileHandle*8*512);
	
	if((fileHandle < 0) || (fileHandle > NUM_HDD_PAGES))
	{
		//FAILURE TO FIND FILE
		xil_printf("\r\n---In function: bufferData:\r\n");
		xil_printf("File not found: %s\r\n\r\n", fileHandle);
		return -1;
	}
	if(PMeta->deleted == 1)
	{
		//FILE ALREADY DELETED
		xil_printf("\r\n---In function: bufferData:\r\n");
		xil_printf("File is already deleted: %s\r\n\r\n", fileHandle);
		return -1;
	}
	
	char* bufferLocation = PMeta->bufferLoc;
	Xuint32 fileLocation = PMeta->fileLoc;
	
	semWait(&HDDSem);
	
	//Write file to SysAce
	while(!XSysAce_IsCFReady(&SysAce));

	Status = XSysAce_SectorWrite(&SysAce, fileLocation, 8, bufferLocation);
	if (Status != XST_SUCCESS)
		return -1;
			
	semSignal(&HDDSem);
	
	int success = exportFileSystem(fileHandle);
	if(success != 0)
	{
		xil_printf("Failure exporting metadata.");
		return -1;
	}
	
	//Success!
	return 0;
}

Xuint32 createFile(char* filename)
{
	xil_printf("Creating file: %s\r\n", filename);
	
	int fileHandle;
	for(fileHandle = 0; fileHandle < NUM_HDD_PAGES; fileHandle++)
	{
		PMETA PMeta;
		PMeta = (PMETA)((Xuint32)HDDMetaStart + fileHandle*8*512);
		if(PMeta->name == filename)
		{
			//FOUND A FILE
			xil_printf("\r\n---In function: createFile:\r\n");
			xil_printf("File already exists: %s\r\n\r\n", filename);
			return -1;
		}
	}
	for(fileHandle = 0; fileHandle < NUM_HDD_PAGES; fileHandle++)
	{
		PMETA PMeta;
		PMeta = (PMETA)((Xuint32)HDDMetaStart + fileHandle*8*512);
		if(*(PMeta->name) == NULL)
		{
			//FOUND AN EMPTY SLOT
			int i;
			for(i = 0; i < 31; i++)
			{
				(PMeta->name)[i] = filename[i];
			}
			PMeta->deleted = 0;
			PMeta->fileLoc = (((((Xuint32)((Xuint32)PMeta)) - ((Xuint32)HDDMetaStart))/META_SIZE)*SIZE_PAGE);
			PMeta->sizeOnDisk = SIZE_PAGE;
			PMeta->sizeOfData = 0;
			PMeta->bufferLoc = 0;
			
			exportFileSystem(fileHandle);
			return 0;
		}
	}
	
	//File System Full
	xil_printf("File System is full.\r\n");
	return -1;
}

int deleteFile(Xuint32 fileHandle)
{
	xil_printf("Delete file\r\n");
	PMETA PMeta;
	PMeta = (PMETA)((Xuint32)HDDMetaStart + fileHandle*8*512);
	
	if((fileHandle < 0) || (fileHandle > NUM_HDD_PAGES))
	{
		//FAILURE TO FIND FILE
		xil_printf("\r\n---In function: bufferData:\r\n");
		xil_printf("File not found: %s\r\n\r\n", fileHandle);
		return -1;
	}
	if(PMeta->deleted == 1)
	{
		//FILE ALREADY DELETED
		xil_printf("\r\n---In function: bufferData:\r\n");
		xil_printf("File is already deleted: %s\r\n\r\n", fileHandle);
		return -1;
	}
	
	PMeta->deleted = 1;
	
	exportFileSystem(fileHandle);
	
	//Success!
	xil_printf("\r\nFile deleted: %s\r\n\r\n", fileHandle);
	return 0;
}

int readData(Xuint32 fileHandle, int bytes)
{
	xil_printf("Read data\r\n");
	/*int success = bufferData(fileHandle);
	PMETA PMeta;
	PMeta = (PMETA)((Xuint32)HDDMetaStart + fileHandle*8*512);

	char* bufferLoc = PMeta->bufferLoc;*/
	
	if((bytes > 512) || (bytes < 1))
	{
		xil_printf("Invalid number of bytes to read!\r\n");
		return -1;
	}
	
	
	char buf[512];
	
	while(!XSysAce_IsCFReady(&SysAce));

	Status = XSysAce_SectorRead(&SysAce, 0, 8, (char*)buf);
	if (Status != XST_SUCCESS)
		return -1;
		
	int j;
	for(j = 0; j < bytes; j++)
	{
		char a = *(char*)(buf + j);
		xil_printf("%x", a);
	}
	xil_printf("\r\n");
	
	//Success!
	return 0;
}

int writeData(Xuint32 fileHandle, int numBytes, char* buf)
{
	xil_printf("Write data\r\n");
	//int success = bufferData(fileHandle);
	
	if((numBytes < 1) || (numBytes > 4096))
	{
		xil_printf("Invalid number of bytes");
		return -1;
	}
	
	semWait(&HDDSem);
	
	//Write file to SysAce
	while(!XSysAce_IsCFReady(&SysAce));

	Status = XSysAce_SectorWrite(&SysAce, 0, 8, buf);
	if (Status != XST_SUCCESS)
		return -1;
			
	semSignal(&HDDSem);
	
	//Success!
	//success = writeToDisk(fileHandle);
	//if(success != 0)
		//return sizeof(buf);
		
	return 0;
}

int resetHDD(int pages)
{
	xil_printf("Reset HDD\r\n");
	//Create cluster of all 0's
	void* start = KernMemAllocPages(1);
	int j;
	for(j = 0; j < 4096; j++)
	{
		((char*)start)[j] = NULL;
	}
	
	semWait(&HDDSem);
	//Write newly created cluster to all clusters to be cleared
	int i;
	for(i = 0; i < pages; i++)
	{		
		while(!XSysAce_IsCFReady(&SysAce));

		Status = XSysAce_SectorWrite(&SysAce, i, 8, (char*)start);
		if (Status != XST_SUCCESS)
			return -1;
			
		if(i%100 == 0)
			xil_printf("Deleting File System: %d / %d\r\n", i, pages);
	}
	
	semSignal(&HDDSem);

	return 0;
}
