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

#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "DSP28x_Project.h"

// 使用前，声明本文件中的相关函数；
void I2CA_Init(void);
Uint16 AIC23Write(int Address,int Data);
void Delay(int time);
void delay();
interrupt void  ISRMcbspSend();
//串口
void scib_echoback_init(void);
void scib_fifo_init(void);
void scib_xmit(int a);
void scib_msg(char *msg);
__interrupt void scibRxFifoIsr(void);
unsigned char crc8(unsigned char *buf, int len);
//__interrupt void scibTxFifoIsr(void);

char sdata[5];    // Send data for SCI-B
char rdataB[8] ;    // Received data for SCI-B
char *msg;
#define POLY 0x07   //生成多项式
int BAOTOU = 0xAB;
int BAOWEI = 0xEF;

void main(void)
{
   InitSysCtrl();//配置系统时钟

   InitMcbspaGpio();	//zq
   InitI2CGpio();
   InitScibGpio();

// Disable CPU interrupts
   DINT;

   InitPieCtrl();

// Disable CPU interrupts and clear all CPU interrupt flags:
   IER = 0x0000;
   IFR = 0x0000;

   InitPieVectTable();

   SCIStdioInit();
//   SCIPuts("\r\n ============Test Start===========.\r\n", -1);
//   SCIPuts("Welcome to TL28335 Audio Mic In Demo application.\r\n\r\n", -1);

   I2CA_Init();

	AIC23Write(0x00,0x00);
	Delay(100);
	AIC23Write(0x02,0x00);
	Delay(100);
	AIC23Write(0x04,0x7f);
	Delay(100);
	AIC23Write(0x06,0x7f);
	Delay(100);
	AIC23Write(0x08,0x14);
	Delay(100);
	AIC23Write(0x0A,0x00);
	Delay(100);
	AIC23Write(0x0C,0x00);
	Delay(100);
	AIC23Write(0x0E,0x43);
	Delay(100);
	AIC23Write(0x10,0x23);
	Delay(100);
	AIC23Write(0x12,0x01);
	Delay(100);		//AIC23Init

    InitMcbspa();          // Initalize the Mcbsp-A

    EALLOW;	// This is needed to write to EALLOW protected registers
	PieVectTable.MRINTA = &ISRMcbspSend;
	PieVectTable.SCIRXINTB = &scibRxFifoIsr;
	//PieVectTable.SCITXINTB = &scibTxFifoIsr;
	EDIS;   // This is needed to disable write to EALLOW protected registers
	 //串口初始化
	scib_fifo_init();	   // Initialize the SCI FIFO
	scib_echoback_init();  // Initalize SCI for echoback
	msg = "\r\nWelcome to TL28335 SCIB Demo application.\n\0";
	scib_msg(msg);

	msg = "\r\nYou will enter a character, and the DSP will echo it back! \n\0";
	scib_msg(msg);//发送
	ScibRegs.SCIFFRX.bit.RXFFOVRCLR=1;//清除标志位

	 Uint16 i;
	    	for(i = 0; i<5; i++)
	        {
	          sdata[i] = 0;
	        }

	//使能中断
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
	PieCtrlRegs.PIEIER9.bit.INTx3=1;     // PIE Group 9, int1
	PieCtrlRegs.PIEIER9.bit.INTx4=0;     // PIE Group 9, INT2
	IER |= M_INT9;  // Enable CPU INT

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER6.bit.INTx5=0;     // Enable PIE Group 6, INT 5
    IER |= M_INT6;                            // Enable CPU INT6

	EINT;   // Enable Global interrupt INTM
	ERTM;	// Enable Global realtime interrupt DBGM

	while(1)
	{

	}
}   // end of main



__interrupt void scibRxFifoIsr(void)
{
	Uint16 i;
	//while(ScibRegs.SCIFFRX.bit.RXFFST !=1)// wait for XRDY =1 for empty state

		for(i=0;i<8;i++)
		{
			rdataB[i]=ScibRegs.SCIRXBUF.all;
		}
		if(rdataB[0]==0xFF)
		{
			 PieCtrlRegs.PIEIER6.bit.INTx5=1;     // Enable PIE Group 6, INT 5
		}else if(rdataB[0]==0xFE)
		{
			PieCtrlRegs.PIEIER6.bit.INTx5=0;     // Enable PIE Group 6, INT 5
		}
		msg = "\n\nDSP Receive: ";
		scib_msg(msg);
		scib_msg(rdataB);

	ScibRegs.SCIFFTX.bit.TXFFINTCLR=1;
	//SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;  // Clear Overflow flag
	ScibRegs.SCIFFRX.bit.RXFFINTCLR=1;  // Clear Interrupt flag

	PieCtrlRegs.PIEACK.all|=0x100;       // Issue PIE ack

}

