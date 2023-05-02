/*
 * 功能描述：
 *     本程序可以完成数据的采集及带有CRC校验的数据传输
 *
 */

// 包含所需的头文件
#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "DSP28x_Project.h"

// 使用前，声明本文件中的相关函数；
void I2CA_Init(void);							 // I2C-A总线初始化函数声明
Uint16 AIC23Write(int Address, int Data);		 // AIC23音频编解码器寄存器写入函数声明
void Delay(int time);							 // 延时函数声明
interrupt void ISRMcbspSend();					 // McBSP中断服务函数声明
void AIC23Init(void);							 // AI23音频解码器声明
void scib_echoback_init(void);					 // 串口B初始化-回声模式
void scib_fifo_init(void);						 // 串口B FIFO初始化
void scib_xmit(int a);							 // 串口B发送数据
void scib_msg(char *msg);						 // 串口B发送字符串
__interrupt void scibRxFifoIsr(void);			 // 串口B接收中断服务函数
unsigned char crc8(unsigned char *buf, int len); // CRC校验

// 定义全局变量
char sdata[5];	   // 用于SCI-B发送数据
char rdataB[8];	   // 用于SCI-B接收数据
#define POLY 0x07  // CRC计算采用的多项式 POLY=0x07
int BAOTOU = 0xAB; // 帧头字节
int BAOWEI = 0xEF; // 帧尾字节
#define START 0xFF // 开始标志
#define STOP 0xFE  // 结束标志

// 主函数
void main(void)
{
	InitSysCtrl(); // 配置系统时钟

	InitMcbspaGpio(); // 初始化McBSP模块的GPIO口
	InitI2CGpio();	  // 初始化 I2C 模块的GPIO口
	InitScibGpio();	  // 初始化 SCI 模块的GPIO口

	// 禁用CPU中断
	DINT;

	InitPieCtrl(); //  初始化PIE控制器

	// 禁用所有CPU中断并清除所有中断标志位
	IER = 0x0000;
	IFR = 0x0000;

	InitPieVectTable(); // 初始化 PIE 向量表

	SCIStdioInit(); // 配置SCI输出

	I2CA_Init(); // 初始化I2C模块

	AIC23Init(); // 初始化AI23音频解码器

	InitMcbspa(); // 初始化McBSP模块

	EALLOW;									 // 写入受EALLOW保护的寄存器
	PieVectTable.MRINTA = &ISRMcbspSend;	 // 配置MRINTA中断向量
	PieVectTable.SCIRXINTB = &scibRxFifoIsr; // 配置SCIRXINTB中断向量

	EDIS; // 禁止写入受EALLOW保护的寄存器
	// 串行通信接口（SCI）缓存区初始化
	scib_fifo_init();	  // 初始化SCI FIFO
	scib_echoback_init(); // 回送的SCI的初始化

	ScibRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // 清除接收消息中的溢出标志

	Uint16 i;
	for (i = 0; i < 5; i++) //  初始化sdata数组
	{
		sdata[i] = 0;
	}

	// PIE中断控制器配置
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // 启用PIE块
	PieCtrlRegs.PIEIER9.bit.INTx3 = 1; // PIE Group 9, int1
	PieCtrlRegs.PIEIER9.bit.INTx4 = 0; // PIE Group 9, INT2
	IER |= M_INT9;					   // Enable CPU INT

	PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // 启用PIE块
	PieCtrlRegs.PIEIER6.bit.INTx5 = 0; // 启用PIE第6组，INT5
	IER |= M_INT6;					   // 启用CPU INT6

	EINT; // 启用全局中断INTM

	EINT; // 启用全局中断
	ERTM; // 启用全局实时中断

	while (1) // 进入循环
	{
	}
} // 主函数结束

__interrupt void scibRxFifoIsr(void)
{
	// while(ScibRegs.SCIFFRX.bit.RXFFST !=1)// wait for XRDY =1 for empty state
	Uint16 i;
	Uint16 flag_start = 0;
	Uint16 flag_stop = 0;
	for (i = 0; i < 8; i++)
	{
		rdataB[i] = ScibRegs.SCIRXBUF.all; // 读取串口接收缓冲寄存器中的值
		if (rdataB[i] == START)
		{
			flag_start++;
		}
		else if (rdataB[i] == STOP)
		{
			flag_stop++;
		}
	}
	if (flag_start > 5) // 判断接收到大于5次开始命令
	{
		PieCtrlRegs.PIEIER6.bit.INTx5 = 1; // 使能McBSP中断
	}
	else if (flag_stop > 5) // 判断接收到大于5次结束命令
	{
		PieCtrlRegs.PIEIER6.bit.INTx5 = 0; // 失能McBSP中断
	}
	else
		;
	ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1; // 清除中断标志

	PieCtrlRegs.PIEACK.all |= 0x100; // 清除外部中断信号标志，并确认该中断已经被处理
}

