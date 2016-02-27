#include <reg52.h>
#include <intrins.h> 
#include "type.h"
#include "nrf24l01.h"
unsigned char TxBuf[20]={0};  //发送数据的缓冲区
unsigned char RxBuf[20]={0};	//接受缓冲区
unsigned char flag;
unsigned char bdata sta;
uchar idata PM_Dat=0;
sbit	RX_DR	=sta^6;
sbit	TX_DS	=sta^5;
sbit	MAX_RT	=sta^4;

sbit KEY1=P1^3;
sbit KEY2=P1^4;
sbit KEY3=P1^5;
sbit BEEP=P2^7;	
uchar Set_flag=0;
uchar beep_flag=0;
uchar H_dat=40,L_dat=20;

void delay1ms(unsigned int ms)//延时1毫秒（不够精确的）
{
	unsigned int i,j;
   for(i=0;i<ms;i++)
    for(j=0;j<100;j++);
}

#include"DS18B20.H"
#include "1602.C"

void delayus(uint dat)
{
	for(;dat>0;dat--)
	{
		_nop_();	
	}
}

void keyscan()
{
	uchar i=0;
	if(!KEY1)
	{
		delay1ms(5);
		if(!KEY1)
		{
			Set_flag++;	
			if(Set_flag>2)
			{
				Set_flag=0;
			}
			while(!KEY1);
		}	
	}
	switch(Set_flag)
	{
		case 1:
			if(!KEY2)
			{
				delay1ms(5);
				if(!KEY2)
				{
					if(H_dat<99)
					H_dat++;
					while(!KEY2);
				}	
			}
			else if(!KEY3)
			{
				delay1ms(5);
				if(!KEY3)
				{
					if(H_dat>0)
					H_dat--;
					while(!KEY3);
				}	
			}
		break;
		case 2:
			if(!KEY2)
			{
				delay1ms(5);
				if(!KEY2)
				{
					if(L_dat<99)
					L_dat++;
					while(!KEY2);
				}	
			}
			else if(!KEY3)
			{
				delay1ms(5);
				if(!KEY3)
				{
					if(L_dat>0)
					L_dat--;
					while(!KEY3);
				}	
			}
		break;
		default:
		break;
	}
}
void Control()
{
	static uchar i=0;
	i++;
	if(i>10)
	i=0;
	if(Tem_dispbuf[4]<L_dat)
	{
		beep_flag|=0x04;
	}
	else
	{	beep_flag&=0xfB;
		
	}
	if(Tem_dispbuf[4]>=H_dat)
	{
		beep_flag|=0x08;
	}
	else
	{
		beep_flag&=0xf7;	
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
	TxBuf[0]=0x00; 
	Read_18B20_Temperature();
	Read_18B20_Temperature();
	delay1ms(200);
	Read_18B20_Temperature();
	Read_18B20_Temperature();
	delay1ms(200);
	Read_18B20_Temperature();
	Read_18B20_Temperature();
	delay1ms(200);
	nRF24L01_Config();	  //初始化NRF24L01
	SPI_RW_Reg(FLUSH_TX,0);	
	EX1=1;
	IT1=1;
	EA=1;
	init_1602();
	while(1)
	{
		Read_18B20_Temperature();
		keyscan();
		Control();
		display();
		TxBuf[0]=0xAA;
		TxBuf[2]=Tem_dispbuf[4];
		TxBuf[3]=Tem_dispbuf[0]; 
		TxBuf[4]=beep_flag;
		nRF24L01_TxPacket(TxBuf); //发送数据
		delay1ms(10);
	}

}

void ISR_int1(void) interrupt 2	  //中断服务程序
{
	sta=SPI_Read(STATUS);	// 读状态寄存器
	if(RX_DR)				// 接受状态位
	{
		SPI_Read_Buf(RD_RX_PLOAD,RxBuf,TX_PLOAD_WIDTH);// 从RX_FIFO把数据读出来
		//flag=1;
	}	  
	if(TX_DS)			  //发送完成标志位
	{
		flag=1;
	}
	if(MAX_RT)	   //超时标志
	{
		SPI_RW_Reg(FLUSH_TX,0);
	}
	SPI_RW_Reg(WRITE_REG+STATUS,sta);// clear RX_DR or TX_DS or MAX_RT interrupt flag
		
}

