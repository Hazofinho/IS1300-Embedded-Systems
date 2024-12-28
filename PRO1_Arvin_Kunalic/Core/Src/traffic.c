/**************************************************************************//**
 * @file     traffic.c
 * @brief    The main program of the Traffic Light Project
 * 
 * @details  This file contains the core state machine logic for managing 
 *           traffic lights and pedestrian crossings in a dual-intersection 
 *           setup. The program dynamically transitions between intersections 
 *           based on sensor input (e.g., car detection, pedestrian requests) 
 *           and time-based delays to guarantee smooth and efficient traffic flow. 
 * 
 *           Key Features:
 *           - State machine with distinct states for managing intersections 
 *             (Intersection1, Intersection2, Wait20s, Wait30s).
 *           - Transition logic based on real-time inputs and active timers.
 *           - Support for pedestrian crossings with timed light changes.
 *           - Integration with STM32 timers and GPIOs for hardware control.
 * 
 *           The system prioritizes scheduling and efficiency by properly 
 *           delaying during transitions and handling conflicting inputs 
 *           (e.g., active cars and pedestrian requests).
 * 
 ******************************************************************************
 * @author   Arvin Kunalic
 * @version  1.0
 * @date     20-December-2024
 * @note     Confirm hardware peripherals (timers, GPIOs) and sensors are 
 *           correctly configured to support the state machine logic. Timers 
 *           and interrupts should be initialized in the STM32CubeMX project.
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

#include "timer_config.h"
#include "traffic_functions.h"
#include "595_shiftreg.h"
#include "ssd1306_config.h"
#include "fonts.h"
#include <stm32l476xx.h>
#include "clock.h"

/* States */
typedef enum {
  Intersection1,
  Intersection2,
  Wait20s,
  Wait30s,
} states;
static states State, NextState;

