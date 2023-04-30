/*
 * 			广州创龙电子科技有限公司
 *
 * Copyright 2015 Tronlong All rights reserved
 */

/*
 * 功能描述：
 *     本例程将从Mic In接口输入的音频数据送到Line Out接口播放
 *
 */

#include "DSP2833x_Device.h"   // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h" // DSP2833x Examples Include File
#include "DSP28x_Project.h"    // Device Headerfile and Examples Include File

// 定义接收缓冲区和指针
#define RX_BUFF_SIZE 8
char rx_buffer[RX_BUFF_SIZE];
volatile Uint16 rx_index = 0;

// 使用前，声明本文件中的相关函数；
void I2CA_Init(void);
Uint16 AIC23Write(int Address, int Data);
void Delay(int time);
void delay();
interrupt void ISRMcbspSend();
interrupt void scib_rx_isr(void);

// Prototype statements for functions found within this file.
void scib_echoback_init(void);
void scib_fifo_init(void);
void scib_xmit(int a);
void scib_msg(char *msg);

// Global counts used in this example
Uint16 LoopCount;
Uint16 ErrorCount;
char *msg;
int temp1 = 0;
Uint16 ReceivedChar;

void main(void)
{

    InitSysCtrl();

    InitScibGpio();
    InitMcbspaGpio(); // zq
    InitI2CGpio();

    // Disable CPU interrupts
    DINT;

    InitPieCtrl();

    // Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;

    InitPieVectTable();

    LoopCount = 0;
    ErrorCount = 0;

    scib_fifo_init();     // Initialize the SCI FIFO
    scib_echoback_init(); // Initalize SCI for echoback

    //   msg = "\r\nWelcome to TL28335 SCIB Demo application.\n\0";
    //   scib_msg(msg);

    //   msg = "\r\nYou will enter a character, and the DSP will echo it back! \n\0";
    //   scib_msg(msg);
    ScibRegs.SCIFFRX.bit.RXFFOVRCLR = 1;

    // 配置接收缓冲区
    ScibRegs.SCIFFRX.all = 0x2060; // enable RX FIFO interrupt and clear FIFO
    ScibRegs.SCIFFRX.bit.RXFFIENA = 1;
    ScibRegs.SCIFFRX.bit.RXFFIL = 1;
    ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear RX FIFO interrupt flag
    // 启用SCI-B通道的接收中断
    ScibRegs.SCICTL2.bit.RXBKINTENA = 1;

    SCIStdioInit();
    //   SCIPuts("\r\n ============Test Start===========.\r\n", -1);
    //   SCIPuts("Welcome to TL28335 Audio Mic In Demo application.\r\n\r\n", -1);

    I2CA_Init();

    AIC23Write(0x00, 0x00); // 左线输入通道音量控制 00 17
    Delay(100);
    AIC23Write(0x02, 0x00); // 右线路输入通道音量控制 00 17
    Delay(100);
    AIC23Write(0x04, 0x60); // 左通道耳机音量控制 7f  79  70  60  30
    Delay(100);
    AIC23Write(0x06, 0x60); // 右通道耳机音量控制 7f  79  70  60  30
    Delay(100);
    AIC23Write(0x08, 0xF5); // 模拟音频路径控制 （麦克风输入，关闭静音，关闭增强）15  F5  35
    Delay(100);
    AIC23Write(0x0A, 0x00); // 数字音频路径控制 （关闭去加重控制）
    Delay(100);
    AIC23Write(0x0C, 0x00); // 掉电控制 （全部开启）
    Delay(100);
    AIC23Write(0x0E, 0x43); // 数字音频接口格式 （主模式，16bit输入，DSP 格式，帧同步后跟两个数据字）
    Delay(100);
    AIC23Write(0x10, 0x23); // 采样率控制 （44.1kHz）
    Delay(100);
    AIC23Write(0x12, 0x01); // 数字接口激活
    Delay(100);             // AIC23Init

    InitMcbspa(); // Initalize the Mcbsp-A

    EALLOW; // This is needed to write to EALLOW protected registers
    PieVectTable.MRINTA = &ISRMcbspSend;
    PieVectTable.SCIRXINTB = &scib_rx_isr;
    EDIS; // This is needed to disable write to EALLOW protected registers

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // Enable the PIE block
    PieCtrlRegs.PIEIER6.bit.INTx5 = 0; // Enable PIE Group 6, INT 5
    IER |= M_INT6;                     // Enable CPU INT6

    PieCtrlRegs.PIEIER9.bit.INTx3 = 1; // enable SCI-B RX interrupt
    IER |= M_INT9;

    EINT; // Enable Global interrupt INTM
    ERTM; // Enable Global realtime interrupt DBGM

    while (1)
    {
        //	       msg = "\r\nEnter a character: \0";
        //	       scib_msg(msg);

        // Wait for inc character
        while (ScibRegs.SCIFFRX.bit.RXFFST != 1)
        {
        } // wait for XRDY =1 for empty state

        // Get character
        ReceivedChar = ScibRegs.SCIRXBUF.all;

        if (ReceivedChar == 0xFF)
        {
            PieCtrlRegs.PIEIER6.bit.INTx5 = 1;
        }

        // Echo character back
        //	       msg = "  You sent: \0";
        //	       scib_msg(msg);
        //	       scib_xmit(ReceivedChar);

        //	       LoopCount++;
        //		scib_xmit(temp1>>8); // 发送高8位
        //		scib_xmit(temp1);    // 发送低8位
    }
} // end of main