// 定义McBSP发送时钟中断服务函数
interrupt void ISRMcbspSend(void) // 发送中断程序
{
	EINT;	   // 启用全局中断
	int temp1; // 定义临时变量temp1

	temp1 = McbspaRegs.DRR1.all;								   // 将从McBSP设备接收到的数据读取到temp1中
	McbspaRegs.DXR1.all = temp1;								   // 将temp1从缓冲区写入到McBSP设备中进行发送
	scib_xmit(((temp1 & 0x0F00) >> 4) | ((temp1 & 0xF000) >> 12)); // 将16位数据分成两个8位的字节，两个八字节数据分别进行左循环移位加密传输
	scib_xmit(((temp1 & 0x000F) << 4) | ((temp1 & 0x00F0) >> 4));
	PieCtrlRegs.PIEACK.all = 0x0020; // 清除响应的中断标志位，以便下一次继续处理产生此中断的事件
}

// Test 1,SCIA  DLB,8位数据位, 波特率0x000F, 默认设置，1个停止位，无奇偶校验
void scib_echoback_init()
{
	// 注：时钟在InitSysCtrl()函数中已经开启到Scib外设上。

	ScibRegs.SCICCR.all = 0x0007; // 1个停止位，不使用回环模式
								  // 不使用奇偶校验，8个字符位，
								  // 异步模式，空闲线协议
	// 使能TX、RX、内部SCICLK，禁用RX ERR、SLEEP、TXWAKE
	ScibRegs.SCICTL1.all = 0x0003;
	ScibRegs.SCICTL2.all = 0x0003;
	ScibRegs.SCICTL2.bit.TXINTENA = 1;
	ScibRegs.SCICTL2.bit.RXBKINTENA = 1;

// 设置波特率
#if (CPU_FRQ_150MHZ)
	ScibRegs.SCIHBAUD = 0x0000; // 115200 baud @LSPCLK = 37.5MHz.（37500000/（8*115200）-1）
	ScibRegs.SCILBAUD = 0x0028;
#endif
#if (CPU_FRQ_100MHZ)
	ScibRegs.SCIHBAUD = 0x0000; // 115200 baud @LSPCLK = 20MHz.
	ScibRegs.SCILBAUD = 0x0015;
#endif
	ScibRegs.SCICTL1.all = 0x0023; // 取消SCI复位状态
}

// 初始化AI23音频解码器
void AIC23Init(void)
{
	AIC23Write(0x00, 0x00); // 左声道麦克风输入音量控制:音量设置为-35dB
	Delay(100);
	AIC23Write(0x02, 0x00); // 右声道麦克风输入音量控制:音量设置为-35dB
	Delay(100);
	AIC23Write(0x04, 0x60); // 左声道耳机输出音量控制:音量设置为-25dB
	Delay(100);
	AIC23Write(0x06, 0x60); // 右声道耳机输出音量控制:音量设置为-25dB
	Delay(100);
	AIC23Write(0x08, 0xF5); // 模拟音频路径控制:侧音衰减开，设置为-15dB;mic增强为20dB
	Delay(100);
	AIC23Write(0x0A, 0x00); // 数字音频路径控制 全关
	Delay(100);
	AIC23Write(0x0C, 0x00); // 断电控制 全开
	Delay(100);
	AIC23Write(0x0E, 0x43); // 数字音频接口格式：主模式；DSP格式；输入为16位
	Delay(100);
	AIC23Write(0x10, 0x23); // 采样率控制：采样率为44.1kHz
	Delay(100);
	AIC23Write(0x12, 0x01); // 数字接口激活:激活
	Delay(100);
}

// 发送字节函数
void scib_xmit(int a)
{
	// 检查发送FIFO是否为空，如果不是则等待
	while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
	{
	}

	// 将输入的字节发送到SCITXBUF中
	ScibRegs.SCITXBUF = a;
}

// 输入一个字符串，通过串口scib发送
void scib_msg(char *msg)
{
	int i;
	i = 0;
	// 使用while循环遍历整个字符串，直到遇到字符串结束标志'\0'
	while (msg[i] != '\0')
	{
		scib_xmit(msg[i]); // 逐个字符发送

		i++;
	}
}

