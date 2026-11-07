/*
 * MC_STK_private.h
 */
#ifndef MC_STK_PRIVATE_H_
#define MC_STK_PRIVATE_H_

#define STK_BASE_ADDRESS 	(0xE000E010UL)

typedef struct
{			// the offset
	u32 STK_CTRL ; //0x00
	u32 STK_LOAD ; //0x04
	u32 STK_VAL  ; //0x08
	u32 STK_CALIB; //0x0C
}STK_TYPE;

// Pointer macro
#define SYSTICK   ((volatile STK_TYPE *) STK_BASE_ADDRESS)

// for STK_CTRL register
#define ENABLE					0
#define TICK_INT				1
#define CLK_SOURCE				2
#define COUN_FLAG				16

// for clock bus source
#define 	AHB				0
#define 	AHB_DIV_BY_8	1

static u32 Global_u32IntervalTicks;

#endif /* MC_STK_PRIVATE_H_ */
