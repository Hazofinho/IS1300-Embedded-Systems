/**************************************************************************//**
 * @file     traffic_functions.c
 * @brief    Helper functions for traffic light control and state management.
 * 
 * @details  This file contains utility functions and global variables used to 
 *           manage the traffic light system. Key functionalities include:
 *           - System initialization: Setting up the OLED display, shift registers,
 *             and timers to their initial states.
 *           - Traffic state monitoring: Functions to check the activity status 
 *             of cars and intersections.
 *           - Timer management: Utility to stop and reset timers, reducing code redundancy.
 * 
 ******************************************************************************
 * @author   Arvin Kunalic
 * @version  1.0
 * @date     20-December-2024
 *****************************************************************************/

/* Includes -----------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "traffic_functions.h"
#include "595_shiftreg.h"
#include "ssd1306_config.h"
#include "fonts.h"
#include <stm32l476xx.h>
#include "clock.h"

/* Variables ----------------------------------------------------------------*/
volatile bool car1_active = 0;
volatile bool car2_active = 0;
volatile bool car3_active = 0;
volatile bool car4_active = 0;

/**************************************************************************//**
 * @brief    Initializes the entire traffic light program
 * @details  The function initializes the OLED screen, shift registers start-state,
 *           timers, and displays the cars and pedestrian states.
 * @version  1.0
 * @param    None
 * @return   None
 * @see      595_shiftreg.c/.h, ssd1306_config.c/.h and stm32l4xx_it.c
 *****************************************************************************/
void init_program(void) {
  /* init screen */
  init_OLED();
  clear_screen();
  /* init shift registers and it's start-state */
  reset_595register();
  update_shiftreg_buffer(init_state);
  buffer_to_SPI();

  /* Timer init */
  __HAL_TIM_SetCounter(&htim5, 0); // Reset counter
  __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_UPDATE); // Clear interrupt flag

  /* Display at start */
  draw_string(0, 0, "No pedestrian");
  draw_string(0, 8, "       is waiting..");
  draw_string(0, 31, "Car1 inactive");
  draw_string(0, 39, "Car2 inactive");
  draw_string(0, 47, "Car3 inactive");
  draw_string(0, 55, "Car4 inactive");
}

/**************************************************************************//**
 * @brief    Stops and resets a timers counter register (ARR)
 * @details  This function is just used in order to reduce redundancy in coding,
 *           providing a 2 in 1 combo.
 * @version  1.0
 * @param    TIM_HandleTypeDef *htim_x, specifies the timer to reset.
 * @return   None
 * @note     This function only works for Polling Timers and not Interrupt-Driven Timers!
 *****************************************************************************/
void stop_and_resetTimer(TIM_HandleTypeDef *htim_x) {
  HAL_TIM_Base_Stop(htim_x);
  __HAL_TIM_SetCounter(htim_x, 0);
}

/**************************************************************************//**
 * @brief    Checks if there are no active cars in traffic.
 * @details  Polls the global variables: carX_active 1-4, if they are all
 *           equal to zero (false) the function returns true. Indicating no single
 *           active car.
 * @version  1.0
 * @param    None
 * @return   boolean 
 *****************************************************************************/
bool no_active_cars(void) {
  if (!car1_active && !car2_active && !car3_active && !car4_active) {
    return 1;
  } else {
    return 0;
  }
}

/**************************************************************************//**
 * @brief    Checks if there are active cars at one of the Intersections.
 * @details  Polls the global variables: carX_active 1-4, if the cars at
 *           the intersection are active, it returns true.
 * @version  1.0
 * @param    uint8_t intersection, the intersection identifier (1 or 2).
 * @return   boolean
 * @note     This function will not work properly if an identifier other than
 *           1 or 2 is entered .
 *****************************************************************************/
bool active_cars_at(uint8_t intersection) {
  bool status = 0;
  if (intersection == 1) {
    (car1_active || car3_active) ? (status = 1) : (status = 0);
    return status;
  } else if (intersection == 2) {
    (car2_active || car4_active) ? (status = 1) : (status = 0);
    return status;
  } 
  return -1;
}