void Traffic(void) {
    init_program();
    State = Intersection2;
    NextState = Intersection2;

    while (1) {
        State = NextState;

        switch (State) {
            case Intersection1: {
                static uint8_t stage = 0;

                /* Stage 0: If switching from an active intersection to an inactive */
                if (stage == 0) {
                    /* If Intersection1 already is green, skip this stage */
                    if (intersection1_green) {
                        stage = 1;
                        break;
                    }

                    /* Stop active Intersection2 */
                    if (!intersection2_red) {
                        stop_intersection(2);
                    }

                    /* 5s after cars are stopped, allow pedestrians to walk across inactive lane */
                    if (intersection2_red && __HAL_TIM_GetCounter(&htim4) >= pedestrian_Delay) {  
                        stop_and_resetTimer(&htim4);
                        stop_pedestrian(1);
                        go_pedestrian(2);
                        HAL_TIM_Base_Start(&htim4);
                        stage = 1;
                    }  else {
                        break;
                    }
                }

                /* Stage 1: If not already, turn on Intersection1 */
                if (stage == 1 && crosswalk1_red) {
                    if (!intersection1_green) {
                        go_intersection(1);
                    } else if (intersection1_green) {
                        stop_and_resetTimer(&htim4);
                        stage = 2;
                    }
                    break;
                } 

                /* Stage 2: If/when Intersection1 is green, check the following */
                if (stage == 2) {
                
                    /* Pedestrain waiting? */
                    if (PL1_SW_HIT) {
                        NextState = Intersection2;
                        stage = 0;
                        break;
                    }

                    /* Any active cars at all? */
                    if (no_active_cars()) {
                        NextState = Wait30s;
                        stage = 0;
                        HAL_TIM_Base_Start(&htim15);
                        break;
                    }

                    /* If there are active cars at the active Intersection */
                    if (active_cars_at(1)) {
                        /* If cars are also waiting at red light */
                        if (active_cars_at(2)) {
                        NextState = Wait20s;
                        stage = 0;
                        HAL_TIM_Base_Start(&htim15);
                        break;
                        } else { // No cars are waiting at a red light
                            stage = 2;
                            break;
                        }
                    }

                    /* No active cars at the active Intersection, but cars waiting at inactive Intersection */
                    if (!(active_cars_at(1)) && (active_cars_at(2))) {
                        NextState = Intersection2;
                        stage = 0;
                        HAL_TIM_Base_Start(&htim4);
                        break;
                    } else {
                        NextState = Intersection1;
                        stage = 2;
                    }
                    break;
                }
            }

            case Intersection2: {
                static uint8_t stage = 0;
                
                /* Stage 0: If switching from an active intersection to an inactive */
                if (stage == 0) {
                    /* If Intersection2 already is green, skip this stage */
                    if (intersection2_green) {
                        stage = 1;
                        break;
                    }

                    /* Stop active Intersection1 */
                    if (!intersection1_red) {
                        stop_intersection(1);
                    } 

                    /* 5s after cars are stopped, allow pedestrians to walk across inactive lane  */
                    if (intersection1_red && __HAL_TIM_GetCounter(&htim4) >= pedestrian_Delay) {            
                        stop_and_resetTimer(&htim4);
                        stop_pedestrian(2);
                        go_pedestrian(1);
                        HAL_TIM_Base_Start(&htim4);
                        stage = 1;
                    } else {
                        break;
                    }
                }

                /* Stage 1: If not already, turn on Intersection2 */
                if (stage == 1 && crosswalk2_red) {
                    if (!intersection2_green) {
                        go_intersection(2);
                    } else if (intersection2_green) {
                        stop_and_resetTimer(&htim4);
                        stage = 2;
                    }
                    break;
                } 

                /* Stage 2: If/when Intersection2 is green, check the following */
                if (stage == 2) {
                    
                    /* Pedestrain waiting? */
                    if (PL2_SW_HIT) {
                        NextState = Intersection1;
                        stage = 0;
                        break;
                    }

                    /* Any active cars at all? */
                    if (no_active_cars()) {
                        NextState = Wait30s;
                        stage = 0;
                        HAL_TIM_Base_Start(&htim15);
                        break;
                    }

                    /* If there are active cars at the active Intersection*/
                    if (active_cars_at(2)) {
                        /* If cars are also waiting at red light */
                        if (active_cars_at(1)) {
                        NextState = Wait20s;
                        stage = 0,
                        HAL_TIM_Base_Start(&htim15);
                        break;
                        } else { // No cars are waiting at a red light
                            stage = 2;
                            break;
                        }
                    }

                    /* No active cars at the active Intersection, but cars waiting at inactive Intersection */
                    if (!(active_cars_at(2)) && (active_cars_at(1))) {
                        NextState =Intersection1;
                        stage = 0;
                        HAL_TIM_Base_Start(&htim4);
                        break;
                    } else {
                        NextState = Intersection2;
                        stage = 2;
                    }
                    break;
                }
            }

            /* You'll only end up here if there are active cars at the intersection and at the inactive Intersection */
            case Wait20s:
                /* If PL1_SW is pressed while wating interesction1 is active, transition immideately */
                if (PL1_SW_HIT && intersection1_green) {
                    stop_and_resetTimer(&htim15);
                    NextState = Intersection2;
                    break; /* If PL1_SW is pressed while intersection2 is active, turn on crosswalk1 after 5s */
                } 

                /* If PL2_SW is pressed while waiting interesction2 is active, transition immideately */
                if (PL2_SW_HIT && intersection2_green) {
                    stop_and_resetTimer(&htim15);
                    NextState = Intersection1;
                    break; /* If PL2_SW is pressed while intersection1 is active, turn on crosswalk2 after 5s */
                } 

                /* Waits ~ 5s (transition_time = 15s => total time = 20s) */
                if (__HAL_TIM_GetCounter(&htim15) >= red_delay_Max) {
                    stop_and_resetTimer(&htim15);

                    /* If the Intersection before, entering wait was 1, It's the 2:nd Intersections turn */
                    if (intersection1_green) {
                        NextState = Intersection2;
                        HAL_TIM_Base_Start(&htim4);
                        break;
                    }

                    /* Vice versa ^^ */
                    if (intersection2_green) {
                        NextState = Intersection1;
                        HAL_TIM_Base_Start(&htim4);
                        break;
                    }
                } else {
                    NextState = Wait20s;
                }
            break;

            /* You'll only end up here if there are no active cars at a green intersection */
            case Wait30s:
                /* A car is active, go back and check what should be done */
                if (!no_active_cars()) {
                    stop_and_resetTimer(&htim15);
                    if (intersection1_green) {
                        NextState = Intersection1;
                        break;
                    } else if (intersection2_green) {
                        NextState = Intersection2;
                        break;
                    }
                }

                /* If PL1_SW is pressed while wating interesction1 is active, transition immideately */
                if (PL1_SW_HIT && intersection1_green) {
                    stop_and_resetTimer(&htim15);
                    NextState = Intersection2;
                    break; /* If PL1_SW is pressed while intersection2 is active, turn on crosswalk1 after 5s */
                } 

                /* If PL2_SW is pressed while waiting interesction2 is active, transition immideately */
                if (PL2_SW_HIT && intersection2_green) {
                    stop_and_resetTimer(&htim15);
                    NextState = Intersection1;
                    break; /* If PL2_SW is pressed while intersection1 is active, turn on crosswalk2 after 5s */
                } 

                /* Waits ~15s (transition_time = 15s => total time = 30s) */
                if (__HAL_TIM_GetCounter(&htim15) >= green_Delay) {
                    stop_and_resetTimer(&htim15);
                    
                    /* Intersection1 was active before the wait, now switch intersection */
                    if (intersection1_green) {
                        NextState = Intersection2;
                        HAL_TIM_Base_Start(&htim4);
                        break;
                    }

                    /* Intersection2 was active before the wait, now switch intersection */
                    if (intersection2_green) {
                        NextState = Intersection1;
                        HAL_TIM_Base_Start(&htim4);
                        break;
                    }

                } else {
                    NextState = Wait30s;
                }
            break;
        }
    }
}
