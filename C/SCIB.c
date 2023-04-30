/*
 * 			广州创龙电子科技有限公司
 *
 * Copyright 2015 Tronlong All rights reserved
 */

/*
 * 功能描述：
 *     使用Micro USB线连接开发板USB TO UARTB和PC机,上位机设置串口波特率
 * 为115200,DSP通过串口B接收到字符后，会通过串口B打印出来。
 *
 */

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

// Prototype statements for functions found within this file.
void scib_echoback_init(void);
void scib_fifo_init(void);
void scib_xmit(int a);
void scib_msg(char *msg);

// Global counts used in this example
Uint16 LoopCount;
Uint16 ErrorCount;

void main(void)
{

    Uint16 ReceivedChar;
    char *msg;

// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the DSP2833x_SysCtrl.c file.
   InitSysCtrl();

// Step 2. Initalize GPIO:
// This example function is found in the DSP2833x_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
   // InitGpio(); Skipped for this example

// For this example, only init the pins for the SCI-A port.
// This function is found in the DSP2833x_Sci.c file.
   InitScibGpio();

// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
   DINT;

// Initialize PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the DSP2833x_PieCtrl.c file.
   InitPieCtrl();

// Disable CPU interrupts and clear all CPU interrupt flags:
   IER = 0x0000;
   IFR = 0x0000;

// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in DSP2833x_DefaultIsr.c.
// This function is found in DSP2833x_PieVect.c.
   InitPieVectTable();

// Step 4. Initialize all the Device Peripherals:
// This function is found in DSP2833x_InitPeripherals.c
// InitPeripherals(); // Not required for this example

// Step 5. User specific code:

    LoopCount = 0;
    ErrorCount = 0;

    scib_fifo_init();	   // Initialize the SCI FIFO
    scib_echoback_init();  // Initalize SCI for echoback

    msg = "\r\nWelcome to TL28335 SCIB Demo application.\n\0";
    scib_msg(msg);

    msg = "\r\nYou will enter a character, and the DSP will echo it back! \n\0";
    scib_msg(msg);
    ScibRegs.SCIFFRX.bit.RXFFOVRCLR=1;

	for(;;)
    {
//       msg = "\r\nEnter a character: \0";
//       scib_msg(msg);

       // Wait for inc character
       while(ScibRegs.SCIFFRX.bit.RXFFST !=1) { } // wait for XRDY =1 for empty state

       // Get character
       ReceivedChar = ScibRegs.SCIRXBUF.all;

       // Echo character back
//       msg = "  You sent: \0";
//       scib_msg(msg);
       int tt = 1;
//       scib_xmit(ReceivedChar);
       scib_xmit(tt);

       LoopCount++;
    }

}


// Test 1,SCIA  DLB, 8-bit word, baud rate 0x000F, default, 1 STOP bit, no parity
void scib_echoback_init()
{
    // Note: Clocks were turned on to the Scib peripheral
    // in the InitSysCtrl() function

 	ScibRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
                                   // No parity,8 char bits,
                                   // async mode, idle-line protocol
	ScibRegs.SCICTL1.all =0x0003;  // enable TX, RX, internal SCICLK,
                                   // Disable RX ERR, SLEEP, TXWAKE
	ScibRegs.SCICTL2.all =0x0003;
	ScibRegs.SCICTL2.bit.TXINTENA =1;
	ScibRegs.SCICTL2.bit.RXBKINTENA =1;
	#if (CPU_FRQ_150MHZ)
	      ScibRegs.SCIHBAUD    =0x0000;  // 115200 baud @LSPCLK = 37.5MHz.
	      ScibRegs.SCILBAUD    =0x0028;
	#endif
	#if (CPU_FRQ_100MHZ)
      ScibRegs.SCIHBAUD    =0x0000;  // 115200 baud @LSPCLK = 20MHz.
      ScibRegs.SCILBAUD    =0x0015;
	#endif
	ScibRegs.SCICTL1.all =0x0023;  // Relinquish SCI from Reset
}

// Transmit a character from the SCI
void scib_xmit(int a)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScibRegs.SCITXBUF=a;
}

void scib_msg(char * msg)
{
    int i;
    i = 0;
    while(msg[i] != '\0')
    {
        scib_xmit(msg[i]);
        i++;
    }
}

// Initalize the SCI FIFO
void scib_fifo_init()
{
    ScibRegs.SCIFFTX.all=0xE040;
    ScibRegs.SCIFFRX.all=0x204f;
    ScibRegs.SCIFFCT.all=0x0;

}






//===========================================================================
// No more.
//===========================================================================

