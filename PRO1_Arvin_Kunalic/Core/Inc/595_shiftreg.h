/**************************************************************************//**
 * @file     595_shiftreg.h
 * @brief    Header file for 595_shiftreg.c
 *
 * @details  This file contains the definitions and function prototypes
 *           required for controlling a series of 74HC595D shift registers
 *           using SPI communication. The implementation provides functionality
 *           to manipulate traffic lights, pedestrian lights, and other
 *           peripherals connected through the shift registers. Features include:
 *           - Bitwise control of traffic and pedestrian light states.
 *           - Timed transitions and delays for traffic light sequencing.
 *           - Functions for pedestrian light flashing and state toggling.
 *           - Utilities SPI for updating and transmitting the shift register buffer.
 * 
 ******************************************************************************
 * @author   Arvin Kunalic
 * @version  2.0
 * @date     20-December-2024
 * @note     This header should be included alongside the `shiftreg.c` source file.
 *           Confirm proper hardware connections and configurations in your project.
 *******************************************************************************/

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef SHIFTREG_H
#define SHIFTREG_H

/* Includes -----------------------------------------------------------------*/
#include "spi.h"
#include "usart.h"
#include "stm32l4xx_hal.h"
#include <stdbool.h>

/* Exported constants -------------------------------------------------------*/

/* Buffer Size */
#define SHIFTREG_BUFFER_SIZE 3

/* Register Indexes */
#define U1                  2
#define U2                  1
#define U3                  0

/* --- Traffic and Pedestrian Light Masks --- 
* TL = "Traffic Light", PL = "Pedestrain Light" */

/* U1 (Street Direction 1) */
#define TL1_Red             0x010000    
#define TL1_Yellow          0x020000 
#define TL1_Green           0x040000  
#define PL1_Red             0x080000
#define PL1_Green           0x100000  
#define PL1_Blue            0x200000 

/* U2 (Street Direction 2 and 4) */
#define TL2_Red             0x0100   
#define TL2_Yellow          0x0200   
#define TL2_Green           0x0400   
#define PL2_Red             0x0800
#define PL2_Green           0x1000
#define PL2_Blue            0x2000

/* U3 (Street Direction 2 and 3) */
/* Direction 3*/
#define TL3_Red             0x01   
#define TL3_Yellow          0x02   
#define TL3_Green           0x04   
/* Direction 2*/
#define TL4_Red             0x08  
#define TL4_Yellow          0x10  
#define TL4_Green           0x20  
/*---------------------------------------------------------------------------*/


/* Exported variables -------------------------------------------------------*/
extern uint8_t shiftreg_buffer[SHIFTREG_BUFFER_SIZE];
extern const uint32_t init_state;

extern volatile bool crosswalk1_green;
extern volatile bool crosswalk1_red;
extern volatile bool crosswalk2_green;
extern volatile bool crosswalk2_red;

extern volatile bool PL1_SW_HIT;
extern volatile bool PL2_SW_HIT;

extern volatile bool intersection1_green;
extern volatile bool intersection1_red;
extern volatile bool intersection2_green;
extern volatile bool intersection2_red;

/* Exported functions -------------------------------------------------------*/
void reset_595register(void);
void buffer_to_SPI(void);
void update_shiftreg_buffer(uint32_t value);

void set_pin(uint32_t pins);
void clear_pin(uint32_t pins);

void toggle_pedestrian(uint8_t crosswalk);
void go_pedestrian(uint8_t crosswalk);
void stop_pedestrian(uint8_t crosswalk);

void go_intersection(uint8_t intersection);
void stop_intersection(uint8_t intersection);

#endif