interrupt void  ISRMcbspSend(void)
{
	EINT;
	int temp1;

	temp1=McbspaRegs.DRR1.all;
	//temp2=McbspaRegs.DRR2.all;

	McbspaRegs.DXR1.all = temp1;        //放音
	//McbspaRegs.DXR2.all = temp2;
	unsigned char data[2] = {(temp1 >> 8), temp1};   //输入数据
	unsigned char crc = crc8(data, sizeof(data));  //计算CRC校验值
	sdata[0]=BAOTOU;
	sdata[1]=data[0];
	sdata[2]=data[1];
	sdata[3]=crc;
	sdata[4]=BAOWEI;
	Uint16 cnt;
	for(cnt=0;cnt<5;cnt++)
	{
		scib_xmit(sdata[cnt]);
	}
	PieCtrlRegs.PIEACK.all = 0x0020;
//	PieCtrlRegs.PIEIFR6.bit.INTx5 = 0;
//	ERTM;
}
// Test 1,SCIA  DLB, 8-bit word, baud rate 0x000F, default, 1 STOP bit, no parity
void scib_echoback_init()
{
    // Note: Clocks were turned on to the Scib 外部
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
	      ScibRegs.SCIHBAUD    =0x0000;  // 115200 baud @LSPCLK = 37.5MHz.（37500000/（8*115200）-1）
	      ScibRegs.SCILBAUD    =0x0028;
	#endif
	#if (CPU_FRQ_100MHZ)
      ScibRegs.SCIHBAUD    =0x0000;  // 115200 baud @LSPCLK = 20MHz.
      ScibRegs.SCILBAUD    =0x0015;
	#endif
	ScibRegs.SCICTL1.all =0x0023;  // Relinquish SCI from Reset
}

// Transmit a character from the SCI
//发送字节函数
void scib_xmit(int a)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScibRegs.SCITXBUF=a;

}

//发送数据包

//发字符串
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
    ScibRegs.SCIFFTX.bit.SCIRST=1;// FIFO复位
    ScibRegs.SCIFFTX.bit.SCIFFENA=1;// 使能FIFO
    // ScibRegs.SCIFFTX.bit.TXFFINTCLR = 1;  // 清除发送FIFO中断标志位
    ScibRegs.SCIFFTX.bit.TXFFIENA = 0;      // 使能发送FIFO中断
    ScibRegs.SCIFFTX.bit.TXFFIL = 0;        // 发送FIFO中断级别

    // SciaRegs.SCIFFRX.all=0x0028;
    ScibRegs.SCIFFRX.bit.RXFFIENA = 1;      // 使能接收FIFO中断
    ScibRegs.SCIFFRX.bit.RXFFIL = 15;        // 接收FIFO中断级别

    ScibRegs.SCIFFCT.all=0x00;              // FIFO传送延时为0

    ScibRegs.SCICTL1.bit.SWRESET=1;         // 重启SCIB

    ScibRegs.SCIFFTX.bit.TXFIFOXRESET=1;    // 重新使能发送FIFO的操作
    ScibRegs.SCIFFRX.bit.RXFIFORESET=1;     // 重新使能接收FIFO的操作

    //ScibRegs.SCIFFRX.all=0x204f;
    //ScibRegs.SCIFFCT.all=0x0;

}

void I2CA_Init(void)
{
   // Initialize I2C
   I2caRegs.I2CSAR = 0x001A;		// Slave address（从设备） - EEPROM control code

   #if (CPU_FRQ_150MHZ)             // Default - For 150MHz SYSCLKOUT
        I2caRegs.I2CPSC.all = 14;   // Prescaler - need 7-12 Mhz on module clk (150/15 = 10MHz)
   #endif
   #if (CPU_FRQ_100MHZ)             // For 100 MHz SYSCLKOUT
     I2caRegs.I2CPSC.all = 9;	    // Prescaler - need 7-12 Mhz on module clk (100/10 = 10MHz)
   #endif

   I2caRegs.I2CCLKL = 100;			// NOTE: must be non zero
   I2caRegs.I2CCLKH = 100;			// NOTE: must be non zero
   I2caRegs.I2CIER.all = 0x24;		// Enable SCD & ARDY interrupts

//   I2caRegs.I2CMDR.all = 0x0020;	// Take I2C out of reset
   I2caRegs.I2CMDR.all = 0x0420;	// Take I2C out of reset		//zq
   									// Stop I2C when suspended

   I2caRegs.I2CFFTX.all = 0x6000;	// Enable FIFO mode and TXFIFO
   I2caRegs.I2CFFRX.all = 0x2040;	// Enable RXFIFO, clear RXFFINT,

   return;
}

Uint16 AIC23Write(int Address,int Data)
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
	int i,j,k=0;
	for(i=0;i<time;i++)
		for(j=0;j<1024;j++)
			k++;
}

void delay(Uint32 k)
{
   while(k--);
}

//计算CRC值
unsigned char crc8(unsigned char *buf, int len)
{
    unsigned char crc = 0;
    int i = 0;
    int j = 0;
    for (i = 0; i < len; i++)           //循环处理每一个字节数据
    {
        crc ^= buf[i];                     //异或数据和CRC
        for (j = 0; j < 8; ++j)        //CRC按位移动处理
        {
            if (crc & 0x80)                //检查最高位是否为1
            {
                crc = ((crc << 1) ^ POLY); //若为1，则左移一位后异或生成多项式
            }
            else
            {
                crc <<= 1;                 //否则左移一位
            }
        }
    }
    return crc;
}
//串口发送中断
//__interrupt void scibTxFifoIsr(void)
//{
////	msg = "\n\nDSP Send: ";
////	scib_msg(msg);
////	Uint16 i;
////	for(i=0; i< 8; i++)
////	{
////	   ScibRegs.SCITXBUF=sdataB[i];     // Send data
////	}
//
//	//ScibRegs.SCIFFTX.bit.TXFFINTCLR=1;  // Clear SCI Interrupt flag
//	PieCtrlRegs.PIEACK.all|=0x100;      // Issue PIE ACK
//	EINT;
//}



//===========================================================================
// No more.
//===========================================================================
