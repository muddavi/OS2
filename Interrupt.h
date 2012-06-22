//****************   Interrupt Management   *********************
//Name: Alan Layfield		Class: OS II
//Date Due: 11/10/11		Professor: Yogi Dandass
//***************************************************************

#ifndef INTERRUPT_H
#define INTERRUPT_H

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
#define INTR_ID 						0
#define BUTTON_CHANNEL 				1
#define GPIO_ALL_BUTTONS  			0xFFFF
#define BUTTON_INTERRUPT 			XGPIO_IR_CH1_MASK
#define GPIO_DEVICE_ID         	XPAR_PUSHBUTTONS_5BIT_DEVICE_ID
#define INTC_DEVICE_ID         	XPAR_OPB_INTC_0_DEVICE_ID

//***************************************************************
//********************* INTERRUPT FUNCTIONS *********************
//***************************************************************
void Interrupt_Init(void);
void GpioIsr(void*);
void TimerIsr(void*);

#endif
