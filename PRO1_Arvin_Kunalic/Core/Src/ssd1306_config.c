/**************************************************************************//**
 * @file     ssd1306_config.c
 * @brief    Implementation of SSD1306 OLED display control for the 
 *           NUCLEO-L476RG development board.
 *
 * @details  This file contains functions and configurations to control and 
 *           interface with the SSD1306 OLED display using the SPI protocol. 
 *           It provides initialization, data transmission, and utility 
 *           functions for rendering text, images, and graphical elements 
 *           on the display. Core features include:
 *           - Display initialization with configurable settings.
 *           - Framebuffer management for efficient screen updates.
 *           - Drawing characters and strings.
 *           - Screen clearing and scrolling utilities.
 *           - Hardware reset handling.
 *
 ******************************************************************************
 * @author   Arvin Kunalic
 * @version  1.0
 * @date     01-December-2024
 * @note     Make sure of proper hardware connections between the SSD1306 OLED 
 *           and the NUCLEO-L476RG for correct functionality. Refer to the 
 *           hardware schematic for pin mappings.
 *****************************************************************************/

/* Includes -----------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx_hal.h"
#include "stm32l476xx.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "ssd1306_config.h"
#include "fonts.h"
#include <string.h>

/* Variables ----------------------------------------------------------------*/
uint8_t OLED_framebuffer[OLED_BUFFER_SIZE] = {0};

/**************************************************************************//**
 * @brief   Resets the SSD1306 OLED display.
 *
 * @details This function resets the SSD1306 by pulling the RESET pin low for
 *          20ms, veriying the display is in a known state before configuration.
 *          Resetting the display clears any previous settings or RAM contents.
 *
 * @version 1.0
 * @param   None
 * @return  None
 *****************************************************************************/
void reset_OLED(void) {
    HAL_GPIO_WritePin(Disp_Reset_GPIO_Port, Disp_Reset_Pin, GPIO_PIN_RESET); // Reset OLED
    HAL_Delay(20);
    HAL_GPIO_WritePin(Disp_Reset_GPIO_Port, Disp_Reset_Pin, GPIO_PIN_SET); // Release reset
}

/**************************************************************************//**
 * @brief   Writes data to the command register of the display.
 *
 * @details This function writes a single command byte to the SSD1306 command
 *          register using SPI. The CS (chip select) and D/C (data/command) pins
 *          are toggled to differentiate between commands and data transfers.
 *
 * @version 1.0
 * @param   Explain the parameters ex: 'uint32_t value, A 24-bit value representing the desired output
 *                                                      state for the shift registers.'
 * @return  Return type, description of what the function returns default None
 * @see     send_data_OLED
 *****************************************************************************/
