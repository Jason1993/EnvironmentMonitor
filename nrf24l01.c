#include <reg52.h>
#include "nrf24l01.h"
#include "type.h"


uchar const TX_ADDRESS[TX_ADR_WIDTH]  = {0x34,0x43,0x10,0x10,0x01}; // 静态发送地址

//uchar TxBuf[TX_PLOAD_WIDTH];
//uchar RxBuf[TX_PLOAD_WIDTH];
//uchar flag;
unsigned int temp;

sbit 	MISO	=P3^7;	   
sbit 	MOSI	=P3^4;
sbit	SCK	    =P3^5;
sbit	CE	    =P3^0;
sbit	CSN		=P3^1;
sbit	IRQ		=P3^3;	   //采用中断写时必须接到P32或P33上

/****************************************************/
void delay130us()
{
	uchar i,j;
	for(i=0;i<255;i++)
    	for (j=0;j<2;j++);
}
/***************************************************/
void delay10us()
{
uchar i;
i++;
i++;
i++;
i++;
}

/**************************************************/
/**************************************************/
/*void init_int0(void)
{
	EA=1;
	EX0=1;						// Enable int0 interrupt.
} */
/**************************************************/

//  写一个字节到 24L01，同时读出一个字节 
uchar SPI_RW(uchar byte)   
{   
uchar bit_ctr;   
for(bit_ctr=0;bit_ctr<8;bit_ctr++)    // output 8-bit  
{   
MOSI = (byte & 0x80);  			     // output 'byte', MSB to MOSI  
byte = (byte << 1);  				 // shift next bit into MSB.. 
SCK = 1;  							  // Set SCK high.. 
byte |= MISO;  						 // capture current MISO bit  
SCK = 0;  							 // ..then set SCK low again   
}  
return(byte);  						 // return read byte 
} 

//   向寄存器 reg写一个字节，同时返回状态字节 
uchar SPI_RW_Reg(BYTE reg, BYTE value) 
{ 
 uchar status; 
  
 CSN = 0;                   // CSN low, init SPI transa
 status = SPI_RW(reg);      // select register 
 SPI_RW(value);             // ..and write value to it.. 
 CSN = 1;                   // CSN high again 
  return(status);            // return nRF24L01 status byte 
} 
//  读一个字节值从寄存器中 
BYTE SPI_Read(BYTE reg)
{
	BYTE reg_val;

  	CSN = 0;                // CSN low, initialize SPI communication...
  	SPI_RW(reg);            // Select register to read from..
  	reg_val = SPI_RW(0);    // ..then read registervalue
  	CSN = 1;                // CSN high, terminate SPI communication

  	return(reg_val);        // return register value
}

//  读出 bytes 字节的数据 
uchar SPI_Read_Buf(BYTE reg, BYTE *pBuf, BYTE bytes) 
{ 
 uchar status,byte_ctr; 
  
 CSN = 0;                      // Set CSN low, init SPI tranaction 
 status = SPI_RW(reg);       // Select register to write to and read status byte 
  
 for(byte_ctr=0;byte_ctr<bytes;byte_ctr++) 
  pBuf[byte_ctr] = SPI_RW(0);    //  
 CSN = 1;                            
  
 return(status);                    // return nRF24L01 status byte 
}  

//  写入 bytes 字节的数据 
uchar SPI_Write_Buf(BYTE reg, BYTE *pBuf, BYTE bytes) 
{ 
 uchar status,byte_ctr; 
  
 CSN = 0;                    
 status = SPI_RW(reg);    
 for(byte_ctr=0; byte_ctr<bytes; byte_ctr++) // 
  SPI_RW(*pBuf++); 
 CSN = 1;                 // Set CSN high again 
 return(status);          //  
} 
 
//  接收函数，返回 1 表示有数据收到，否则没有数据接受到 
/*unsigned char nRF24L01_RxPacket(unsigned char* rx_buf) 
{ 
    unsigned char revale=0; 
    // set in RX mode 
    SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);   // Set PWR_UP bit, enable CRC(2 bytes) & Prim:RX. RX_DR enabled..    

    CE = 1; // Set CE pin high to enable RX device 
	delay130us(); 
    sta=SPI_Read(STATUS); // read register STATUS's value 

	 if(RX_DR)    // if receive data ready (RX_DR) interrupt 
	 { 
	     CE = 0;   // stand by mode 

	  SPI_Read_Buf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH); 	 // read receive payload from RX_FIFO buffer 
	
	  revale =1; 
	 } 
 SPI_RW_Reg(WRITE_REG+STATUS,sta); 			// clear RX_DR or TX_DS or MAX_RT interrupt flag 

 return revale; 
} 		*/
 
 //  发送函数 
void nRF24L01_TxPacket(unsigned char * tx_buf) 
{ 
 CE=0; 

 SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH);  // Writes data to TX payload 

 SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);     	 // Set PWR_UP bit, enable CRC(2 bytes) & Prim:TX. MAX_RT & TX_DS enabled.. 
 
 CE=1; 
 delay10us(); 
 CE=0; 
} 
//  配置函数 
void nRF24L01_Config(void) 
{ 
 //initial io 
 CE=0;   // chip enable 
 CSN=1;   // Spi disable  
 SCK=0;   // Spi clock line init high 
 CE=0; 
SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);        // Set PWR_UP bit, enable CRC(2 bytes) & Prim:RX. RX_DR enabled.. 

 SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);        // Enable Auto.Ack:Pipe0
 SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);    // Enable Pipe0

 SPI_RW_Reg(WRITE_REG + SETUP_AW, 0x03);     // Setup address width=5 bytes 
 SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x1a);    // 500us + 86us, 10 retrans... 

 SPI_RW_Reg(WRITE_REG + RF_CH, 0);         	   // Select RF channel 0
 SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);       // TX_PWR:0dBm, Datarate:1Mbps, LNA:HCURR 

SPI_RW_Reg(WRITE_REG+RX_PW_P0, RX_PLOAD_WIDTH);   //Number of bytes in RX payload in data pipe 0 
SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     // Writes TX_Address to nRF24L01
SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // RX_Addr0 same as TX_Adr for Auto.Ack
CE=1;   
 } 

void SetRX_Mode(void) 
{ 
	CE=0; 
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);    // IRQ收发完成中断响应，16位CRC ，主接收 
	CE = 1;  
	delay130us(); 
} 