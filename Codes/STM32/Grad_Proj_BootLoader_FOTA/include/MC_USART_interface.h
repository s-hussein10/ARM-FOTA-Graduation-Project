/*
 * MC_USART_interface.h
 *
 *  Created on: 31 Aug 2026
 *      Author: Sief
 */

#ifndef MC_USART_INTERFACE_H_
#define MC_USART_INTERFACE_H_


typedef enum
{
	USART6_INDEX ,
	USART1_INDEX
}USART_INDEX;

typedef enum
{
	EVEN ,
	ODD  ,
	NO_PARITY

}USART_Parity;

typedef enum
{
	NO_8_BIT ,
	NO_9_BIT

}USART_Word_Length;

typedef enum
{
	NO_1_BIT	,
	NO_0_5_BIT  ,
	NO_2_BIT	,
	NO_1_5_BIT

}USART_Stop_Bits;

typedef enum
{
	TX_ONLY,
	RX_ONLY,
	TX_RX
}USART_Mode;

typedef struct
{
	USART_INDEX     	INDEX 		;
	USART_Parity		Parity		;
	USART_Word_Length	Word_Length	;
	USART_Stop_Bits 	Stop_Bits	;
	u32					BaudRate	;
	USART_Mode			Mode		;
}USART_ConfigType;

void USART_voidInit(USART_ConfigType* Copy_pstConfig);

void USART_voidSendChar(USART_INDEX Copy_enuInstance, u8 Copy_u8Char);
void USART_voidSendStr(USART_INDEX Copy_enuInstance, u8* Copy_u8Str);

u8   USART_u8ReceiveChar(USART_INDEX Copy_enuInstance);
void USART_voidReceiveStr(USART_INDEX Copy_enuInstance, u8* Copy_pu8Buffer);


//DMA Configration

void USART_voidEnableDMAT(USART_INDEX Copy_enuInstance);
u32 USART_u32GetDRAddress(USART_INDEX Copy_enuInstance);
void USART_voidEnableTCInterrupt(USART_INDEX Copy_enuInstance);
void USART_voidSetTCCallback(USART_INDEX Copy_enuInstance, void (*Copy_pvCallbackFunc)(void));


#endif /* MC_USART_INTERFACE_H_ */
