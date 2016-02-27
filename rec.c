#include "reg52.h"
#include "type.h"
#include "nrf24l01.h"
unsigned char flag;
unsigned char RxBuf[20]={0};
//unsigned char Temp_dat=0,Hum_dat=0,PM_Dat=0;

uchar 	bdata sta;
sbit	RX_DR	=sta^6;
sbit	TX_DS	=sta^5;
sbit	MAX_RT	=sta^4;

sbit BEEP=P2^7;
//uchar PH_dat=0;
uchar temp_dat=0;
uchar temp_dot=0;
uchar beep_flag=0;

void delay1ms(unsigned int ms)//延时1毫秒（不够精确的）
{
	unsigned int i,j;
   	for(i=0;i<ms;i++)
    	for(j=0;j<100;j++);
}

#include "1602.C"

void UartPutString(const uchar *pString) 
{
	while(*pString != '\0')
	{
		SBUF = *(pString++);
		while(!TI);
		TI = 0;	
	}
}


void UartPutByte(char ucData) 
{
	SBUF = ucData;
	while(!TI);
	TI = 0;	
	delay1ms(1);
}

void Control()
{
	static uchar i=0;
	i++;
	if(i>10)
	{
		i=0;
		UartPutByte('T');
		UartPutByte('e');
		UartPutByte('m');
		UartPutByte('p');
		UartPutByte(':');
		UartPutByte(temp_dat/10+'0');	
		UartPutByte(temp_dat%10+'0');
		UartPutByte('.');
		UartPutByte(temp_dot+'0');
		UartPutByte('\n');
	}
	
 	if(beep_flag>0)
	{
		if(i<5)
		BEEP=0;
		else
		BEEP=1;	
	}
	else
	BEEP=1;

}

void main()
{
	nRF24L01_Config();
	SetRX_Mode();
	if(MAX_RT)
	{
		SPI_RW_Reg(FLUSH_TX,0);
	}
	SPI_RW_Reg(WRITE_REG+STATUS,0xFF);// clear RX_DR or TX_DS or MAX_RT interrupt flag
    SPI_RW_Reg(FLUSH_RX,0);
	SCON =0x40;
	TMOD = 0x20;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;
	EX1=1;
	IT1=0;
	EA=1;
	init_1602();
	while(1)
	{
		Control(); 
		display();
		if(RxBuf[0]==0xAA)
		{
			RxBuf[0]=0;
			//PH_dat=RxBuf[1];
			temp_dat=RxBuf[2];
			temp_dot=RxBuf[3];
			beep_flag=RxBuf[4];
		}
		delay1ms(10);
	}
}

void ISR_int1(void) interrupt 2	   //接受中断用于接受发送过来的数据
{
	sta=SPI_Read(STATUS);	// read register STATUS's value
	if(RX_DR)				// if receive data ready (RX_DR) interrupt
	{
		SPI_Read_Buf(RD_RX_PLOAD,RxBuf,TX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer
		flag=1;
	}
	if(MAX_RT)
	{
		SPI_RW_Reg(FLUSH_RX,0);	
		
	}
	SPI_RW_Reg(WRITE_REG+STATUS,sta);// clear RX_DR or TX_DS or MAX_RT interrupt flag
}