void send_command_OLED(uint8_t command) {
    HAL_GPIO_WritePin(Disp_CS_GPIO_Port, Disp_CS_Pin, GPIO_PIN_RESET);                 // Select OLED
    HAL_GPIO_WritePin(Disp_Data_Instr_GPIO_Port, Disp_Data_Instr_Pin, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit(&hspi2, &command, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(Disp_CS_GPIO_Port, Disp_CS_Pin, GPIO_PIN_SET); // Deselect OLED
}

/**************************************************************************//**
 * @brief   Writes data to the data register of the display.
 * @details A more in-depth explenation of how it does the thing.
 * @version 1.0
 * @param   Explain the parameters ex: 'uint32_t value, A 24-bit value representing the desired output
 *                                                      state for the shift registers.'
 * @return  Return type, description of what the function returns default None
 *****************************************************************************/
void send_data_OLED(uint8_t data) {
    HAL_GPIO_WritePin(Disp_CS_GPIO_Port, Disp_CS_Pin, GPIO_PIN_RESET);               // Select OLED
    HAL_GPIO_WritePin(Disp_Data_Instr_GPIO_Port, Disp_Data_Instr_Pin, GPIO_PIN_SET); // Data mode
    HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(Disp_CS_GPIO_Port, Disp_CS_Pin, GPIO_PIN_SET); // Deselect OLED
}

/**************************************************************************//**
 * @brief   Initializes the SSD1306 OLED display.
 *
 * @details This function configures the SSD1306 display by sending a sequence
 *          of initialization commands. The initialization sequence is based on
 *          the SSD1306 datasheet and sets the display properties.
 *
 * @version 1.0
 * @param   None
 * @return  None
 *****************************************************************************/
void init_OLED(void) {
    reset_OLED();

    /* Information provided by the datasheet */
    uint8_t init_sequence[] = {
        0xAE,       // Display off
        0xD5, 0x80, // Set clock divide ratio and oscillator frequency
        0xA8, 0x3F, // Set multiplex ratio (1/64)
        0xD3, 0x00, // Set display offset
        0x40,       // Set start line address
        0x8D, 0x14, // Enable charge pump
        0x20, 0x00, // Set memory addressing mode (horizontal)
        0xA1,       // Set segment re-map (A1 for horizontal flip)
        0xC8,       // Set COM output scan direction (C0: Normal C8: for vertical flip)
        0xDA, 0x12, // Set COM pins hardware configuration
        0x81, 0x7F, // Set contrast control
        0xD9, 0xF1, // Set pre-charge period
        0xDB, 0x40, // Set VCOMH deselect level
        0xA4,       // Entire display ON (resume to RAM content)
        0xA6,       // Normal display mode (A7 for inverse)
        0xAF        // Display ON
    };

    for (uint8_t i = 0; i < sizeof(init_sequence); i++) {
        send_command_OLED(init_sequence[i]);
    }
}

/**************************************************************************//**
 * @brief    Updates the OLED display.
 *
 * @details  This function updates the entire OLED display using the data
 *           stored in the framebuffer. The display is updated page by page,
 *           where each page represents 8 rows of vertical pixels.
 *
 *           For each page:
 *             1. The page start adress is set (Page 0: '0xB0' to Page 7: '0xB7').
 *             2. The lower and higher column start adresses are set,
 *                '0x00' - '0x0F' starting at column 0.
 *             3. 128 bytes of pixel data are written from the frambuffer to the OLED
 *
 *           Each byte in the framebuffer represents 8 vertical pixels in a column.
 *
 * @version  1.0
 * @param    None
 * @return   None
 * @see      send_command_OLED, send_data_OLED
 *****************************************************************************/
void update_screen(void) {
    for (uint8_t page = 0; page < 8; page++) {
        /* Set page start adress */
        send_command_OLED(0xB0 + page);
        /* Set column adresses, start at column 0 */
        send_command_OLED(0x00); // Set lower column start adress
        send_command_OLED(0x10); // Set higher column start adress

        /* Write 128 bytes from current page*/
        for (uint8_t col = 0; col < 128; col++) {
            send_data_OLED(OLED_framebuffer[page * OLED_WIDTH + col]);
        }
    }
}

/**************************************************************************//**
 * @brief    Clears the display.
 * @details  This function sets all pixels of the OLED framebuffer to 0
 *           and sends that to the display, turning off every pixel.
 * @version  1.0
 * @param    None
 * @return   None
 *****************************************************************************/
void clear_screen(void) {
    /* Set all bytes in the framebuffer to 0*/
    memset(OLED_framebuffer, 0x00, sizeof(OLED_framebuffer));
    update_screen(); // Send to display
}

/**************************************************************************//**
 * @brief   Draws a single character on the OLED display.
 *
 * @details This function renders a single character onto the OLED display
 *          at the specified (x, y) position. The character is represented
 *          using a 5x7 font bitmap 'Font5x7'. Each column of the character is
 *          written into the corresponding position in the framebuffer.
 *
 * @version 1.0
 * @param   uint8_t x, The horizontal starting position (0-127).
 * @param   uint8_t y, The vertical starting position (0-63).
 * @param   char c,    The character to render.
 * @return  None
 *
 * @note    The function only updates the framebuffer and not the display.
 *          To show the changes onscreen, call 'update_screen' after this function.
 *
 * @see     draw_string
 *****************************************************************************/
void draw_char(uint8_t x, uint8_t y, char c) {
    /* Is the character a valid ASCII character? */
    if (c < 32 || c > 126)
        return;

    const uint8_t *char_bitmap = Font5x7[c - 32]; // Get bitmap for character

    for (uint8_t i = 0; i < 5; i++) {  // Each column of the character
        OLED_framebuffer[x + (y / 8) * 128 + i] = char_bitmap[i]; // Calculate framebuffer index
    }
}

/**************************************************************************//**
  * @brief   Draws a string of characters on the OLED display.
  *
  * @details This function writes a string to the OLED framebuffer at the
  *          specified (x, y) coordinates. Each character is rendered using
  *          a 5x7 font stored in the `Font5x7` array. Characters are
  *          spaced by 1 pixel horizontally.

  * @version 1.0
  * @param   uint8_t x, The horizontal starting position (0-127).
  * @param   uint8_t y, The vertical starting position (0-63).
  * @param   char *str, Pointer to the null-terminated string to render.
  * @return  None
  * @see     draw_char
  *****************************************************************************/
void draw_string(uint8_t x, uint8_t y, const char *str) {
    while (*str) {
        draw_char(x, y, *str);
        x += 6; // Move cursor to the next character (5 pixels + 1 for spacing)
        if (x + 5 >= 128) {           // If at the end of the screen
            x = 0;  // Move to the beginning of the next line
            y += 8; // Move down one row
        }
        str++;
    }
    update_screen();
}