// Test 1,SCIA  DLB, 8-bit word, baud rate 0x000F, default, 1 STOP bit, no parity
void scib_echoback_init()
{
    // Note: Clocks were turned on to the Scib peripheral
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
    ScibRegs.SCIHBAUD = 0x0000; // 115200 baud @LSPCLK = 37.5MHz.
    ScibRegs.SCILBAUD = 0x0028;
#endif
#if (CPU_FRQ_100MHZ)
    ScibRegs.SCIHBAUD = 0x0000; // 115200 baud @LSPCLK = 20MHz.
    ScibRegs.SCILBAUD = 0x0015;
#endif
    ScibRegs.SCICTL1.all = 0x0023; // Relinquish SCI from Reset
}

// Transmit a character from the SCI
void scib_xmit(int a)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
    {
    }
    ScibRegs.SCITXBUF = a;
}

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
    ScibRegs.SCIFFTX.all = 0xE040;
    ScibRegs.SCIFFRX.all = 0x204f;
    ScibRegs.SCIFFCT.all = 0x0;
}
void I2CA_Init(void)
{
    // Initialize I2C
    I2caRegs.I2CSAR = 0x001A; // Slave address - EEPROM control code

#if (CPU_FRQ_150MHZ)          // Default - For 150MHz SYSCLKOUT
    I2caRegs.I2CPSC.all = 14; // Prescaler - need 7-12 Mhz on module clk (150/15 = 10MHz)
#endif
#if (CPU_FRQ_100MHZ)         // For 100 MHz SYSCLKOUT
    I2caRegs.I2CPSC.all = 9; // Prescaler - need 7-12 Mhz on module clk (100/10 = 10MHz)
#endif

    I2caRegs.I2CCLKL = 100;     // NOTE: must be non zero
    I2caRegs.I2CCLKH = 100;     // NOTE: must be non zero
    I2caRegs.I2CIER.all = 0x24; // Enable SCD & ARDY interrupts

    //   I2caRegs.I2CMDR.all = 0x0020;	// Take I2C out of reset
    I2caRegs.I2CMDR.all = 0x0420; // Take I2C out of reset		//zq
                                  // Stop I2C when suspended

    I2caRegs.I2CFFTX.all = 0x6000; // Enable FIFO mode and TXFIFO
    I2caRegs.I2CFFRX.all = 0x2040; // Enable RXFIFO, clear RXFFINT,

    return;
}

Uint16 AIC23Write(int Address, int Data)
{
    if (I2caRegs.I2CMDR.bit.STP == 1)
    {
        return I2C_STP_NOT_READY_ERROR;
    }

    // Setup slave address
    I2caRegs.I2CSAR = 0x1A;

    // Check if bus busy
    if (I2caRegs.I2CSTR.bit.BB == 1)
    {
        return I2C_BUS_BUSY_ERROR;
    }

    // Setup number of bytes to send
    // MsgBuffer + Address
    I2caRegs.I2CCNT = 2;
    I2caRegs.I2CDXR = Address;
    I2caRegs.I2CDXR = Data;
    // Send start as master transmitter
    I2caRegs.I2CMDR.all = 0x6E20;
    return I2C_SUCCESS;
}

void Delay(int time)
{
    int i, j, k = 0;
    for (i = 0; i < time; i++)
        for (j = 0; j < 1024; j++)
            k++;
}

void delay(Uint32 k)
{
    while (k--)
        ;
}

interrupt void ISRMcbspSend(void)
{
    //	int temp1;
    //	int temp2;

    temp1 = McbspaRegs.DRR1.all; // 低16位
                                 //	temp2=McbspaRegs.DRR2.all;   // 高16位

    McbspaRegs.DXR1.all = temp1; // 放音
                                 //	McbspaRegs.DXR2.all = temp2;

    scib_xmit(temp1 >> 8); // 发送高8位
    scib_xmit(temp1);      // 发送低8位

    //	int tt = 1;
    //	scib_xmit(tt);

    PieCtrlRegs.PIEACK.all = 0x0020;
    //	PieCtrlRegs.PIEIFR6.bit.INTx5 = 0;
    //	ERTM;
    //    ReceivedChar = ScibRegs.SCIRXBUF.all;
    //
}

interrupt void scib_rx_isr(void)
{
    //    char rx_data = (char)ScibRegs.SCIRXBUF.all;
    ReceivedChar = ScibRegs.SCIRXBUF.all;

    // 检查缓冲区是否已满
    //    if (rx_index < RX_BUFF_SIZE)
    //    {
    //        // 存储接收到的数据
    //        rx_buffer[rx_index++] = rx_data;
    //    }
    //    else
    //    {
    //        // 缓冲区已满，重置指针
    //        rx_index = 0;
    //    }

    // 打印接收到的数据
    //    printf("Received data: %c\n", rx_data);

    // 清除中断标志
    //    PieCtrlRegs.PIEACK.all |= M_INT9;
    PieCtrlRegs.PIEACK.all |= M_INT9;
    ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear RX FIFO interrupt flag
}

//===========================================================================
// No more.
//===========================================================================
