sbit EN=P1^2;
sbit RW=P1^1;
sbit RS=P1^0;
unsigned char code LCD1602_Table[]="0123456789:-";
	//LCD1602œ‘ æ±Í≥ﬂ	        0123456789abcdef
unsigned char code dis_tab1[]={"  temp: --.- C  "};
unsigned char code dis_tab2[]={" H:-- C  L:-- C "};
void write_date(uchar date)
{
	EN=0;
	RS=1;
	P0=date;
	EN=1;
	delay1ms(3);
	EN=0;
}
void write_com(uchar date)
{
	EN=0;
	RS=0;
	P0=date;
	EN=1;
	delay1ms(3);
	EN=0;
}
void init_1602()
{
	unsigned char i;
	RW=0;
	write_com(0x38);
	write_com(0x0c);
	write_com(0x06);
	write_com(0x01);
	write_com(0x80);
	for(i=0;i<16;i++)
	{write_date(dis_tab1[i]);}
	write_com(0x80+0x40);
	for(i=0;i<16;i++)
	{write_date(dis_tab2[i]);}
}
void display()
{
	static uchar i=0;
	i++;
	if(i>10)
	i=0;

	write_com(0x80+8);	
	write_date(LCD1602_Table[Tem_dispbuf[4]%100/10]);
	write_date(LCD1602_Table[Tem_dispbuf[4]%10]);  
	write_date('.');
	write_date(LCD1602_Table[Tem_dispbuf[0]]); 
	write_date(0xdf); 

	write_com(0x80+3+0x40);
	if((Set_flag==1)&&(i<5))
	{
		write_date(' ');
		write_date(' ');	
	}	
	else
	{
		write_date(LCD1602_Table[H_dat/10]);
		write_date(LCD1602_Table[H_dat%10]);	
	}					   
	write_date(0xdf); 
	write_com(0x80+11+0x40);
	if((Set_flag==2)&&(i<5))
	{
		write_date(' ');
		write_date(' ');	
	}	
	else
	{
		write_date(LCD1602_Table[L_dat/10]);
		write_date(LCD1602_Table[L_dat%10]);
	}
	write_date(0xdf);
}

