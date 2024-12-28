/**************************************************************************//**
 * @file     fonts.h
 * @brief    Header file for 5x7 pixel font definitions.
 * 
 * @details  This header file declares the 5x7 fixed-width font bitmap array
 *           used for graphical display rendering. The font includes printable 
 *           ASCII characters, each represented as a 5-byte array, where each 
 *           byte corresponds to a column of the character's pixel data.
 * 
 *           It also includes standard library headers required for font usage 
 *           in graphical applications. The font array is defined in the 
 *           corresponding source file, `fonts.c`.
 * 
 ****************************************************************************** 
 * @author   Arvin Kunalic
 * @version  1.0
 * @date     20-December-2024
 * 
 * @note     Ensure that the corresponding `fonts.c` file is included in the 
 *           build process to avoid linking errors. The array is organized 
 *           in the order of printable ASCII characters (from space ' ' to '~').
 * 
 ******************************************************************************/

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef FONTS_H
#define FONTS_H

/* Includes -----------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Exported variables -------------------------------------------------------*/
extern uint8_t Font5x7[][5];

#endif