/******************************************************************************
 *  File P      : RCC_program.c
 *  Layer       : MCAL
 *  Module      : RCC (Reset and Clock Control)
 *  Description : Implementation of the RCC driver public API.
 ******************************************************************************/
// include libs
#include  "BIT_MATH.h"
#include  "STD_TYPES.h"

#include  "MC_RCC_config.h"
#include  "MC_RCC_interface.h"
#include  "MC_RCC_private.h"


void RCC_voidInit(void)
{
	#if SYSTEM_CLOCK == HSI
		// HSI on
		SET_BIT(RCC->RCC_CR,RCC_CR_HSION);

		//wait for stability
		while ((GET_BIT(RCC->RCC_CR,RCC_CR_HSIRDY)) ==0)
				{
				}
		// set config of HSI
		CLR_BIT(RCC->RCC_CFGR , 0);
		CLR_BIT(RCC->RCC_CFGR , 1);

	#elif SYSTEM_CLOCK == HSE
		//check define HSE type
		#if HSE_DEFINE_TYPE == MECHANICAL
			CLR_BIT(RCC->RCC_CR ,RCC_CR_HSEBYP);

		#elif HSE_DEFINE_TYPE == ELECTRICAL
			SET_BIT(RCC->RCC_CR ,RCC_CR_HSEBYP);
		#endif


		// HSE on
		SET_BIT(RCC->RCC_CR,RCC_CR_HSEON);

		//wait for stability
		while ((GET_BIT(RCC->RCC_CR,RCC_CR_HSERDY)) ==0)
				{
				}
		// set config of HSE
		SET_BIT(RCC->RCC_CFGR , 0);
		CLR_BIT(RCC->RCC_CFGR , 1);
///////////////////////////////////////////
#elif SYSTEM_CLOCK == PLL_HSI

		// HSI on
		SET_BIT(RCC->RCC_CR,RCC_CR_HSION);

		//wait for stability
		while ((GET_BIT(RCC->RCC_CR,RCC_CR_HSIRDY)) ==0)
				{
				}

		/* configure PLLM, PLLN, PLLP */
		RCC->RCC_PLLCFGR = (PLL_M << RCC_PLLCFGR_PLLM0) |
		                    (PLL_N << RCC_PLLCFGR_PLLN0) |
		                    (PLL_P << RCC_PLLCFGR_PLLP0);

		/* enable PLL */
		SET_BIT(RCC->RCC_CR, RCC_CR_PLLON);
		while ((GET_BIT(RCC->RCC_CR, RCC_CR_PLLRDY)) == 0)
		{
		}

		/*  select PLL as system clock -> SW = 10 */
		CLR_BIT(RCC->RCC_CFGR, RCC_CFGR_SW0);
		SET_BIT(RCC->RCC_CFGR, RCC_CFGR_SW1);

		/* wait until SWS confirms PLL is the active source -> SWS = 10 */
		while ((GET_BIT(RCC->RCC_CFGR, RCC_CFGR_SWS0)) != 0)
		{
		}
		while ((GET_BIT(RCC->RCC_CFGR, RCC_CFGR_SWS1)) == 0)
		{
		}

	#elif SYSTEM_CLOCK == PLL_HSE
		//check define HSE type
			#if HSE_DEFINE_TYPE == MECHANICAL
				CLR_BIT(RCC->RCC_CR ,RCC_CR_HSEBYP);

			#elif HSE_DEFINE_TYPE == ELECTRICAL
				SET_BIT(RCC->RCC_CR ,RCC_CR_HSEBYP);
			#endif


			// HSE on
			SET_BIT(RCC->RCC_CR,RCC_CR_HSEON);

			//wait for stability
			while ((GET_BIT(RCC->RCC_CR,RCC_CR_HSERDY)) ==0)
					{
					}

		/* configure PLLM, PLLN, PLLP */
		RCC->RCC_PLLCFGR = (PLL_M << RCC_PLLCFGR_PLLM0) |
		                    (PLL_N << RCC_PLLCFGR_PLLN0) |
		                    (PLL_P << RCC_PLLCFGR_PLLP0) |
		                    (1U    << RCC_PLLCFGR_PLLSRC);

		/* enable PLL */
		SET_BIT(RCC->RCC_CR, RCC_CR_PLLON);
		while ((GET_BIT(RCC->RCC_CR, RCC_CR_PLLRDY)) == 0)
		{
		}

		/* select PLL as system clock -> SW = 10 */
		CLR_BIT(RCC->RCC_CFGR, RCC_CFGR_SW0);
		SET_BIT(RCC->RCC_CFGR, RCC_CFGR_SW1);

		/* wait until SWS confirms PLL is active -> SWS = 10 */
		while ((GET_BIT(RCC->RCC_CFGR, RCC_CFGR_SWS0)) != 0)
		{
		}
		while ((GET_BIT(RCC->RCC_CFGR, RCC_CFGR_SWS1)) == 0)
		{
		}

	#endif

}

void RCC_voidEnblePeripheral(u8 copy_u8Bus, u8 copy_u8peripheral)
{
	switch (copy_u8Bus)
	{
	case AHB1:
		SET_BIT(RCC->RCC_AHB1ENR, copy_u8peripheral);
		break;

	case AHB2:
		SET_BIT(RCC->RCC_AHB2ENR, copy_u8peripheral);
		break;

	case APB1:
		SET_BIT(RCC->RCC_APB1ENR, copy_u8peripheral);
		break;

	case APB2:
		SET_BIT(RCC->RCC_APB2ENR, copy_u8peripheral);
		break;

	default:
		/* Invalid bus - do nothing */
		break;
	}
}
void RCC_voidDisablePeripheral(u8 copy_u8Bus, u8 copy_u8peripheral)
{
	switch (copy_u8Bus)
	{
	case AHB1:
		CLR_BIT(RCC->RCC_AHB1ENR, copy_u8peripheral);
		break;

	case AHB2:
		CLR_BIT(RCC->RCC_AHB2ENR, copy_u8peripheral);
		break;

	case APB1:
		CLR_BIT(RCC->RCC_APB1ENR, copy_u8peripheral);
		break;

	case APB2:
		CLR_BIT(RCC->RCC_APB2ENR, copy_u8peripheral);
		break;

	default:
		/* Invalid bus - do nothing */
		break;
	}
}
