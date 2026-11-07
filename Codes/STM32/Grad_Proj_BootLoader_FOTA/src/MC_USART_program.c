#include  "BIT_MATH.h"
#include  "STD_TYPES.h"

#include  "MC_USART_config.h"
#include  "MC_USART_interface.h"
#include  "MC_USART_private.h"

void USART_voidInit(USART_ConfigType* Copy_pstConfig)
{
	volatile USART_type* Local_pstUSART;

	switch(Copy_pstConfig -> INDEX)
	{
		case USART6_INDEX:
			Local_pstUSART = USART6;
			break;
		case USART1_INDEX:
			Local_pstUSART = USART1;
			break;
		default:
			return;
	}

	if (Copy_pstConfig-> Word_Length == NO_9_BIT )
	{
		SET_BIT(Local_pstUSART -> USART_CR1 , M );
	}
	else if (Copy_pstConfig-> Word_Length == NO_8_BIT )
	{
		CLR_BIT(Local_pstUSART -> USART_CR1 , M );
	}

	if (Copy_pstConfig-> Parity == NO_PARITY )
	{
		CLR_BIT(Local_pstUSART -> USART_CR1 , PCE );
	}
	else if (Copy_pstConfig-> Parity == EVEN )
	{
		SET_BIT(Local_pstUSART -> USART_CR1 , PCE );
		CLR_BIT(Local_pstUSART -> USART_CR1 , PS );
	}
	else if (Copy_pstConfig-> Parity == ODD )
	{
		SET_BIT(Local_pstUSART -> USART_CR1 , PCE );
		SET_BIT(Local_pstUSART -> USART_CR1 , PS );
	}

	if (Copy_pstConfig-> Stop_Bits == NO_1_BIT )
	{
		Local_pstUSART -> USART_CR2 &= ~(STOP_MASK << STOP_BIT_POSITION);
		Local_pstUSART -> USART_CR2 |=  (0b00      << STOP_BIT_POSITION);
	}
	else if (Copy_pstConfig-> Stop_Bits == NO_0_5_BIT)
	{
		Local_pstUSART -> USART_CR2 &= ~(STOP_MASK << STOP_BIT_POSITION);
		Local_pstUSART -> USART_CR2 |=  (0b01      << STOP_BIT_POSITION);
	}
	else if (Copy_pstConfig-> Stop_Bits == NO_2_BIT )
	{
		Local_pstUSART -> USART_CR2 &= ~(STOP_MASK << STOP_BIT_POSITION);
		Local_pstUSART -> USART_CR2 |=  (0b10      << STOP_BIT_POSITION);
	}
	else if (Copy_pstConfig-> Stop_Bits == NO_1_5_BIT )
	{
		Local_pstUSART -> USART_CR2 &= ~(STOP_MASK << STOP_BIT_POSITION);
		Local_pstUSART -> USART_CR2 |=  (0b11      << STOP_BIT_POSITION);
	}

	if (Copy_pstConfig-> Mode == TX_RX )
	{
		SET_BIT(Local_pstUSART -> USART_CR1 , RE );
		SET_BIT(Local_pstUSART -> USART_CR1 , TE );
	}
	else if (Copy_pstConfig-> Mode == RX_ONLY )
	{
		SET_BIT(Local_pstUSART -> USART_CR1 , RE );
		CLR_BIT(Local_pstUSART -> USART_CR1 , TE );
	}
	else if (Copy_pstConfig-> Mode == TX_ONLY )
	{
		CLR_BIT(Local_pstUSART -> USART_CR1 , RE );
		SET_BIT(Local_pstUSART -> USART_CR1 , TE );
	}

	u32 Local_u32Temp     = USART_APB2_CLOCK_HZ / Copy_pstConfig->BaudRate;
	u32 Local_u32Mantissa = Local_u32Temp / 16;
	u32 Local_u32Fraction = Local_u32Temp % 16;

	Local_pstUSART->USART_BRR &= ~(MANTISSA_MASK << MANTISSA_BIT_POSITION);
	Local_pstUSART->USART_BRR |=  (Local_u32Mantissa << MANTISSA_BIT_POSITION);

	Local_pstUSART->USART_BRR &= ~(FRACTION_MASK << FRACTION_BIT_POSITION);
	Local_pstUSART->USART_BRR |=  (Local_u32Fraction << FRACTION_BIT_POSITION);

	SET_BIT(Local_pstUSART -> USART_CR1 , UE );
}

