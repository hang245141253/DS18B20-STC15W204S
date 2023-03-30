//**************************
//程序说明:stc15系列采集ds18b20温度数据，并从串口输出
//采用的是11.0592M晶振，波特率9600
//**************************
#include "STC15Fxxxx.H" 

//IO口声明
#define ds18b20_io P55   //P5.5口作为数据读取接口


//变量定义
u16 datas;//保存温度数据

//函数声明
void Delay3us();
void Delay6us();
void Delay24us();
void Delay30us();
void Delay300us();
void Delay600us();
void Delay50ms();
void Delay150ms();
void Delay1000ms();
void led50(u8 i);												//led闪烁50毫秒
void led150(u8 i);											//led闪烁150毫秒
void led1000(u8 i);											//led闪烁1秒

// DS18B20
bit  ds18b20_init();                    //初始化ds18b20
u8   ds18b20_read_one_char();           //从ds18b20读取一个字节
void ds18b20_write_one_char(u8 dat);    //向ds18b20写入一个字节
void ds18b20_ready_read_temp();         //准备读取数据
u16  ds18b20_read_temp_val();           //读取数据,返回温度值，返回实际值的100倍，且万位为符号位，例：12556为-25.56度，2556为25.56度

// Uart
void  serial_init();                    //串口初始化
void  serial_send_byte(u8 dat);      	  //串口发送一个字节的数据
void  serial_send_string(u8 *dat);			//串口发送字符串

//主函数
void main()
{
	P5M1 = 0;	P5M0 = 0x10;	//P54设置为推挽输出，其他为准双向口
	led50(10);							//开机提示
	P54=0;
	datas=0;
	serial_init();					//串口初始化
	
	while(1)
	{
		datas=ds18b20_read_temp_val();
		serial_send_string("temperature: ");
		if(datas/10000 == 1)                                            //如果万位为1，则表示温度为负
			serial_send_byte('-');                                  //输出负号
		serial_send_byte(datas%10000/1000+'0');     //十位
		serial_send_byte(datas%1000/100+'0');         //个位
		serial_send_byte('.');
		serial_send_byte(datas%100/10+'0');         //小数点后第一位
		serial_send_byte(datas%10+'0');                 //小数点后第二位
		serial_send_string("C\r\n");
		
		if(datas<3400)
		{
			P54=0;		//继电器为0断开
			led1000(1);
		}
		else if(datas>3500)
		{
			P54=1;		//继电器为1连通
			led150(6);
		}
		else//临界值处理(34°C~35°C)
		{
			led50(10);//led快闪表示温度正处于临界值
		}
	}

}
//ds18b20初始化
bit ds18b20_init()
{
    u8 init_success_tag=0;
    ds18b20_io=1;//拉高
    Delay6us();//6us
    ds18b20_io=0;//拉低
    Delay600us();//600us
    ds18b20_io=1;//拉高
    Delay30us();//30us
    init_success_tag=ds18b20_io;
    Delay600us();//600us
    return init_success_tag;
}
//读取一个字节
u8 ds18b20_read_one_char()
{
    u8 i=0;
    u8 dat=0;
    for(i=0;i<8;i++)
    {
        ds18b20_io=1;//拉高
        NOP1();
        ds18b20_io=0;//拉低
        dat>>=1;
        NOP1();
        ds18b20_io=1;//拉高
        Delay6us();//6us
        if(ds18b20_io==1)
            dat|=0x80;
        else
            dat|=0x00;
        Delay24us();//24us
    }
    return (dat);
}
//向传感器写入一个字节
void ds18b20_write_one_char(u8 dat)
{
    u8 i=0;
    for(i=0;i<8;i++)
    {
        ds18b20_io=1;//拉高
        NOP1();
        ds18b20_io=0;//拉低
        ds18b20_io=dat&0x01;
        Delay30us();//30us
        ds18b20_io=1;//拉高
        Delay3us();//3us
        dat>>=1;
    }
    Delay6us();
    Delay6us();//12us
}
//准备读取一个温度值
void ds18b20_ready_read_temp()
{
    ds18b20_init();                          //初始化
    ds18b20_write_one_char(0xcc);//忽略读序列号
    ds18b20_write_one_char(0x44);//启动温度转换
    Delay300us();                                   //300us,等待转换完毕
    ds18b20_init();                          //初始化
    ds18b20_write_one_char(0xcc);//忽略读序列号
    ds18b20_write_one_char(0xbe);//读取温度寄存器
}
//读取数据,返回温度值，返回实际值的100倍，且万位为符号位，例：12556
u16 ds18b20_read_temp_val()
{
    u16 temp_16_bit=0;
    u8  temp_L=0;
    u8  temp_H=0;
    ds18b20_ready_read_temp();
    temp_L=ds18b20_read_one_char();//读取温度低八位
    temp_H=ds18b20_read_one_char();//读取温度高八位
    if(temp_H>0x7f)
    {
        temp_L=~temp_L;                    //补码转换，取反加一
        temp_H=~temp_H+1;  
        temp_16_bit=100;
    }
    temp_16_bit=temp_16_bit+temp_H*16+temp_L/16;
    temp_16_bit=temp_16_bit*100;
    temp_16_bit=temp_16_bit+((temp_L&0x0f)*10/16)*10;
    temp_16_bit=temp_16_bit+(temp_L&0x0f)*100/16%10;
    return (u16)(temp_16_bit);
}
//延时函数
void Delay3us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	_nop_();
	i = 5;
	while (--i);
}

