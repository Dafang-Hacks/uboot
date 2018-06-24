/*
 * (C) Copyright 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * This has been changed substantially by Gerald Van Baren, Custom IDEAS,
 * vanbaren@cideas.com.  It was heavily influenced by LiMon, written by
 * Neil Russell.
 */
//#define	DEBUG

#include <common.h>
#include <ingenic_soft_i2c.h>

#define I2C_ACK		0		/* PD_SDA level to ack a byte */
#define I2C_NOACK	1		/* PD_SDA level to noack a byte */

/*-----------------------------------------------------------------------
 * Local functions
 */
static void send_reset(struct i2c *i2c);
static void send_start(struct i2c *i2c);
static void send_stop(struct i2c *i2c);
static void send_ack(struct i2c *i2c,int);
static int  write_byte(struct i2c *i2c,unsigned char byte);
static unsigned char read_byte(struct i2c *i2c,int);

static void i2c_scl(struct i2c *i2c,int bit)
{
	gpio_direction_output(i2c->scl,bit);
}

static void i2c_sda(struct i2c *i2c,int bit)
{
	if(bit) {
		gpio_direction_input(i2c->sda);
	} else {
		gpio_direction_output(i2c->sda,bit);
	}
}

static int i2c_read_sda(struct i2c *i2c)
{
	return gpio_get_value(i2c->sda);
}

/*-----------------------------------------------------------------------
 * Send a reset sequence consisting of 9 clocks with the data signal high
 * to clock any confused device back into an idle state.  Also send a
 * <stop> at the end of the sequence for belts & suspenders.
 */
static void send_reset(struct i2c *i2c)
{
	int j;

	i2c_scl(i2c,1);
	i2c_sda(i2c,1);
	for(j = 0; j < 9; j++) {
		i2c_scl(i2c,0);
		udelay(5);
		udelay(5);
		i2c_scl(i2c,1);
		udelay(5);
		udelay(5);
	}
	send_stop(i2c);
}

/*-----------------------------------------------------------------------
 * START: High -> Low on SDA while SCL is High
 */
static void send_start(struct i2c *i2c)
{

	udelay(5);
	i2c_sda(i2c,1);
	udelay(5);
	i2c_scl(i2c,1);
	udelay(5);
	i2c_sda(i2c,0);
	udelay(5);
}

/*-----------------------------------------------------------------------
 * STOP: Low -> High on SDA while SCL is High
 */
static void send_stop(struct i2c *i2c)
{
	i2c_scl(i2c,0);
	udelay(5);
	i2c_sda(i2c,0);
	udelay(5);
	i2c_scl(i2c,1);
	udelay(5);
	i2c_sda(i2c,1);
	udelay(5);
}

/*-----------------------------------------------------------------------
 * ack should be I2C_ACK or I2C_NOACK
 */
static void send_ack(struct i2c *i2c,int ack)
{
	i2c_scl(i2c,0);
	udelay(5);
	i2c_sda(i2c,ack);
	udelay(5);
	i2c_scl(i2c,1);
	udelay(5);
	udelay(5);
	i2c_scl(i2c,0);
	udelay(5);
}

/*-----------------------------------------------------------------------
 * Send 8 bits and look for an acknowledgement.
 */
static int write_byte(struct i2c *i2c,unsigned char data)
{
	int j;
	int nack;

	for(j = 0; j < 8; j++) {
		i2c_scl(i2c,0);
		udelay(5);
		i2c_sda(i2c,data & 0x80);
		udelay(5);
		i2c_scl(i2c,1);
		udelay(5);
		udelay(5);

		data <<= 1;
	}

	/*
	 * Look for an <ACK>(negative logic) and return it.
	 */
	i2c_scl(i2c,0);
	udelay(5);
	i2c_sda(i2c,1);
	udelay(5);
	i2c_scl(i2c,1);
	udelay(5);
	udelay(5);
	nack = i2c_read_sda(i2c);
	i2c_scl(i2c,0);
	udelay(5);

	return(nack);	/* not a nack is an ack */
}