void USART_voidSendChar(USART_INDEX Copy_enuInstance, u8 Copy_u8Char)
{
	volatile USART_type* Local_pstUSART;

	switch(Copy_enuInstance)
	{
		case USART6_INDEX:
			Local_pstUSART = USART6;
			break;
		case USART1_INDEX:
			Local_pstUSART = USART1;
			break;
		default:
			return;
	}

	while (GET_BIT(Local_pstUSART->USART_SR, TXE) == 0);

	Local_pstUSART->USART_DR = Copy_u8Char;
}

void USART_voidSendStr(USART_INDEX Copy_enuInstance, u8* Copy_u8Str)
{
	u8 Local_u8Index = 0;

	while (Copy_u8Str[Local_u8Index] != '\0')
	{
		USART_voidSendChar(Copy_enuInstance, Copy_u8Str[Local_u8Index]);   
		Local_u8Index++;
	}
}

u8 USART_u8ReceiveChar(USART_INDEX Copy_enuInstance)
{
	volatile USART_type* Local_pstUSART;
	u8 Local_u8ReceivedChar;

	switch(Copy_enuInstance)
	{
		case USART6_INDEX:
			Local_pstUSART = USART6;
			break;
		case USART1_INDEX:
			Local_pstUSART = USART1;
			break;
		default:
			return 0;
	}

	while (GET_BIT(Local_pstUSART->USART_SR, RXNE) == 0);

	Local_u8ReceivedChar = Local_pstUSART->USART_DR;

	return Local_u8ReceivedChar;
}

void USART_voidReceiveStr(USART_INDEX Copy_enuInstance, u8* Copy_pu8Buffer)
{
	u8 Local_u8Index = 0;
	u8 Local_u8ReceivedChar;

	do
	{
		Local_u8ReceivedChar = USART_u8ReceiveChar(Copy_enuInstance);   

		Copy_pu8Buffer[Local_u8Index] = Local_u8ReceivedChar;

		Local_u8Index++;

	} while (Local_u8ReceivedChar != '\r');

	Copy_pu8Buffer[Local_u8Index - 1] = '\0';
}

//**********************************DMA*******************************************************

void USART_voidEnableDMAT(USART_INDEX Copy_enuInstance)
{
	volatile USART_type* Local_pstUSART;

	switch(Copy_enuInstance)
	{
	case USART1_INDEX:
		Local_pstUSART = USART1;
		break;
	case USART6_INDEX:
		Local_pstUSART = USART6;
		break;
	default:
		return; /* invalid instance, do nothing */
	}

	SET_BIT(Local_pstUSART-> USART_CR3 , DMAT);
}

u32 USART_u32GetDRAddress(USART_INDEX Copy_enuInstance)
{
	volatile USART_type* Local_pstUSART;

	switch(Copy_enuInstance)
	{
	case USART1_INDEX:
		Local_pstUSART = USART1;
		break;
	case USART6_INDEX:
		Local_pstUSART = USART6;
		break;
	default:
		return 0; /* invalid instance */
	}

	return (u32)&(Local_pstUSART->USART_DR);
}

void USART_voidEnableTCInterrupt(USART_INDEX Copy_enuInstance)
{
	volatile USART_type* Local_pstUSART;

	switch(Copy_enuInstance)
	{
	case USART1_INDEX:
		Local_pstUSART = USART1;
		break;
	case USART6_INDEX:
		Local_pstUSART = USART6;
		break;
	default:
		return; /* invalid instance, do nothing */
	}

	SET_BIT(Local_pstUSART->USART_CR1,TCIE );
}

static void (*Global_apfCallback[2])(void) = {NULL};

void USART_voidSetTCCallback(USART_INDEX Copy_enuInstance, void (*Copy_pvCallbackFunc)(void))
{
	if((Copy_enuInstance == USART1_INDEX) || (Copy_enuInstance == USART6_INDEX))
	{
		Global_apfCallback[Copy_enuInstance] = Copy_pvCallbackFunc;
	}
	/* else: invalid instance, silently ignored */
}

void USART1_IRQHandler(void)
{
	if(GET_BIT(USART1->USART_SR, TC) && GET_BIT(USART1->USART_CR1, TCIE))
	{
		CLR_BIT(USART1->USART_SR, TC); /* clear by writing 0 directly to TC */

		if(Global_apfCallback[USART1_INDEX] != NULL)
		{
			Global_apfCallback[USART1_INDEX]();
		}
	}
}

void USART6_IRQHandler(void)
{
	if(GET_BIT(USART6->USART_SR, TC) && GET_BIT(USART6->USART_CR1, TCIE))
	{
		CLR_BIT(USART6->USART_SR, TC); /* clear by writing 0 directly to TC */

		if(Global_apfCallback[USART6_INDEX] != NULL)
		{
			Global_apfCallback[USART6_INDEX]();
		}
	}
}
