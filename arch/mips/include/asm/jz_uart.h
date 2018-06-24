/*
 * Ingenic XBurst SoC UART block definitions
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
 */

#ifndef __JZ_UART_H__
#define __JZ_UART_H__

struct jz_uart {
	uint8_t rbr_thr_dllr;
		/* 0x00 R  8b receive buffer reg */
		/* 0x00 W  8b transmit hold reg */
		/* 0x00 RW 8b divisor latch low reg */
	uint8_t pad00[3];
	uint8_t dlhr_ier;
		/* 0x04 RW 8b divisor latch high reg */
		/* 0x04 RW 8b interrupt enable reg */
	uint8_t pad01[3];
	uint8_t iir_fcr;
		/* 0x08 R  8b interrupt identification reg */
		/* 0x08 W  8b FIFO control reg */
	uint8_t pad02[3];
	uint8_t lcr;	/* 0x0C RW 8b Line control reg */
	uint8_t pad03[3];
	uint8_t mcr;	/* 0x10 RW 8b modem control reg */
	uint8_t pad04[3];
	uint8_t lsr;	/* 0x14 R  8b line status reg */
	uint8_t pad05[3];
	uint8_t msr;	/* 0x18 R  8b modem status reg */
	uint8_t pad06[3];
	uint8_t spr;	/* 0x1C RW 8b scratch pad reg */
	uint8_t pad07[3];
	uint8_t isr;	/* 0x20 RW 8b infrared selection reg */
	uint8_t pad08[3];
	uint8_t umr;	/* 0x24 RW 8b */
};

/*
 * Define macros for UART_IER
 * UART Interrupt Enable Register
 */
#define UART_IER_RIE	(1 << 0) /* 0: receive fifo full interrupt disable */
#define UART_IER_TIE	(1 << 1) /* 0: transmit fifo empty interrupt disable */
#define UART_IER_RLIE	(1 << 2) /* 0: receive line status interrupt disable */
#define UART_IER_MIE	(1 << 3) /* 0: modem status interrupt disable */
#define UART_IER_RTIE	(1 << 4) /* 0: receive timeout interrupt disable */

/*
 * Define macros for UART_ISR
 * UART Interrupt Status Register
 */
#define UART_ISR_IP	(1 << 0) /* 0: interrupt is pending 1: no interrupt */
#define UART_ISR_IID	(7 << 1) /* Source of Interrupt */
#define UART_ISR_IID_MSI  (0 << 1) /* Modem status interrupt */
#define UART_ISR_IID_THRI (1 << 1) /* Transmitter holding register empty */
#define UART_ISR_IID_RDI  (2 << 1) /* Receiver data interrupt */
#define UART_ISR_IID_RLSI (3 << 1) /* Receiver line status interrupt */
/* FIFO mode select, set when UART_FCR.FE is set to 1 */
#define UART_ISR_FFMS	(3 << 6)
#define UART_ISR_FFMS_NO_FIFO	(0 << 6)
#define UART_ISR_FFMS_FIFO_MODE	(3 << 6)

/*
 * Define macros for UART_FCR
 * UART FIFO Control Register
 */
#define UART_FCR_FE	(1 << 0)	/* 0: non-FIFO mode  1: FIFO mode */
#define UART_FCR_RFLS	(1 << 1)	/* write 1 to flush receive FIFO */
#define UART_FCR_TFLS	(1 << 2)	/* write 1 to flush transmit FIFO */
#define UART_FCR_DMS	(1 << 3)	/* 0: disable DMA mode */
#define UART_FCR_UUE	(1 << 4)	/* 0: disable UART */
#define UART_FCR_RTRG	(3 << 6)	/* Receive FIFO Data Trigger */
#define UART_FCR_RTRG_1	(0 << 6)
#define UART_FCR_RTRG_4	(1 << 6)
#define UART_FCR_RTRG_8	(2 << 6)
#define UART_FCR_RTRG_15	(3 << 6)

/*
 * Define macros for UART_LCR
 * UART Line Control Register
 */
#define UART_LCR_WLEN	(3 << 0)	/* word length */
#define UART_LCR_WLEN_5	(0 << 0)
#define UART_LCR_WLEN_6	(1 << 0)
#define UART_LCR_WLEN_7	(2 << 0)
#define UART_LCR_WLEN_8	(3 << 0)
#define UART_LCR_STOP	(1 << 2)
	/* 0: 1 stop bit when word length is 5,6,7,8
	   1: 1.5 stop bits when 5; 2 stop bits when 6,7,8 */
