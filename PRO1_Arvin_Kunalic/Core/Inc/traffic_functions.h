/**************************************************************************//**
 * @file     traffic_functions.h
 * @brief    Header for traffic_functions.c file
 * 
 * @details  This file declares variables and functions used for managing the 
 *           traffic light control system.
 * 
 *           The functions declared in this header are designed to simplify 
 *           state management and hardware control in the traffic light project.
 * 
 ******************************************************************************
 * @author   Arvin Kunalic
 * @version  1.0
 * @date     20-December-2024
 * @note     Something worth noting
 *****************************************************************************/

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef TRAFFIC_FUNCTIONS_H
#define TRAFFIC_FUNCTIONS_H

/* Includes -----------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "595_shiftreg.h"
#include "ssd1306_config.h"
#include "fonts.h"
#include <stm32l476xx.h>
#include "clock.h"

/* Exported variables -------------------------------------------------------*/

/* Active car indicators, 0 = car not active, 1 = active car */
extern volatile bool car1_active;
extern volatile bool car2_active;
extern volatile bool car3_active;
extern volatile bool car4_active;

/* Exported functions -------------------------------------------------------*/

void init_program(void);
void stop_and_resetTimer(TIM_HandleTypeDef *htim_x);
bool no_active_cars(void);
bool active_cars_at(uint8_t intersection);

#endif