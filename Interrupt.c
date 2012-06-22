//****************   Interrupt Management   *********************
//Name: Alan Layfield		Class: OS II
//Date Due: 11/10/11		Professor: Yogi Dandass
//***************************************************************

#include "Interrupt.h"

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

//***************************************************************
//********************** GLOBAL VARIABLES ***********************
//***************************************************************
XGpio Gpio; // The Instance of the GPIO Driver
XIntc Intc; // The Instance of the Interrupt Controller Driver

//***************************************************************
//******************* INTERRUPT FUNCTIONS ***********************
//***************************************************************
void Interrupt_Init(void)
{
	//PUSH BUTTON INTERRUPT
	/* Code */
	XIntc_Initialize(&Intc, INTC_DEVICE_ID);
	XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
	XIntc_Connect(&Intc, INTR_ID, GpioIsr, &Gpio);
	
	/* Buttons are all inputs */
	XGpio_SetDataDirection(&Gpio, BUTTON_CHANNEL, GPIO_ALL_BUTTONS);
	
	XGpio_InterruptEnable(&Gpio, BUTTON_INTERRUPT);
	XGpio_InterruptGlobalEnable(&Gpio);
	
	/* Enable the Interrupt vector at the interrupt controller */
	XIntc_Enable(&Intc, INTR_ID);
	
	XExc_Init();
	
	/*****************************************************************
	* Initialize the PPC405 exception table and register the interrupt
	* controller handler with the exception table
	******************************************************************/
	XExc_RegisterHandler(XEXC_ID_NON_CRITICAL_INT, (XExceptionHandler)XIntc_InterruptHandler, &Intc);
	/* Now Register Timer Interrupt */
	XExc_RegisterHandler(XEXC_ID_PIT_INT, (XExceptionHandler)TimerIsr, (void *)0);
	
		
	/* Enable non-critical exceptions in the PowerPC */
	XExc_mEnableExceptions(XEXC_NON_CRITICAL);
	
	/*******************************************************************
	* Start the interrput controller such that interrupts are recognized
	* and handled by the processor.
	********************************************************************/
	XIntc_Start(&Intc, XIN_REAL_MODE);
	
	//TIMER INTERRUPT
	/* Set Timer Interval (1 Sec) */
	XTime_PITSetInterval(0x0F000000);
	/* Automatically Restarts Timer */
	XTime_PITEnableAutoReload();
	/* Clear Timer Reg */
	XTime_TSRClearStatusBits(XREG_TSR_CLEAR_ALL);
	/* Clear Queued Timer Interrupts */
	XTime_PITClearInterrupt();
	/* Enable Interrupt */
	XTime_PITEnableInterrupt();	
}

void GpioIsr(void *InstancePtr)
{
	XGpio *GpioPtr = (XGpio *)InstancePtr;
	Xuint32 Led;
	Xuint32 LedState;
	Xuint32 Buttons;
	Xuint32 ButtonFound;
	Xuint32 ButtonsChanged = 0;
	static Xuint32 PreviousButtons;
	
	/***************************************************************
	* There should not be any other interrupts occurring other than
	* the button changes.
	***************************************************************/
	if((XGpio_InterruptGetStatus(GpioPtr) & BUTTON_INTERRUPT) != BUTTON_INTERRUPT)
		return;
	
	/***************************************************************
	* Read state of push buttons and determine which ones changed
	* states from the previous interrupt. Save a copy of the buttons
	* for the next interrupt.
	****************************************************************/
	Buttons = XGpio_DiscreteRead(GpioPtr, BUTTON_CHANNEL);
	
	// Clear the interrupt such that it is no longer pending in the GPIO
	
	(void) XGpio_InterruptClear(GpioPtr, BUTTON_INTERRUPT);
	
	// Do we need to clear the interrupt at the INTC?
	Yield();
	
}

void TimerIsr(void *InstancePtr)
{
	/* Clear Interrupt */
	XTime_PITClearInterrupt();
	/* Allow Next Thread To Run */
	Yield();
}