/*-----------------------------------------------------------------------
 * if ack == I2C_ACK, ACK the byte so can continue reading, else
 * send I2C_NOACK to end the read.
 */
static unsigned char read_byte(struct i2c *i2c,int ack)
{
	int  data;
	int  j;

	/*
	 * Read 8 bits, MSB first.
	 */
	i2c_sda(i2c,1);
	data = 0;
	for(j = 0; j < 8; j++) {
		i2c_scl(i2c,0);
		udelay(5);
		i2c_scl(i2c,1);
		udelay(5);
		data <<= 1;
		data |= i2c_read_sda(i2c);
		udelay(5);
	}
	send_ack(i2c,ack);

	return(data);
}

/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/

/*-----------------------------------------------------------------------
 * Initialization
 */
void i2c_init(struct i2c *i2c)
{
	gpio_request(i2c->scl, "soft_i2c");
	gpio_request(i2c->sda, "soft_i2c");
	gpio_direction_output(i2c->sda,1);
	gpio_direction_output(i2c->scl,1);
	send_reset(i2c);
}

/*-----------------------------------------------------------------------
 * Read bytes
 */
int  i2c_read(struct i2c *i2c,unsigned char chip,
		unsigned int addr, int alen, unsigned char *buffer, int len)
{
	int shift;
#ifdef DEBUG
	printf("i2c_read: chip %x addr %x alen %d buffer %p len %d\n",
			chip, addr, alen, buffer, len);
#endif
	/*
	 * Do the addressing portion of a write cycle to set the
	 * chip's address pointer.  If the address length is zero,
	 * don't do the normal write cycle to set the address pointer,
	 * there is no address pointer in this chip.
	 */
	send_start(i2c);
	if(alen > 0) {
		if(write_byte(i2c,chip << 1)) {	/* write cycle */
			send_stop(i2c);
			printf("i2c_read, no chip responded %02X\n", chip);
			return(1);
		}
		shift = (alen-1) * 8;
		while(alen-- > 0) {
			if(write_byte(i2c,addr >> shift)) {
				printf("i2c_read, address not <ACK>ed\n");
				return(1);
			}
			shift -= 8;
		}

		/* Some I2C chips need a stop/start sequence here,
		 * other chips don't work with a full stop and need
		 * only a start.  Default behaviour is to send the
		 * stop/start sequence.
		 */
#ifdef CONFIG_SOFT_I2C_READ_REPEATED_START
		send_start(i2c);
#else
		send_stop(i2c);
		send_start(i2c);
#endif
	}
	/*
	 * Send the chip address again, this time for a read cycle.
	 * Then read the data.  On the last byte, we do a NACK instead
	 * of an ACK(len == 0) to terminate the read.
	 */
	write_byte(i2c,(chip << 1) | 1);	/* read cycle */
	while(len-- > 0) {
		*buffer++ = read_byte(i2c,len == 0);
	}
	send_stop(i2c);
	return(0);
}

/*-----------------------------------------------------------------------
 * Write bytes
 */
int  i2c_write(struct i2c *i2c,unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len)
{
	int shift, failures = 0;

#ifdef DEBUG
	printf("i2c_write: chip %x addr %x alen %d buffer %p len %d\n",
		chip, addr, alen, buffer, len);
#endif
	send_start(i2c);
	if(write_byte(i2c,chip << 1)) {	/* write cycle */
		send_stop(i2c);
		printf("i2c_write, no chip responded %02X\n", chip);
		return(1);
	}
	shift = (alen-1) * 8;
	while(alen-- > 0) {
		if(write_byte(i2c,addr >> shift)) {
			printf("i2c_write, address not <ACK>ed\n");
			return(1);
		}
		shift -= 8;
	}

	while(len-- > 0) {
		if(write_byte(i2c,*buffer++)) {
			failures++;
		}
	}
	send_stop(i2c);
	return(failures);
}

int i2c_probe(struct i2c *i2c, unsigned char addr)
{
	int rc;

	send_start(i2c);
	rc = write_byte(i2c, (addr << 1) | 0);
	send_stop(i2c);
	printf("i2c probe:%d, addr:%x\n", rc, addr);

	return (rc ? 1 : 0);
}
