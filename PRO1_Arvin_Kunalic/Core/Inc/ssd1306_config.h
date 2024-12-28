/**************************************************************************//**
 * @file     ssd1306_config.h
 * @brief    Header file for ssd1306_config.c
 *
 * @details  This header file declares the functions and variables 
 *           required to interface with the SSD1306 OLED display. It provides:
 *           - Function prototypes for initializing and operating the display.
 *           - Macro definitions for display settings (e.g., screen dimensions).
 *           - Global framebuffer declaration for managing display updates.
 *           - Pin mappings for SPI and GPIO control specific to the SSD1306.
 *
 ******************************************************************************
 * @author   Arvin Kunalic
 * @version  1.0
 * @date     01-December-2024
 * @note     This header should be included in projects interfacing with 
 *           the SSD1306 OLED display. Ensure the accompanying source file 
 *           (ssd1306_config.c) is included in the build process.
 *
 *****************************************************************************/

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef SSD1306_CONFIGH
#define SSD1306_CONFIG_H

/* Includes -----------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"

/* Defines ------------------------------------------------------------------*/
/* Screen size (pixels)*/
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_BUFFER_SIZE OLED_WIDTH *OLED_HEIGHT / 8

/* Exported variables -------------------------------------------------------*/

/* 128x64 display, 1 byte = 8 vertical pixels */
extern uint8_t OLED_framebuffer[OLED_BUFFER_SIZE];

/* Exported functions -------------------------------------------------------*/
void reset_OLED(void);
void send_command_OLED(uint8_t command);
void send_data_OLED(uint8_t data);
void init_OLED(void);
void update_screen(void);
void clear_screen(void);
void draw_char(uint8_t x, uint8_t y, char c);
void draw_string(uint8_t x, uint8_t y, const char *str);

#endif