#include "STC32G.h"

//主频35Mhz，使用spi0与tft屏幕相连接

//定义常用数据类型
#define uint8_t unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long

// tft常用命令定义
#define SWRESET   0x01  // 软件复位
#define SLPOUT    0x11  // 退出睡眠模式
#define COLMOD    0x3A  // 设置颜色格式
#define MADCTL    0x36  // 显存访问方向控制
#define CASET     0x2A  // 列地址设置
#define RASET     0x2B  // 行地址设置
#define DISPON    0x29  // 开启显示


// tft常用颜色
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define WHITE   0xFFFF
#define BLACK   0x0000

//定义tft屏幕接线引脚
sbit LCD_DC=P0^1;
sbit LCD_RST=P0^0;

/*---------------软件延时--------------*/
void Delay20ms(void)	//@35.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 174998UL;
	while (i) i--;
}

void Delay200ms(void)	//@35.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 1749998UL;
	while (i) i--;
}




/*---------------与电脑通讯-------------*/

//uart初始化函数
/*可用波特率
972200


*/
void Uart1_Init(void)	//972200bps@35.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器时钟1T模式
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xF7;			//设置定时初始值
	TH1 = 0xFF;			//设置定时初始值
	ET1 = 0;			//禁止定时器中断
	TR1 = 1;			//定时器1开始计时
}


char Uart_Read(){
	while(!RI);
	RI=0;
	return SBUF;
}







void Uart_Write(uint8_t str){
	SBUF=str;
	while(!TI);
	TI=0;
}
/*
//uart中断函数

void	UartItrpt() interrupt 4
{
	if(RI){
		RI=0;
		SBUF=SBUF;
	}
}
*/

/*---------------与屏幕通讯-------------*/


/*---------------spi函数---------------*/
void SPI_Init(){
	P_SW1&=0xf3;
	/*
	一系列配置
	SSIG=1
	SPEN=1
	DORD=0
	MSTR=1
	CPOL=0
	CPHA=1
	SPR=11
	*/
	SPCTL=0xDF;
	
	SPSTAT=0xc0;//清空标志
	ESPI=0;//禁用SPI中断
	
	//配置高速spi
	CLKSEL|=0x80;
	USBCLK|=0x80;
	Delay200ms();
	CLKSEL|=0x40;
	HSCLKDIV=0;
	HSSPI_CFG2|=0x20;
	
}
/*
void SPI_Isr() interrupt 9
{
	SPIF=1;
	//SPDAT=???;
}
*/

/*-------------屏幕函数-----------*/


//写入命令
void LCD_WriteCmd(uint8_t cmd){
	LCD_DC=0;//命令模式
	SPDAT=cmd;//写入
	while(!SPIF);//等待到命令写完
	SPIF=1;//清除标志
}

//写入数据
void LCD_WriteData(uint8_t cmd){
	LCD_DC=1;//数据模式
	SPDAT=cmd;//写入
	while(!SPIF);//等待到命令写完
	SPIF=1;//清除标志
}

//写16位颜色数据（RGB565）
void LCD_WriteColor(uint16_t color){
	LCD_WriteData(color >>8);	//高字节
	LCD_WriteData(color);			//低字节
}

//设置显示窗口
void SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
		x0+=2;
		x1+=2;
		y0+=3;//我不知道为什么坐标会出现这样的偏移，但这样是好用的（大雾
		y1+=3;
    LCD_WriteCmd(CASET);
    LCD_WriteData(0x00); LCD_WriteData(x0);
    LCD_WriteData(0x00); LCD_WriteData(x1);
    
    LCD_WriteCmd(RASET);
    LCD_WriteData(0x00); LCD_WriteData(y0);
    LCD_WriteData(0x00); LCD_WriteData(y1);
    
    LCD_WriteCmd(0x2C);  // 显存写入命令
}

// 全屏填充颜色
void LCD_FillScreen(uint16_t color) {
		uint32_t total = 128 * 128;
    SetWindow(0, 0, 127, 127);
    
    LCD_DC = 1;  // 进入数据模式
    
    
    while(total--) {
        LCD_WriteColor(color);
    }
}
//屏幕初始化
void LCD_Init() {
    // 硬件复位
    LCD_RST=0;
    Delay20ms();
    LCD_RST=1;
    Delay20ms();
    // 软件初始化序列
    LCD_WriteCmd(SWRESET); Delay20ms();
    LCD_WriteCmd(SLPOUT);  Delay200ms();  // 必须延时>120ms
    
    // 设置16位色RGB565
    LCD_WriteCmd(COLMOD);
    LCD_WriteData(0x05);  // 0x05=16bit/pixel
    
    // 显存访问方向（参数可调整旋转方向）
    LCD_WriteCmd(MADCTL);
    /* 参数说明（常用值）：
     * 0xA0: 横向显示 (0°)
     * 0x60: 竖向显示 (90°)
     * 0xC0: 横向翻转(180°)
     * 0x00: 竖向翻转(270°)
     */
    LCD_WriteData(0xC0|0x08);//|0x08是设置RGB格式
    
    // 设置显示区域（128x128全屏）
    SetWindow(0, 0, 127, 127);
    
    // 开启显示
    LCD_WriteCmd(DISPON);
    //Delay200ms();
}
void main(){
	//spi要求
	P1M0 |= 0x28;  // 0011 1000
	P1M1 &= 0x00; // 1100 0111
	//uart要求
	P3M0 = 0x00; P3M1 = 0x00; 
	//屏幕的res&dc要求
	P0M0 = 0x00; P0M1 = 0x00; 
	//硬件初始化
	Uart1_Init();
	SPI_Init();
	//屏幕初始化
	LCD_Init();
	Uart_Write(0x88);
	
	LCD_FillScreen(0x0000);
	
	SetWindow(0,0,85,47);
	
	LCD_DC=1;//屏幕进入数据模式
	
	
	while(1){
		LCD_WriteData(Uart_Read());
	}
	
	
}