#define UART_LCR_STOP_1	(0 << 2)
	/* 0: 1 stop bit when word length is 5,6,7,8
	   1: 1.5 stop bits when 5; 2 stop bits when 6,7,8 */
#define UART_LCR_STOP_2	(1 << 2)
	/* 0: 1 stop bit when word length is 5,6,7,8
	   1: 1.5 stop bits when 5; 2 stop bits when 6,7,8 */

#define UART_LCR_PE	(1 << 3) /* 0: parity disable */
#define UART_LCR_PROE	(1 << 4) /* 0: even parity  1: odd parity */
#define UART_LCR_SPAR	(1 << 5) /* 0: sticky parity disable */
#define UART_LCR_SBRK	(1 << 6) /* write 0 normal, write 1 send break */
/* 0: access UART_RDR/TDR/IER  1: access UART_DLLR/DLHR */
#define UART_LCR_DLAB	(1 << 7)

/*
 * Define macros for UART_LSR
 * UART Line Status Register
 */
/* 0: receive FIFO is empty  1: receive data is ready */
#define UART_LSR_DR	(1 << 0)
/* 0: no overrun error */
#define UART_LSR_ORER	(1 << 1)
/* 0: no parity error */
#define UART_LSR_PER	(1 << 2)
/* 0; no framing error */
#define UART_LSR_FER	(1 << 3)
/* 0: no break detected  1: receive a break signal */
#define UART_LSR_BRK	(1 << 4)
/* 1: transmit FIFO half "empty" */
#define UART_LSR_TDRQ	(1 << 5)
/* 1: transmit FIFO and shift registers empty */
#define UART_LSR_TEMT	(1 << 6)
/* 0: no receive error  1: receive error in FIFO mode */
#define UART_LSR_RFER	(1 << 7)

/*
 * Define macros for UART_MCR
 * UART Modem Control Register
 */
#define UART_MCR_DTR	(1 << 0) /* 0: DTR_ ouput high */
#define UART_MCR_RTS	(1 << 1) /* 0: RTS_ output high */
/* 0: UART_MSR.RI is set to 0 and RI_ input high */
#define UART_MCR_OUT1	(1 << 2)
/* 0: UART_MSR.DCD is set to 0 and DCD_ input high */
#define UART_MCR_OUT2	(1 << 3)
#define UART_MCR_LOOP	(1 << 4) /* 0: normal  1: loopback mode */
#define UART_MCR_MCE	(1 << 7) /* 0: modem function is disable */

/*
 * Define macros for UART_MSR
 * UART Modem Status Register
 */
#define UART_MSR_DCTS	(1 << 0) /* 0: no change on CTS_ since last read */
#define UART_MSR_DDSR	(1 << 1) /* 0: no change on DSR_ since last read */
#define UART_MSR_DRI	(1 << 2) /* 0: no change on RI_  since last read */
#define UART_MSR_DDCD	(1 << 3) /* 0: no change on DCD_ since last read */
#define UART_MSR_CTS	(1 << 4) /* 0: CTS_ pin is high */
#define UART_MSR_DSR	(1 << 5) /* 0: DSR_ pin is high */
#define UART_MSR_RI	(1 << 6) /* 0: RI_ pin is high */
#define UART_MSR_DCD	(1 << 7) /* 0: DCD_ pin is high */

/*
 * Define macros for SIRCR
 * Slow IrDA Control Register
 */
#define SIRCR_TSIRE (1 << 0) /* 0: TX is in UART mode 1: IrDA mode */
#define SIRCR_RSIRE (1 << 1) /* 0: RX is in UART mode 1: IrDA mode */
#define SIRCR_TPWS  (1 << 2) /* 0: transmit 0 pulse width is 3/16 of bit length
				1: 0 pulse width is 1.6us for 115.2Kbps */
#define SIRCR_TXPL  (1 << 3) /* 0: encoder generates a positive pulse for 0 */
#define SIRCR_RXPL  (1 << 4) /* 0: decoder interprets positive pulse as 0 */

#endif /* __JZ_UART_H__ */
