
#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <jz_lcd/byd_8991.h>

//#define DEBUG

extern struct byd_8991_data byd_8991_pdata;

/*PB30  LCD_RESET   */
#define RESET(n)\
     gpio_direction_output(byd_8991_pdata.gpio_lcd_disp, n)

/*PC0  SPI_CS   */
#define CS(n)\
     gpio_direction_output(byd_8991_pdata.gpio_spi_cs, n)

/*PC1  SPI_CLK   */
#define SCK(n)\
     gpio_direction_output(byd_8991_pdata.gpio_spi_clk, n)

/*PC10 SPI_MOSI */
#define SDO(n)\
	 gpio_direction_output(byd_8991_pdata.gpio_spi_mosi, n)

/*PC11 SPI_MISO */
#define SDI()\
	gpio_get_value(byd_8991_pdata.gpio_spi_miso)


void SPI_3W_SET_CMD(unsigned char c)
{
	unsigned char i;
	
/*delete timedelays for increasing the start-up speed.
    Oscilloscope test shows that : without timedelays,the signal CKL's high level
    is able to keep 360ns,and the low level can keep up to 700ns,meeting with the lcd's 
    timing requirements(see details in the 8991F.PDF  page18).
    if your 8991_lcd can't work normally because of the chang,try to recover those udelays,and connect with me.
    */
//	udelay(1);             
	SCK(0);
	SDO(0);
//	udelay(1);
	SCK(1);
//	udelay(2);
	for(i=0;i<8;i++)
	{
		SCK(0);
		SDO(((c&0x80)>>7));
//		udelay(1);
		SCK(1);
//		udelay(2);
		c=c<<1;
	}
}

void SPI_3W_SET_PAs(unsigned char d)
{
	unsigned char i;
	
//	udelay(1);
	SCK(0);
	SDO(1);
//	udelay(1);
	SCK(1);
//	udelay(2);
	for(i=0;i<8;i++)
	{
		SCK(0);
		SDO(((d&0x80)>>7));
//		udelay(1);
		SCK(1);
//		udelay(2);
		d=d<<1;
	}
}
unsigned char SPI_GET_REG_VAL()
{
	unsigned char i;
	unsigned char data = 0;
	unsigned char tmp = 0;

//	udelay(1);
	for(i=0;i<8;i++)
	{
		SCK(0);
		data <<= 1;
		tmp=SDI();
		data|=tmp;
//		udelay(1);
		SCK(1);
//		udelay(2);
		serial_puts("sdi= ");
		serial_putc((tmp+0x30));
		serial_putc('\n');
	}
	return data;
}

unsigned char SPI_READ_REG(unsigned char reg)
{
	int data = 0;
	
	CS(0);
//	udelay(1);
	SPI_3W_SET_CMD(0xB9); //Set_EXTC
	SPI_3W_SET_PAs(0xFF);
	SPI_3W_SET_PAs(0x83);
	SPI_3W_SET_PAs(0x69);
	
	SPI_3W_SET_CMD(reg);
	data = SPI_GET_REG_VAL();
	CS(1);
//	udelay(1);
	serial_puts("reg = 0x");
	serial_putc((data+0x30));
	serial_putc('\n');
}

