/***************************************************
	 功能：温度采集，分别设置
	 	   设置两路的上下限温度值并储存在EEPROM中，
		   掉电不丢失。

***************************************************/
#define uchar unsigned char
#define uint unsigned int
sbit DS18B20_DQ=P2^0;
uchar Tem_dispbuf[5]={0,0,0,0,0};// 显示数据暂存
uchar  data DS18B20_Temp_data[4]={0x00,0x00,0x00,0x00};	  //储存温度值得数组
uchar code  DS18B20_TEM_Deccode[16]={0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x04,			//温度小数位查表数组
                      				 0x05,0x06,0x06,0x07,0x08,0x08,0x09,0x09};
bit DS18B20_Presence;			//18b20复位成功标示位  =0 成功   =1 失败
bit Symbol_flag=0;				//
void Delay_6us(uchar xus)	//6*xus+11us的延时时间(经仿真得到的)
{
	  while(xus--);
}

void DS18B20_RESET()	  //复位
{
 	DS18B20_DQ=1;
	Delay_6us(2);

	DS18B20_DQ=0;
	Delay_6us(80);		        //至少480us的低电平信号

	DS18B20_DQ=1;		        //拉高等待接收18b20的存在脉冲信号
	Delay_6us(10);

	DS18B20_Presence=DS18B20_DQ;

	Delay_6us(40);
	DS18B20_DQ=1;		        //拉高使总线在空闲状态

}

void Write_DS18B20_OneChar(uchar dat)	  //写一个字节
{
	uchar i=0;
	for(i=8;i>0;i--)
	{
		DS18B20_DQ=0;
		DS18B20_DQ=dat&0x01;
		Delay_6us(5);
		DS18B20_DQ=1;
		dat>>=1;
	}
}

uchar Read_DS18B20_OneChar()		//读一个字节
{
	uchar dat=0;
	uchar i=0;
	for(i=8;i>0;i--)
	{
		DS18B20_DQ=0;
		dat>>=1;
		DS18B20_DQ=1;

		if(DS18B20_DQ)
		dat|=0x80;
		Delay_6us(4);
	}
	return dat;
}

void Read_18B20_Temperature()
{
	static unsigned char START_FLAG=0;		//读取DS18B20内部EEPROM标志位
	DS18B20_RESET();						//复位18B20
	if(!DS18B20_Presence)					//复位成功
	{
		Write_DS18B20_OneChar(0XCC);		//跳过读序列号
		Write_DS18B20_OneChar(0X44);		//启动温度转换
		Delay_6us(82);						//等待温度转换时间500us左右

		DS18B20_RESET();					//复位18B20
		Write_DS18B20_OneChar(0XCC);		//发送匹配ROM指令
		Write_DS18B20_OneChar(0XBE);
		DS18B20_Temp_data[0]=Read_DS18B20_OneChar(); //Temperature LSB
		DS18B20_Temp_data[1]=Read_DS18B20_OneChar(); //Temperature MSB
		if(START_FLAG<3)
		{
			DS18B20_Temp_data[2]=Read_DS18B20_OneChar();  //温度上限
			DS18B20_Temp_data[3]=Read_DS18B20_OneChar();  //温度下限
			START_FLAG++;
		}
	}
		if(DS18B20_Temp_data[1]>0x80)
		Symbol_flag=1;
		else
		Symbol_flag=0;
		Tem_dispbuf[0]=DS18B20_TEM_Deccode[DS18B20_Temp_data[0]&0x0f];			//小数位
		Tem_dispbuf[4]=((DS18B20_Temp_data[1]&0x0f)<<4)|((DS18B20_Temp_data[0]&0xf0)>>4);	//取出温度值得整数位
		Tem_dispbuf[3]=Tem_dispbuf[4]/100;
		Tem_dispbuf[2]=Tem_dispbuf[4]%100/10;
		Tem_dispbuf[1]=Tem_dispbuf[4]%10; 	
	
}

//void Set_DS18B20(uchar TH,uchar TL)  //修改报警温度和温度转换精度
//{
////	uchar i;
//	DS18B20_RESET();					//复位18B20
//	Write_DS18B20_OneChar(0XCC);
//	Write_DS18B20_OneChar(0x4E); 	 //写入"写暂存器"命令,修改TH和TL和分辩率配置寄存器
//							    //先写TH,再写TL,最后写配置寄存器
//	Write_DS18B20_OneChar(TH);		//写入想设定的温度报警上限
//	Write_DS18B20_OneChar(TL);		//写入想设定的温度报警下限
//	Write_DS18B20_OneChar(0x7f);		//写配置寄存器,格式为0 R1 R0 1,1 1 1 1
//									//R1R0=00分辨率娄9位,R1R0=11分辨率为12位   
//									//保存设置
//	Delay_6us(10);					//等待数据稳定
//	DS18B20_RESET();
//	Write_DS18B20_OneChar(0xCC);    	// 跳过读序号列号的操作
//	Write_DS18B20_OneChar(0x48);		//回调设置
//
//	DS18B20_RESET();
//	Write_DS18B20_OneChar(0xCC);    	// 跳过读序号列号的操作
//	Write_DS18B20_OneChar(0xB8);
//	while(!DS18B20_DQ);
//}  