#ifndef MC_USART_PRIVATE_H_
#define MC_USART_PRIVATE_H_

#define BASE_ADDRESS_USART6 		(0x40011400)	// APB2
#define BASE_ADDRESS_USART1 		(0x40011000)	// APB2

typedef struct
{
	u32 USART_SR  ; //0x00
	u32 USART_DR  ; //0x04
	u32 USART_BRR ; //0x08
	u32 USART_CR1 ; //0x0c
	u32 USART_CR2 ; //0x10
	u32 USART_CR3 ; //0x14
	u32 USART_GTPR; //0x18
}USART_type;

#define USART6 ((volatile USART_type*)(BASE_ADDRESS_USART6))
#define USART1 ((volatile USART_type*)(BASE_ADDRESS_USART1))

// bits of register USART_SR
#define 	CTS 	9
#define 	LBD 	8
#define 	TXE 	7
#define 	TC 		6
#define 	RXNE 	5
#define 	IDLE 	4
#define 	ORE 	3
#define 	NF	 	2
#define 	FE	 	1
#define 	PE	 	0

// bits of register USART_BRR
#define 	MANTISSA_BIT_POSITION    4
#define 	MANTISSA_MASK            0xFFF
#define 	FRACTION_BIT_POSITION    0
#define 	FRACTION_MASK            0xF

// bits of register USART_CR1
#define 	OVER8	15
#define 	UE	 	13
#define 	M 		12
#define 	WAKE 	11
#define 	PCE	 	10
#define 	PS	 	9
#define 	PEIE 	8
#define 	TXEIE	7
#define 	TCIE	6
#define 	RXNEIE	5
#define 	IDLEIE	4
#define 	TE 		3
#define 	RE	 	2
#define 	RWU	 	1
#define 	SBK	 	0

// bits of register USART_CR2
#define 	LINEN 					14
#define 	STOP_BIT_POSITION   	12
#define 	STOP_MASK            	0b11
#define 	CLKEN 					11
#define 	CPOL					10
#define 	CPHA					9
#define 	LBCL					8
#define 	LBDIE					6
#define 	LBDL					5
#define 	ADD_BIT_POSITION    	0
#define 	ADD_MASK            	0b1111

// bits of register USART_CR3
#define 	ONEBIT 		11
#define 	CTSIE 		10
#define 	CTSE 		9
#define 	RTSE		8
#define 	DMAT		7
#define 	DMAR		6
#define 	SCEN		5
#define 	NACK		4
#define 	HDSEL		3

#endif /* MC_USART_PRIVATE_H_ */