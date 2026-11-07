/*
 * HA_TFT_interface.h
 *
 *  Created on: 22 Sep 2026
 *      Author: Sief
 */

#ifndef HA_TFT_INTERFACE_H_
#define HA_TFT_INTERFACE_H_

void TFT_voidInit(void);

void TFT_voidFillScreen(u16 Copy_u16Color);

void TFT_voidDisplay(const u16 *Copy_pu16Image);

#endif /* HA_TFT_INTERFACE_H_ */