void Delay6us()		//@11.0592MHz
{
	unsigned char i;

	i = 14;
	while (--i);
}

void Delay24us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	_nop_();
	i = 63;
	while (--i);
}

void Delay30us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 80;
	while (--i);
}

void Delay300us()		//@11.0592MHz
{
	unsigned char i, j;

	_nop_();
	_nop_();
	i = 4;
	j = 54;
	do
	{
		while (--j);
	} while (--i);
}

void Delay600us()		//@11.0592MHz
{
	unsigned char i, j;

	i = 7;
	j = 113;
	do
	{
		while (--j);
	} while (--i);
}

void Delay50ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 3;
	j = 26;
	k = 223;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void Delay150ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 7;
	j = 78;
	k = 167;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void Delay1000ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 43;
	j = 6;
	k = 203;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


//led闪烁函数
void led50(u8 i)
{
	while(i != 0)
	{
		P33=!P33;
		Delay50ms();
		P33=!P33;
		Delay50ms();
		i--;
	}
}
void led150(u8 i)
{
	while(i != 0)
	{
		P33=!P33;
		Delay150ms();
		P33=!P33;
		Delay150ms();
		i--;
	}
}

void led1000(u8 i)
{
	while(i != 0)
	{
		P33=!P33;
		Delay1000ms();
		P33=!P33;
		Delay1000ms();
		i--;
	}
}

//串口初始化
void serial_init()
{
    SCON = 0x50;                //8位数据,可变波特率
    AUXR |= 0x01;                //串口1选择定时器2为波特率发生器
    AUXR &= 0xFB;                //定时器2时钟为Fosc/12,即12T
    T2L = 0xE8;                //设定定时初值  11.0592M
    T2H = 0xFF;                //设定定时初值
    AUXR |= 0x10;                //启动定时器2
    ES = 1;     //串行中断 允许控制位
    //REN = 1;    //串口1允许接收
    EA = 1;
}
//串口发送一个字节数据
void serial_send_byte(u8 dat)
{
    SBUF=dat;
    while(!TI) ;			//等到TI标志变为1
    TI=0;
}

//串口发送一串字符串
void serial_send_string(u8 *dat)
{
    while(*dat)
    {
        serial_send_byte(*(dat++));
    }
}

//********************* 串口1中断函数************************
void UART1_int(void) interrupt UART1_VECTOR
{
    if(RI)
    {
      TI = 0;
    }
    if(TI)		// 发送完成
    {
			TI = 0;
    }
}