// 初始化 SCI FIFO
void scib_fifo_init()
{
	ScibRegs.SCIFFTX.bit.SCIRST = 1;   // FIFO复位
	ScibRegs.SCIFFTX.bit.SCIFFENA = 1; // 使能FIFO

	ScibRegs.SCIFFRX.bit.RXFFIENA = 1; // 使能接收FIFO中断
	ScibRegs.SCIFFRX.bit.RXFFIL = 15;  // 接收FIFO中断级别

	ScibRegs.SCIFFCT.all = 0x00; // FIFO传送延时为0

	ScibRegs.SCICTL1.bit.SWRESET = 1; // 重启SCIB

	ScibRegs.SCIFFTX.bit.TXFIFOXRESET = 1; // 重新使能发送FIFO的操作
	ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;  // 重新使能接收FIFO的操作
}

// 初始化I2C
void I2CA_Init(void)
{
	// Initialize I2C
	I2caRegs.I2CSAR = 0x001A; // 从设备地址 - EEPROM控制码

#if (CPU_FRQ_150MHZ)		  // 默认为150MHz SYSCLKOUT
	I2caRegs.I2CPSC.all = 14; // 预分频器 - 模块时钟需要在7-12MHz之间 (150/15 = 10MHz)
#endif
#if (CPU_FRQ_100MHZ)		 // 设为100 MHz SYSCLKOUT
	I2caRegs.I2CPSC.all = 9; // 预分频器 - 模块时钟需要在7-12MHz之间 (100/10 = 10MHz)
#endif

	I2caRegs.I2CCLKL = 100;		// NOTE: must be non zero
	I2caRegs.I2CCLKH = 100;		// NOTE: must be non zero
	I2caRegs.I2CIER.all = 0x24; // 使能SCD & ARDY中断

	//   I2caRegs.I2CMDR.all = 0x0020;	// Take I2C out of reset
	I2caRegs.I2CMDR.all = 0x0420; // Take I2C out of reset
								  // Stop I2C when suspended

	I2caRegs.I2CFFTX.all = 0x6000; // 使能FIFO模式和TXFIFO
	I2caRegs.I2CFFRX.all = 0x2040; // 使能RXFIFO，清除RXFFINT

	return;
}

// AIC23音频编解码器寄存器写入的函数，通过设置I2C传输参数，实现了对寄存器的读写操作。
Uint16 AIC23Write(int Address, int Data)
{
	// 判断当前I2C总线是否处于空闲状态，如果不是，则返回I2C_BUS_BUSY_ERROR
	if (I2caRegs.I2CMDR.bit.STP == 1)
	{
		return I2C_STP_NOT_READY_ERROR;
	}

	// 设置从机地址
	I2caRegs.I2CSAR = 0x1A;

	// 如果当前I2C总线正在发送或接收数据，则返回I2C_STP_NOT_READY_ERROR
	if (I2caRegs.I2CSTR.bit.BB == 1)
	{
		return I2C_BUS_BUSY_ERROR;
	}

	// 如果当前I2C总线正在发送或接收数据，则返回I2C_STP_NOT_READY_ERROR
	I2caRegs.I2CCNT = 2; // 设置需要发送的字节数，此处为2个字节
	I2caRegs.I2CDXR = Address;
	I2caRegs.I2CDXR = Data;
	// 发送启动信号并设置工作模式为主机发送模式
	I2caRegs.I2CMDR.all = 0x6E20;

	// 返回I2C传输成功的标志
	return I2C_SUCCESS;
}

void Delay(int time)
{
	int i, j, k = 0;			   // 定义循环变量i、j以及累加器k，初始化k为0
	for (i = 0; i < time; i++)	   // 外层循环，循环次数为time
		for (j = 0; j < 1024; j++) // 内层循环，循环次数为1024
			k++;				   // 累加器k自增1
}

// 计算CRC值
unsigned char crc8(unsigned char *buf, int len)
{
	unsigned char crc = 0;
	int i = 0;
	int j = 0;
	for (i = 0; i < len; i++) // 循环处理每一个字节数据
	{
		crc ^= buf[i];			// 异或数据和CRC
		for (j = 0; j < 8; ++j) // CRC按位移动处理
		{
			if (crc & 0x80) // 检查最高位是否为1
			{
				crc = ((crc << 1) ^ POLY); // 若为1，则左移一位后异或生成多项式
			}
			else
			{
				crc <<= 1; // 否则左移一位
			}
		}
	}
	return crc; // 返回计算的crc值
}

//===========================================================================
// No more.
//===========================================================================
