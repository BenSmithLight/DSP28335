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

#include "DSP28x_Project.h" // Device Headerfile and Examples Include File

// Prototype statements for functions found within this file.
void scib_echoback_init(void);
void scib_fifo_init(void);
void scib_xmit(int a);
void scib_msg(char *msg);
// void sci_rx_isr(void);
__interrupt void scibRxFifoIsr(void);
__interrupt void scibTxFifoIsr(void);

// Global counts used in this example
// Uint16 LoopCount;
// Uint16 ErrorCount;
char sdataB[8]; // Send data for SCI-B
char rdataB[8]; // Received data for SCI-B
char *msg;

void main(void)
{

    // Step 1. 初始化系统控制:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the DSP2833x_SysCtrl.c file.
    InitSysCtrl();

    // Step 2. 初始化 GPIO:
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

    // Step 4. Initialize all the Device Peripherals（外围设备）:
    // This function is found in DSP2833x_InitPeripherals.c
    // InitPeripherals(); // Not required for this example
    EALLOW; // This is needed to write to EALLOW protected registers
    PieVectTable.SCIRXINTB = &scibRxFifoIsr;
    PieVectTable.SCITXINTB = &scibTxFifoIsr;
    EDIS; // This is needed to disable write to EALLOW protected registers

    // Step 5. User specific code:

    // LoopCount = 0;
    // ErrorCount = 0;
    // 串口初始化
    scib_fifo_init();     // Initialize the SCI FIFO
    scib_echoback_init(); // Initalize SCI for echoback

    msg = "\r\nWelcome to TL28335 SCIB Demo application.\n\0";
    scib_msg(msg);

    msg = "\r\nYou will enter a character, and the DSP will echo it back! \n\0";
    scib_msg(msg);                       // 发送
    ScibRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // 清除标志位
    // 初始化接收数据
    Uint16 i;
    for (i = 0; i < 8; i++)
    {
        sdataB[i] = 0;
    }

    // 使能PIE、CPU中断
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx3 = 1; // PIE Group 9, int1
    PieCtrlRegs.PIEIER9.bit.INTx4 = 1; // PIE Group 9, INT2
    IER |= M_INT9;                     // Enable CPU INT
    EINT;
    ERTM;
    while (1)
    {
    }
}

__interrupt void scibTxFifoIsr(void)
{
    //	msg = "\n\nDSP Send: ";
    //	scib_msg(msg);
    //	Uint16 i;
    //	for(i=0; i< 8; i++)
    //	{
    //	   ScibRegs.SCITXBUF=sdataB[i];     // Send data
    //	}

    // ScibRegs.SCIFFTX.bit.TXFFINTCLR=1;  // Clear SCI Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100; // Issue PIE ACK
    EINT;
}

__interrupt void scibRxFifoIsr(void)
{
    Uint16 i;
    // while(ScibRegs.SCIFFRX.bit.RXFFST !=1)// wait for XRDY =1 for empty state

    for (i = 0; i < 8; i++)
    {
        rdataB[i] = ScibRegs.SCIRXBUF.all;
    }
    if (rdataB[0] == 0xFF)
    {
        PieCtrlRegs.PIEIER6.bit.INTx5 = 1; // Enable PIE Group 6, INT 5
    }
    else if (rdataB[0] == 0xFE)
    {
        PieCtrlRegs.PIEIER6.bit.INTx5 = 0; // Enable PIE Group 6, INT 5
    }
    msg = "\n\nDSP Receive: ";
    scib_msg(msg);
    scib_msg(rdataB);

    ScibRegs.SCIFFTX.bit.TXFFINTCLR = 1;
    // SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;  // Clear Overflow flag
    ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= 0x100; // Issue PIE ack
}
// Test 1,SCIA  DLB, 8-bit word, baud rate 0x000F, default, 1 STOP bit, no parity
void scib_echoback_init()
{
    // Note: Clocks were turned on to the Scib 外部
    // in the InitSysCtrl() function

    ScibRegs.SCICCR.all = 0x0007;  // 1 stop bit,  No loopback
                                   // No parity,8 char bits,
                                   // async mode, idle-line protocol
    ScibRegs.SCICTL1.all = 0x0003; // enable TX, RX, internal SCICLK,
                                   // Disable RX ERR, SLEEP, TXWAKE
    ScibRegs.SCICTL2.all = 0x0003;
    ScibRegs.SCICTL2.bit.TXINTENA = 1;
    ScibRegs.SCICTL2.bit.RXBKINTENA = 1;
#if (CPU_FRQ_150MHZ)
    ScibRegs.SCIHBAUD = 0x0000; // 115200 baud @LSPCLK = 37.5MHz.（37500000/（8*115200）-1）
    ScibRegs.SCILBAUD = 0x0028;
#endif
#if (CPU_FRQ_100MHZ)
    ScibRegs.SCIHBAUD = 0x0000; // 115200 baud @LSPCLK = 20MHz.
    ScibRegs.SCILBAUD = 0x0015;
#endif
    ScibRegs.SCICTL1.all = 0x0023; // Relinquish SCI from Reset
}

// Transmit a character from the SCI
// 发送字节函数
void scib_xmit(int a)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
    {
    }
    ScibRegs.SCITXBUF = a;
}

// 发送数据包

// 发字符串
void scib_msg(char *msg)
{
    int i;
    i = 0;
    while (msg[i] != '\0')
    {
        scib_xmit(msg[i]);
        i++;
    }
}

// Initalize the SCI FIFO
void scib_fifo_init()
{
    ScibRegs.SCIFFTX.bit.SCIRST = 1;   // FIFO复位
    ScibRegs.SCIFFTX.bit.SCIFFENA = 1; // 使能FIFO
    // ScibRegs.SCIFFTX.bit.TXFFINTCLR = 1;  // 清除发送FIFO中断标志位
    ScibRegs.SCIFFTX.bit.TXFFIENA = 1; // 使能发送FIFO中断
    ScibRegs.SCIFFTX.bit.TXFFIL = 8;   // 发送FIFO中断级别

    // SciaRegs.SCIFFRX.all=0x0028;
    ScibRegs.SCIFFRX.bit.RXFFIENA = 1; // 使能接收FIFO中断
    ScibRegs.SCIFFRX.bit.RXFFIL = 8;   // 接收FIFO中断级别

    ScibRegs.SCIFFCT.all = 0x00; // FIFO传送延时为0

    ScibRegs.SCICTL1.bit.SWRESET = 1; // 重启SCIB

    ScibRegs.SCIFFTX.bit.TXFIFOXRESET = 1; // 重新使能发送FIFO的操作
    ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;  // 重新使能接收FIFO的操作

    // ScibRegs.SCIFFRX.all=0x204f;
    // ScibRegs.SCIFFCT.all=0x0;
}

//===========================================================================
// No more.
//===========================================================================
