//****************   FileIO Management   ************************
//Name: Alan Layfield		Class: OS II
//Date Due: 12/03/11		Professor: Yogi Dandass
//***************************************************************

#ifndef FILEIO_H
#define FILEIO_H

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

//***************************************************************
//**************************** STRUCTS **************************
//***************************************************************
typedef struct //METADATA ARRAY UNIT
{
	char name[31];
   Xuint32 deleted;
	Xuint32 fileLoc;
	Xuint32 sizeOnDisk;
	Xuint32 sizeOfData;
	char* bufferLoc;
} META, *PMETA;



//***************************************************************
//******************* GLOBAL VARIABLES **************************
//***************************************************************
#define HDD_SIZE 1073741824
#define META_SIZE 52
#define NUM_HDD_PAGES (HDD_SIZE/SIZE_PAGE)
#define NUM_HDD_ARRAY_NODE_PAGES ((NUM_HDD_PAGES*META_SIZE)/SIZE_PAGE)
void* HDDMetaStart;


//***************************************************************
//******************* FILEIO FUNCTIONS **************************
//***************************************************************
void SysAceInit(void); //Initializes SysAce
XStatus LockSysAce(XSysAce*); //Locks the SysAce feature (FileIO)
Xuint32 importFileSystem(void); //Import FS from disk to RAM
int exportFileSystem(int); //Export FS page from RAM to disk
int bufferData(Xuint32); //Buffers data from file to RAM
int writeToDisk(Xuint32); //Writes data from buffer to HDD
Xuint32 createFile(char*); //Creates a file
int deleteFile(Xuint32); //Deletes a file
int readData(Xuint32, int); //Read data from a file
int writeData(Xuint32, int, char*); //Write data to a file
int resetHDD(int); //Reset x number of clusters on disk to 0s

#endif
