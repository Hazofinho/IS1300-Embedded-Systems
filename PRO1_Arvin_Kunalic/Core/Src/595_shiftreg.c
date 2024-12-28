/**************************************************************************//**
* @file     595_shiftreg.c
* @brief    Implementation of traffic light and pedestrian control using
*           three 8-bit 74HC595D shift registers and GPIO.
*
* @details  This file provides functions for controlling traffic lights and
*           pedestrian lights using SPI communication. It includes utilities
*           for updating shift registers, toggling pins, and managing traffic
*           and pedestrian flow.
*******************************************************************************
* @author   Arvin Kunalic
* @version  2.0
* @date     20-December-2024
* @note     The communication protocol is SPI.
******************************************************************************/

/* Includes -----------------------------------------------------------------*/
#include "595_shiftreg.h"
#include "ssd1306_config.h"
#include "timer_config.h"
#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "tim.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l476xx.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Variables ----------------------------------------------------------------*/
uint8_t shiftreg_buffer[SHIFTREG_BUFFER_SIZE] = {0x00, 0x00, 0x00};
const uint32_t init_state = ((TL2_Green | TL4_Green) | PL2_Red) | ((TL1_Red | TL3_Red) | PL1_Green);

/* Initial start values per requirements R1.1 and R2.8 */
volatile bool crosswalk1_green = 1;
volatile bool crosswalk1_red = 0;

volatile bool crosswalk2_green = 0;
volatile bool crosswalk2_red = 1;

volatile bool PL1_SW_HIT = 0;
volatile bool PL2_SW_HIT = 0;

volatile bool intersection1_green = 0;
volatile bool intersection1_red = 1;

volatile bool intersection2_green = 1;
volatile bool intersection2_red = 0;

/* Functions ---------------------------------------------------------------*/

/**************************************************************************//**
 * @brief   Resets the 74HC595D shift registers.
 * @details Clears all outputs and resets the control lines to prepare the
 *          system for new data.
 * @version 1.0
 * @param   None
 * @return  None
 *****************************************************************************/
