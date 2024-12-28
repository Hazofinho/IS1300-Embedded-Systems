/**************************************************************************//**
 * @file     timer_config.h
 * @brief    Timer configuration constants for the Traffic Light Project.
 *
 * @details  This file contains definitions for timer constants used throughout
 *           the project. It explains their configuration and purpose for delays 
 *           and transitions in the traffic light system.  
 * 
 *      To achieve specific delays, all timers operate with a prescaler of (40,000 - 1), 
 *      which downscales the system clock from 80MHz to 2kHz. This results in
 *      a tick occuring every 0.5ms (T/f = 0.0005s) in each timer, when active.
 *      
 *      Each timer's Auto-Reload-Register (ARR) represents the amount of ticks that can
 *      occur before the timer triggers an interrupt, event (by polling) or wraps.
 *      
 *      The ARR values values are calculated as follows:
 *        ARR = ((System clock / 40,000) * desired timer count [in ms]) - 1
 *       
 *       - TIM3 (ARR = 249):   125ms timer, used to toggle the blue pedestrian lights with interrupts. 
 *       - TIM4 (ARR = 9999):  5s timer, used to keep track of when the pedestrain button was pressed,
 *                             it's also used to transition the traffic lights and to wait before turning pedestrian lights on/off.
 *      
 *       - TIM5 (ARR = 29999): 15s timer, uses interrupts to: after a PL_SW_HIT and the lights 
 *          are green, turn lights red after 15s of being green.
 *      
 *       - TIM15 (ARR = 59,999): Initially meant to work as a 30s timer (still works as that), for tasks
 *                               R2.4 & R2.6. I later realized, with my configuration, in order to achieve
 *                               a 20 or 30s delay. I have to take in consideration the time to transition
 *                               from one intersection to the next, which is 15s (29,999 ticks).
 *                               Which in turn means, I do not really need to have a timer capable of 30s
 *                               time-keeping, but I find this solution easier to understand for someone
 *                               not completely familiar with the project task.
 * 
 ******************************************************************************
 * @author   Arvin Kunalic
 * @version  1.0
 * @date     20-December-2024
 *****************************************************************************/

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef TIMER_CONFIG_H
#define TIMER_CONFIG_H


/* Exported constants -------------------------------------------------------*/

/* -100 for some margin of error */
#define TIMER_2s            (3999 - 100) // 2s Delay
#define TIMER_5s            (9999 - 100) // 5s Delay

#define toggle_Freq         249     // = 125ms (TIM3) 

#define orange_Delay        (5999 - 100)    // 3s delay (TIM4)
#define pedestrian_Delay    (orange_Delay + TIMER_2s)  // ~ 5s (TIM4)

/* 
* When these constants are used, they will result in 20 and 30s delays, 
* but the constants themselves are in fact 5s and 15s.
*/
#define transition_Time     30000   // 15s to transition from one intersection to another
#define red_delay_Max       (((40000 - transition_Time) - 1) - 100)   // ~ 20s total (TIM15)
#define green_Delay         (((60000 - transition_Time) - 1) - 100)   // ~ 30s total (TIM15) 

#endif