void Initial_IC(void)
{

	RESET(1);
	udelay(1000); // Delay 1ms
	RESET(0);
	udelay(10000); // Delay 10ms // This delay time is necessary
	RESET(1);
	udelay(100000); // Delay 100 ms

#ifdef DEBUG
	while(1){
		/*The default reg(0x08)'s value is 0x08, if the	IC is working*/
		SPI_READ_REG(0X0A);
	}
#endif

	CS(0);
//	udelay(10);
	
	SPI_3W_SET_CMD(0xB9); //Set_EXTC
	SPI_3W_SET_PAs(0xFF);
	SPI_3W_SET_PAs(0x83);
	SPI_3W_SET_PAs(0x69);

	SPI_3W_SET_CMD(0xB1); //Set Power
	SPI_3W_SET_PAs(0x85);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x34);
	SPI_3W_SET_PAs(0x0A);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x2A);
	SPI_3W_SET_PAs(0x32);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x01);
	SPI_3W_SET_PAs(0x23);
	SPI_3W_SET_PAs(0x01);
	SPI_3W_SET_PAs(0xE6);
	SPI_3W_SET_PAs(0xE6);
	SPI_3W_SET_PAs(0xE6);
	SPI_3W_SET_PAs(0xE6);
	SPI_3W_SET_PAs(0xE6);

	SPI_3W_SET_CMD(0xB2); // SET Display 480x800
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x23);
	SPI_3W_SET_PAs(0x05);
	SPI_3W_SET_PAs(0x05);
	SPI_3W_SET_PAs(0x70);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0xFF);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x03);
	SPI_3W_SET_PAs(0x03);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x01);
	
	SPI_3W_SET_CMD(0xB4); // SET Display column inversion
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x18);
	SPI_3W_SET_PAs(0x80);
	SPI_3W_SET_PAs(0x06);
	SPI_3W_SET_PAs(0x02);
	
	SPI_3W_SET_CMD(0xB6); // SET VCOM
	SPI_3W_SET_PAs(0x3A);
	SPI_3W_SET_PAs(0x3A);
	
	SPI_3W_SET_CMD(0xD5); // SETGIP
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x03);
	SPI_3W_SET_PAs(0x03);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x01);
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x28);
	SPI_3W_SET_PAs(0x70);
	SPI_3W_SET_PAs(0x11);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x40);
	SPI_3W_SET_PAs(0x06);
	SPI_3W_SET_PAs(0x51);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x41);
	SPI_3W_SET_PAs(0x06);
	SPI_3W_SET_PAs(0x50);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x00);
	
	SPI_3W_SET_CMD(0xE0); // Set Gamma
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x19);
	SPI_3W_SET_PAs(0x38);
	SPI_3W_SET_PAs(0x3D);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x28);
	SPI_3W_SET_PAs(0x46);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x0D);
	SPI_3W_SET_PAs(0x0E);
	SPI_3W_SET_PAs(0x12);
	SPI_3W_SET_PAs(0x15);
	SPI_3W_SET_PAs(0x12);
	SPI_3W_SET_PAs(0x14);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x17);
	SPI_3W_SET_PAs(0x00);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x19);
	SPI_3W_SET_PAs(0x38);
	SPI_3W_SET_PAs(0x3D);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x28);
	SPI_3W_SET_PAs(0x46);
	SPI_3W_SET_PAs(0x07);
	SPI_3W_SET_PAs(0x0D);
	SPI_3W_SET_PAs(0x0E);
	SPI_3W_SET_PAs(0x12);
	SPI_3W_SET_PAs(0x15);
	SPI_3W_SET_PAs(0x12);
	SPI_3W_SET_PAs(0x14);
	SPI_3W_SET_PAs(0x0F);
	SPI_3W_SET_PAs(0x17);
	
	SPI_3W_SET_CMD(0xC1); //Set DGC function
	SPI_3W_SET_PAs(0x01);
	//R
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x1A);
	SPI_3W_SET_PAs(0x20);
	SPI_3W_SET_PAs(0x27);
	SPI_3W_SET_PAs(0x2C);
	SPI_3W_SET_PAs(0x32);
	SPI_3W_SET_PAs(0x36);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x47);
	SPI_3W_SET_PAs(0x50);
	SPI_3W_SET_PAs(0x59);
	SPI_3W_SET_PAs(0x60);
	SPI_3W_SET_PAs(0x68);
	SPI_3W_SET_PAs(0x71);
	SPI_3W_SET_PAs(0x7B);
	SPI_3W_SET_PAs(0x82);
	SPI_3W_SET_PAs(0x89);
	SPI_3W_SET_PAs(0x91);
	SPI_3W_SET_PAs(0x98);
	SPI_3W_SET_PAs(0xA0);
	SPI_3W_SET_PAs(0xA8);
	SPI_3W_SET_PAs(0xB0);
	SPI_3W_SET_PAs(0xB8);
	SPI_3W_SET_PAs(0xC1);
	SPI_3W_SET_PAs(0xC9);
	SPI_3W_SET_PAs(0xD0);
	SPI_3W_SET_PAs(0xD7);
	SPI_3W_SET_PAs(0xE0);
	SPI_3W_SET_PAs(0xE7);
	SPI_3W_SET_PAs(0xEF);
	SPI_3W_SET_PAs(0xF7);
	SPI_3W_SET_PAs(0xFE);
	SPI_3W_SET_PAs(0xCF);
	SPI_3W_SET_PAs(0x52);
	SPI_3W_SET_PAs(0x34);
	SPI_3W_SET_PAs(0xF8);
	SPI_3W_SET_PAs(0x51);
	SPI_3W_SET_PAs(0xF5);
	SPI_3W_SET_PAs(0x9D);
	SPI_3W_SET_PAs(0x75);
	SPI_3W_SET_PAs(0x00);
	//G
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x1A);
	SPI_3W_SET_PAs(0x20);
	SPI_3W_SET_PAs(0x27);
	SPI_3W_SET_PAs(0x2C);
	SPI_3W_SET_PAs(0x32);
	SPI_3W_SET_PAs(0x36);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x47);
	SPI_3W_SET_PAs(0x50);
	SPI_3W_SET_PAs(0x59);
	SPI_3W_SET_PAs(0x60);
	SPI_3W_SET_PAs(0x68);
	SPI_3W_SET_PAs(0x71);
	SPI_3W_SET_PAs(0x7B);
	SPI_3W_SET_PAs(0x82);
	SPI_3W_SET_PAs(0x89);
	SPI_3W_SET_PAs(0x91);
	SPI_3W_SET_PAs(0x98);
	SPI_3W_SET_PAs(0xA0);
	SPI_3W_SET_PAs(0xA8);
	SPI_3W_SET_PAs(0xB0);
	SPI_3W_SET_PAs(0xB8);
	SPI_3W_SET_PAs(0xC1);
	SPI_3W_SET_PAs(0xC9);
	SPI_3W_SET_PAs(0xD0);
	SPI_3W_SET_PAs(0xD7);
	SPI_3W_SET_PAs(0xE0);
	SPI_3W_SET_PAs(0xE7);
	SPI_3W_SET_PAs(0xEF);
	SPI_3W_SET_PAs(0xF7);
	SPI_3W_SET_PAs(0xFE);
	SPI_3W_SET_PAs(0xCF);
	SPI_3W_SET_PAs(0x52);
	SPI_3W_SET_PAs(0x34);
	SPI_3W_SET_PAs(0xF8);
	SPI_3W_SET_PAs(0x51);
	SPI_3W_SET_PAs(0xF5);
	SPI_3W_SET_PAs(0x9D);
	SPI_3W_SET_PAs(0x75);
	SPI_3W_SET_PAs(0x00);
	//B
	SPI_3W_SET_PAs(0x04);
	SPI_3W_SET_PAs(0x13);
	SPI_3W_SET_PAs(0x1A);
	SPI_3W_SET_PAs(0x20);
	SPI_3W_SET_PAs(0x27);
	SPI_3W_SET_PAs(0x2C);
	SPI_3W_SET_PAs(0x32);
	SPI_3W_SET_PAs(0x36);
	SPI_3W_SET_PAs(0x3F);
	SPI_3W_SET_PAs(0x47);
	SPI_3W_SET_PAs(0x50);
	SPI_3W_SET_PAs(0x59);
	SPI_3W_SET_PAs(0x60);
	SPI_3W_SET_PAs(0x68);
	SPI_3W_SET_PAs(0x71);
	SPI_3W_SET_PAs(0x7B);
	SPI_3W_SET_PAs(0x82);
	SPI_3W_SET_PAs(0x89);
	SPI_3W_SET_PAs(0x91);
	SPI_3W_SET_PAs(0x98);
	SPI_3W_SET_PAs(0xA0);
	SPI_3W_SET_PAs(0xA8);
	SPI_3W_SET_PAs(0xB0);
	SPI_3W_SET_PAs(0xB8);
	SPI_3W_SET_PAs(0xC1);
	SPI_3W_SET_PAs(0xC9);
	SPI_3W_SET_PAs(0xD0);
	SPI_3W_SET_PAs(0xD7);
	SPI_3W_SET_PAs(0xE0);
	SPI_3W_SET_PAs(0xE7);
	SPI_3W_SET_PAs(0xEF);
	SPI_3W_SET_PAs(0xF7);
	SPI_3W_SET_PAs(0xFE);
	SPI_3W_SET_PAs(0xCF);
	SPI_3W_SET_PAs(0x52);
	SPI_3W_SET_PAs(0x34);
	SPI_3W_SET_PAs(0xF8);
	SPI_3W_SET_PAs(0x51);
	SPI_3W_SET_PAs(0xF5);
	SPI_3W_SET_PAs(0x9D);
	SPI_3W_SET_PAs(0x75);
	SPI_3W_SET_PAs(0x00);


	SPI_3W_SET_CMD(0x3A); //Set COLMOD
	SPI_3W_SET_PAs(0x77);

	SPI_3W_SET_CMD(0x11); //Sleep Out
	udelay(150000); //at least 120ms

	SPI_3W_SET_CMD(0x29); //Display On 
	CS(1);
}
