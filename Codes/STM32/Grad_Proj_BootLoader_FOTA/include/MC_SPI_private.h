/*
 * MC_SPI_private.h
 *
 *  Created on: 4 Sep 2026
 *      Author: Sief
 */
#ifndef MC_SPI_PRIVATE_H_
#define MC_SPI_PRIVATE_H_

#define BASE_ADDRESS_SPI1 		(0x40013000)	// APB2
#define BASE_ADDRESS_SPI2 		(0x40003800)	// APB1
#define BASE_ADDRESS_SPI3 		(0x40003C00)	// APB1

typedef struct
{
	u32 SPI_CR1  	; //0x00
	u32 SPI_CR2		; //0x04
	u32 SPI_SR  	; //0x08
	u32 SPI_DR 		; //0x0C
	u32 SPI_CRCPR 	; //0x10
	u32 SPI_RXCRCR  ; //0x14
	u32 SPI_TXCRCR 	; //0x18
	u32 SPI_I2SCFGR	; //0x1C
	u32 SPI_I2SPR	; //0x20
}SPI_type;

#define SPI1 ((volatile SPI_type*)(BASE_ADDRESS_SPI1))
#define SPI2 ((volatile SPI_type*)(BASE_ADDRESS_SPI2))
#define SPI3 ((volatile SPI_type*)(BASE_ADDRESS_SPI3))

/* ------------------- SPI_CR1 Bit Positions ------------------- */
#define SPI_CR1_CPHA        0U
#define SPI_CR1_CPOL        1U
#define SPI_CR1_MSTR        2U
#define SPI_CR1_BR0         3U
#define SPI_CR1_BR1         4U
#define SPI_CR1_BR2         5U
#define SPI_CR1_SPE         6U
#define SPI_CR1_LSBFIRST    7U
#define SPI_CR1_SSI         8U
#define SPI_CR1_SSM         9U
#define SPI_CR1_RXONLY      10U
#define SPI_CR1_DFF         11U
#define SPI_CR1_CRCNEXT     12U
#define SPI_CR1_CRCEN       13U
#define SPI_CR1_BIDIOE      14U
#define SPI_CR1_BIDIMODE    15U

/* ------------------- SPI_CR2 Bit Positions ------------------- */
#define SPI_CR2_RXDMAEN     0U
#define SPI_CR2_TXDMAEN     1U
#define SPI_CR2_SSOE        2U
#define SPI_CR2_FRF         4U
#define SPI_CR2_ERRIE       5U
#define SPI_CR2_RXNEIE      6U
#define SPI_CR2_TXEIE       7U

/* ------------------- SPI_SR Bit Positions ------------------- */
#define SPI_SR_RXNE         0U
#define SPI_SR_TXE          1U
#define SPI_SR_CHSIDE       2U
#define SPI_SR_UDR          3U
#define SPI_SR_CRCERR       4U
#define SPI_SR_MODF         5U
#define SPI_SR_OVR          6U
#define SPI_SR_BSY          7U
#define SPI_SR_FRE          8U

#endif /* MC_SPI_PRIVATE_H_ */
