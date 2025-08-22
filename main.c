#include "STC32G.h"

//���峣����������
#define uint8_t unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long

// tft���������
#define SWRESET   0x01  // �����λ
#define SLPOUT    0x11  // �˳�˯��ģʽ
#define COLMOD    0x3A  // ������ɫ��ʽ
#define MADCTL    0x36  // �Դ���ʷ������
#define CASET     0x2A  // �е�ַ����
#define RASET     0x2B  // �е�ַ����
#define DISPON    0x29  // ������ʾ


// tft������ɫ
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define WHITE   0xFFFF
#define BLACK   0x0000

//����tft��Ļ��������
sbit LCD_DC=P0^1;
sbit LCD_RST=P0^0;

/*---------------�����ʱ--------------*/
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




/*---------------�����ͨѶ-------------*/

//uart��ʼ������
/*���ò�����
972200


*/
void Uart1_Init(void)	//972200bps@35.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��ʱ��1Tģʽ
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//���ö�ʱ��ģʽ
	TL1 = 0xF7;			//���ö�ʱ��ʼֵ
	TH1 = 0xFF;			//���ö�ʱ��ʼֵ
	ET1 = 0;			//��ֹ��ʱ���ж�
	TR1 = 1;			//��ʱ��1��ʼ��ʱ
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
//uart�жϺ���

void	UartItrpt() interrupt 4
{
	if(RI){
		RI=0;
		SBUF=SBUF;
	}
}
*/

/*---------------����ĻͨѶ-------------*/


/*---------------spi����---------------*/
void SPI_Init(){
	P_SW1&=0xf3;
	/*
	һϵ������
	SSIG=1
	SPEN=1
	DORD=0
	MSTR=1
	CPOL=0
	CPHA=1
	SPR=11
	*/
	SPCTL=0xDF;
	
	SPSTAT=0xc0;//��ձ�־
	ESPI=0;//����SPI�ж�
	
	//���ø���spi
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

/*-------------��Ļ����-----------*/


//д������
void LCD_WriteCmd(uint8_t cmd){
	LCD_DC=0;//����ģʽ
	SPDAT=cmd;//д��
	while(!SPIF);//�ȴ�������д��
	SPIF=1;//�����־
}

//д������
void LCD_WriteData(uint8_t cmd){
	LCD_DC=1;//����ģʽ
	SPDAT=cmd;//д��
	while(!SPIF);//�ȴ�������д��
	SPIF=1;//�����־
}

//д16λ��ɫ���ݣ�RGB565��
void LCD_WriteColor(uint16_t color){
	LCD_WriteData(color >>8);	//���ֽ�
	LCD_WriteData(color);			//���ֽ�
}

//������ʾ����
void SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
		x0+=2;
		x1+=2;
		y0+=3;//�Ҳ�֪��Ϊʲô��������������ƫ�ƣ��������Ǻ��õģ�����
		y1+=3;
    LCD_WriteCmd(CASET);
    LCD_WriteData(0x00); LCD_WriteData(x0);
    LCD_WriteData(0x00); LCD_WriteData(x1);
    
    LCD_WriteCmd(RASET);
    LCD_WriteData(0x00); LCD_WriteData(y0);
    LCD_WriteData(0x00); LCD_WriteData(y1);
    
    LCD_WriteCmd(0x2C);  // �Դ�д������
}

// ȫ�������ɫ
void LCD_FillScreen(uint16_t color) {
		uint32_t total = 128 * 128;
    SetWindow(0, 0, 127, 127);
    
    LCD_DC = 1;  // ��������ģʽ
    
    
    while(total--) {
        LCD_WriteColor(color);
    }
}
//��Ļ��ʼ��
void LCD_Init() {
    // Ӳ����λ
    LCD_RST=0;
    Delay20ms();
    LCD_RST=1;
    Delay20ms();
    // �����ʼ������
    LCD_WriteCmd(SWRESET); Delay20ms();
    LCD_WriteCmd(SLPOUT);  Delay200ms();  // ������ʱ>120ms
    
    // ����16λɫRGB565
    LCD_WriteCmd(COLMOD);
    LCD_WriteData(0x05);  // 0x05=16bit/pixel
    
    // �Դ���ʷ��򣨲����ɵ�����ת����
    LCD_WriteCmd(MADCTL);
    /* ����˵��������ֵ����
     * 0xA0: ������ʾ (0��)
     * 0x60: ������ʾ (90��)
     * 0xC0: ����ת(180��)
     * 0x00: ����ת(270��)
     */
    LCD_WriteData(0xC0|0x08);//|0x08������RGB��ʽ
    
    // ������ʾ����128x128ȫ����
    SetWindow(0, 0, 127, 127);
    
    // ������ʾ
    LCD_WriteCmd(DISPON);
    //Delay200ms();
}
void main(){
	//spiҪ��
	P1M0 |= 0x28;  // 0011 1000
	P1M1 &= 0x00; // 1100 0111
	//uartҪ��
	P3M0 = 0x00; P3M1 = 0x00; 
	//��Ļ��res&dcҪ��
	P0M0 = 0x00; P0M1 = 0x00; 
	//Ӳ����ʼ��
	Uart1_Init();
	SPI_Init();
	//��Ļ��ʼ��
	LCD_Init();
	Uart_Write(0x88);
	
	LCD_FillScreen(0x0000);
	
	SetWindow(0,0,85,47);
	
	LCD_DC=1;//��Ļ��������ģʽ
	
	
	while(1){
		LCD_WriteData(Uart_Read());
	}
	
	
}