void reset_595register(void) {
    HAL_GPIO_WritePin(_595_Reset_GPIO_Port, _595_Reset_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(_595_STCP_GPIO_Port, _595_STCP_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(_595_STCP_GPIO_Port, _595_STCP_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(_595_Reset_GPIO_Port, _595_Reset_Pin, GPIO_PIN_SET);
}

/**************************************************************************//**
 * @brief   Transmits the contents of the `shiftreg_buffer` to the shift registers.
 * @details Sends the buffer data using SPI and latches the data to update
 *          the physical outputs of the 74HC595D shift registers.
 * @version 1.0
 * @param   None
 * @return  None
 * @note    Make sure 'shiftreg_buffer` is updated before calling this function.
 *****************************************************************************/
void buffer_to_SPI(void) {
    HAL_GPIO_WritePin(_595_STCP_GPIO_Port, _595_STCP_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi3, shiftreg_buffer, SHIFTREG_BUFFER_SIZE, HAL_MAX_DELAY);
    HAL_Delay(10);
    HAL_GPIO_WritePin(_595_STCP_GPIO_Port, _595_STCP_Pin, GPIO_PIN_SET);
}

/**************************************************************************//**
 * @brief   Updates the shift register buffer with a 24-bit value.
 * @details Splits the 24-bit value into three 8-bit chunks, corresponding to
 *          the three shift registers in the chain, and stores them in
 *          `shiftreg_buffer`.
 * @version 2.0
 * @param   uint32_t value, A 32-bit value representing the desired output
 *                          state for the shift registers.
 * @return  None
 * @see     buffer_to_SPI
 *****************************************************************************/
void update_shiftreg_buffer(uint32_t value) {
    uint8_t u1_val = (value & 0xFF0000) >> 16;
    uint8_t u2_val = (value & 0x00FF00) >> 8;
    uint8_t u3_val = value & 0x0000FF;

    shiftreg_buffer[U3] = u3_val;
    shiftreg_buffer[U2] = u2_val;
    shiftreg_buffer[U1] = u1_val;
}

/**************************************************************************//**
 * @brief   Sets a specific pin or multiple pins in the shift register to HIGH.
 * @details Updates the internal shift register buffer to set the specified
 *          pin without affecting the state of other pins, the buffer is then
 *          sent to the registers using SPI.
 * @version 1.2
 * @param   uint32_t pins, The bitmask of the pin/pins to set.
 * @return  None
 * @see     clear_pin, update_shiftreg_buffer, buffer_to_SPI
 *****************************************************************************/
void set_pin(uint32_t pins) {
    uint32_t bitmask = (shiftreg_buffer[U1] << 16)
                     | (shiftreg_buffer[U2] << 8) 
                     | (shiftreg_buffer[U3]);

    uint32_t data = bitmask | pins;

    update_shiftreg_buffer(data);
    buffer_to_SPI();
}

/**************************************************************************//**
 * @brief   Sets a specific pin or multiple pins in the shift register to LOW.
 * @details Updates the internal shift register buffer to clear the specified
 *          pin without affecting the state of other pins, the buffer is then
 *          sent to the registers using SPI.
 * @version 1.2
 * @param   uint32_t pins, The bitmask of the pin/pins to clear.
 * @return  None
 * @see     set_pin, update_shiftreg_buffer, buffer_to_SPI
 *****************************************************************************/
void clear_pin(uint32_t pins) {
    uint32_t bitmask = (shiftreg_buffer[U1] << 16) 
                     | (shiftreg_buffer[U2] << 8) 
                     | (shiftreg_buffer[U3]);

    uint32_t data = bitmask & ~pins;

    update_shiftreg_buffer(data);
    buffer_to_SPI();
}

/**************************************************************************//**
 * @brief   Flashes the pedestrian blue light to indicate a waiting pedestrian.
 * @details Activates the blue light of the specified crosswalk in a
 *          blinking pattern for a duration defined by `Pedestrian_Delay`.
 * @version 2.0
 * @param   uint8_t crosswalk, The crosswalk identifier (1 or 2).
 * @return  None
 * @note    This function only works properly if the identifier is 1 or 2.
 *          If an invalid crosswalk is specified, the function will only toggle
 *          the Pedestrian2 lights.
 * @see     set_pin, clear_pin, HAL_TIM_PeriodElapsedCallback (ISR for timer 3)
 *****************************************************************************/
void toggle_pedestrian(uint8_t crosswalk) {
    static uint32_t pin;
    static bool toggle = 0;

    (crosswalk == 1) ? (pin = PL1_Blue) : (pin = PL2_Blue);

    (toggle) ? (clear_pin(pin)) : (set_pin(pin));

    toggle = !toggle;
}

/**************************************************************************//**
 * @brief   Activates the green pedestrian light and disables red light.
 * @details Changes the state of the pedestrian lights from green to red.
 * @version 2.0
 * @param   uint8_t crosswalk, The crosswalk identifier (1 or 2).
 * @return  None
 * @note    This function only works properly if the identifier is 1 or 2.
 *          If an invalid crosswalk is specified, the function simply returns
 *          without affect.
 * @see     toggle_pedestrian, stop_pedestrian, set_pin, clear_pin
 *****************************************************************************/
void go_pedestrian(uint8_t crosswalk) {
    static uint32_t pin_red, pin_green;

    if (crosswalk == 1) {
        pin_red = PL1_Red;
        pin_green = PL1_Green;
        crosswalk1_green = 1;
        crosswalk1_red = 0;
        draw_string(0, 0, "Pedestrians can    ");
        draw_string(0, 8, "     cross lane 1!");
    } else if (crosswalk == 2) {
        pin_red = PL2_Red;
        pin_green = PL2_Green;
        crosswalk2_green = 1;
        crosswalk2_red = 0;
        draw_string(0, 0, "Pedestrians can    ");
        draw_string(0, 8, "     cross lane 2!");
    } else {
        return; // Invalid intersection
    }

    clear_pin(pin_red);
    set_pin(pin_green);

    /* 
    *   If 'go_pedestrian' is called after a pedestrian button-press, make
    *   sure 'walking_Delay' time is met.
    */
    if (PL1_SW_HIT || PL2_SW_HIT) {

    /* Start pedestrian_Delay timer making sure R1.3 is met */
    HAL_TIM_Base_Start_IT(&htim5); 
    
    }
}

/**************************************************************************//**
 * @brief   Activates the red pedestrian light and disables the green light.
 * @details Changes the state of the pedestrian lights from green to red.
 * @version 1.2
 * @param   uint8_t crosswalk, The crosswalk identifier (1 or 2).
 * @return  None
 * @note    This function only works properly if the identifier is 1 or 2.
 *          If an invalid crosswalk is specified, the function simply returns
 *          without affect.
 * @see     toggle_pedestrian, go_pedestrian, set_pin, clear_pin
*****************************************************************************/
void stop_pedestrian(uint8_t crosswalk) {
    static uint32_t pin_green, pin_red;

    if (crosswalk == 1) {
        pin_green = PL1_Green;
        pin_red = PL1_Red;
        crosswalk1_green = 0;
        crosswalk1_red = 1;
        draw_string(0, 0, "Pedestrians cannot ");
        draw_string(0, 8, "     cross lane 1..");
    } else if (crosswalk == 2) {
        pin_green = PL2_Green;
        pin_red = PL2_Red;
        crosswalk2_green = 0;
        crosswalk2_red = 1;
        draw_string(0, 0, "Pedestrians cannot ");
        draw_string(0, 8, "     cross lane 2..");
    } else {
        return; // Invalid intersection
    }

    clear_pin(pin_green);
    set_pin(pin_red);
}

/**************************************************************************//**
 * @brief   Transitions the traffic lights of an inactive intersection to green.
 * @details This function transitions the intersection lights with staging,
 *          emulating realistic traffic light behavior. The full transition
 *          takes 5 seconds, with the yellow light active for 'orange_Delay' ticks
 *          (1 tick = 0.5 ms).  
 * @version 3.0
 * @param   uint8_t intersection, The intersection identifier (1 or 2).
 * @return  None
 * @note    - This function only works properly if the identifier is 1 or 2.
 *            If an invalid crosswalk is specified, the function simply returns
 *            without affect.
 * 
 *          To function correctly:
 *  
 *            - The function needs to be called repeatedly.
 * 
 *            - A 5s timer (TIM4) has to be started ONCE before calling this function.    
 * @see     stop_intersection, set_pin, clear_pin
 *****************************************************************************/
void go_intersection(uint8_t intersection) {
    static uint32_t greens, yellows, reds;
    static bool stage = 0;

    if (stage == 0) {
        if (intersection == 1) {
            greens = (TL1_Green | TL3_Green);
            yellows = (TL1_Yellow | TL3_Yellow);
            reds = (TL1_Red | TL3_Red);
        } else if (intersection == 2) {
            greens = (TL2_Green | TL4_Green);
            yellows = (TL2_Yellow | TL4_Yellow);
            reds = (TL2_Red | TL4_Red);
        } else {
            return; // Invalid intersection
        }

        if (__HAL_TIM_GetCounter(&htim4) >= TIMER_2s) { // Turn red light off after 2s
            HAL_TIM_Base_Stop(&htim4);
            __HAL_TIM_SetCounter(&htim4, 0);
            clear_pin(reds);
            set_pin(yellows);
            HAL_TIM_Base_Start(&htim4);
            (intersection == 1) ? (intersection1_red = 0) : (intersection2_red = 0);
            stage = 1;
            return;
        } else {
            return;
        }
    }

    if (stage == 1) {
        if (__HAL_TIM_GetCounter(&htim4) >= orange_Delay) {
            HAL_TIM_Base_Stop(&htim4);
            __HAL_TIM_SetCounter(&htim4, 0);
            clear_pin(yellows);
            set_pin(greens);
            (intersection == 1) ? (intersection1_green = 1) : (intersection2_green = 1);
            stage = 0;
            return;
        } else {
            return;
        }
    }
}

/**************************************************************************//**
 * @brief   Transitions the traffic lights of an active intersection to red.
 * @details This function transitions the intersection lights with staging,
 *          emulating realistic traffic light behavior. The full transition
 *          takes 5 seconds, with the yellow light active for 'orange_Delay' ticks
 *          (1 tick = 0.5 ms).  
 * @version 3.0
 * @param   uint8_t intersection, The intersection identifier (1 or 2).
 * @return  None
 * @note    - This function only works properly if the identifier is 1 or 2.
 *            If an invalid crosswalk is specified, the function simply returns
 *            without affect.
 * 
 *          To function correctly:
 *  
 *            - The function needs to be called repeatedly.
 * 
 *            - A 5s timer (TIM4) has to be started ONCE before calling this function.    
 * @see     go_intersection, set_pin, clear_pin
 *****************************************************************************/
void stop_intersection(uint8_t intersection) {
    static uint32_t greens, yellows, reds;
    static bool stage = 0;

    if (stage == 0) {
        if (intersection == 1) {
            greens = (TL1_Green | TL3_Green);
            yellows = (TL1_Yellow | TL3_Yellow);
            reds = (TL1_Red | TL3_Red);
        } else if (intersection == 2) {
            greens = (TL2_Green | TL4_Green);
            yellows = (TL2_Yellow | TL4_Yellow);
            reds = (TL2_Red | TL4_Red);
        } else {
            return; // Invalid intersection
        }
        if (__HAL_TIM_GetCounter(&htim4) >= (TIMER_2s)) { // Turn green light off after 2s
            HAL_TIM_Base_Stop(&htim4);
            __HAL_TIM_SetCounter(&htim4, 0);
            clear_pin(greens);
            set_pin(yellows);
            HAL_TIM_Base_Start(&htim4);
            (intersection == 1) ? (intersection1_green = 0) : (intersection2_green = 0);
            stage = 1;
            return;
        } else {
            return;
        }
    }

    if (stage == 1) {
        if (__HAL_TIM_GetCounter(&htim4) >= orange_Delay) { 
            HAL_TIM_Base_Stop(&htim4);
            __HAL_TIM_SetCounter(&htim4, 0);
            clear_pin(yellows);
            set_pin(reds);
            HAL_TIM_Base_Start(&htim4);
            (intersection == 1) ? (intersection1_red = 1) : (intersection2_red = 1);
            stage = 0;
            return;
        } else {
            return;
        }
    }
}
