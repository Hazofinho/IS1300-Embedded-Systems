/**************************************************************************//**
 * @file     clock.c
 * 
 * @details  This file provides pre generated code using STM32CubeIDE and my own
 *           created ISRs. 
 *           It includes:
 *           - System clock configuration and peripheral initialization.
 *           - GPIO interrupt service routines (ISRs) for pedestrian switches 
 *             and car sensors.
 *           - Timer-based control for pedestrian light transitions, blue light
 *             indicators, and traffic light delays.
 *           - OLED display updates for real-time status of cars and pedestrians.
 * 
 *           Key functionalities:
 *           - React to pedestrian requests using GPIO interrupts.
 *           - Manage pedestrian and traffic light states using timers and shift registers.
 *           - Display system status on the SSD1306 OLED.
 * 
 ******************************************************************************
 * @author   Arvin Kunalic
 * @version  1.0
 * @date     20-December-2024
 ******************************************************************************/

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

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**************************************************************************//**
 * @brief    ISR for the switches and buttons of the traffic light shield
 * @details  Based off of: https://wiki.st.com/stm32mcu/wiki/Getting_started_with_EXTI
 * @version  1.0
 * @param    uint16_t GPIO_Pin, the GPIO pin that triggered the interrupt.
 * @return   None
 * @see      https://wiki.st.com/stm32mcu/wiki/Getting_started_with_EXTI
 *****************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  switch (GPIO_Pin) {
    case PL1_Switch_Pin:
      if (!PL1_SW_HIT && crosswalk1_red) {
        PL1_SW_HIT = 1;
        draw_string(0, 0, "Pedestrian1        ");
        draw_string(0, 8, "   wants to cross..");
        HAL_TIM_Base_Start_IT(&htim3); // Start toggling blue lights
        HAL_TIM_Base_Start(&htim4); // Start 5s timer to transition lights
      }
    break;

    case PL2_Switch_Pin:
      if (!PL2_SW_HIT && crosswalk2_red) {
        PL2_SW_HIT = 1;
        draw_string(0, 0, "Pedestrian2        ");
        draw_string(0, 8, "   wants to cross..");
        HAL_TIM_Base_Start_IT(&htim3); // Start toggling blue lights
        HAL_TIM_Base_Start(&htim4); // Start 5s timer to transition lights
      }
    break;

    case TL1_Car_Pin:
      if (HAL_GPIO_ReadPin(TL1_Car_GPIO_Port, TL1_Car_Pin) == 0) {
        car1_active = 1;
        draw_string(0, 31, "Car1 active  ");
      } else {
        car1_active = 0;
        draw_string(0, 31, "Car1 inactive");
      }
    break;

    case TL2_Car_Pin:
      if (HAL_GPIO_ReadPin(TL2_Car_GPIO_Port, TL2_Car_Pin) == 0) {
        car2_active = 1;
        draw_string(0, 39, "Car2 active  ");
      } else {
        car2_active = 0;
        draw_string(0, 39, "Car2 inactive");
      }
    break;

    case TL3_Car_Pin:
      if (HAL_GPIO_ReadPin(TL3_Car_GPIO_Port, TL3_Car_Pin) == 0) {
        car3_active = 1;
        draw_string(0, 47, "Car3 active  ");
      } else {
        car3_active = 0;
        draw_string(0, 47, "Car3 inactive");
      }
    break;

    case TL4_Car_Pin:
      if (HAL_GPIO_ReadPin(TL4_Car_GPIO_Port, TL4_Car_Pin) == 0) {
        car4_active = 1;
        draw_string(0, 55, "Car4 active  ");
      } else {
        car4_active = 0;
        draw_string(0, 55, "Car4 inactive");
      }
    break;
  }
}

/**************************************************************************//**
 * @brief    ISR for the timers on the STM32L476RG
 * @details  Based off of: 
 *           https://www.digikey.com/en/maker/projects/getting-started-with-stm32-timers-and-timer-interrupts/d08e6493cefa486fb1e79c43c0b08cc6
 * @version  1.0
 * @param    TIM_HandleTypeDef *htim, the Timer that triggered the interrupt.
 * @return   None
 * @see      https://www.digikey.com/en/maker/projects/getting-started-with-stm32-timers-and-timer-interrupts/d08e6493cefa486fb1e79c43c0b08cc6
 *****************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM3) {
    /* Toggle the blue LEDS every 125ms, with TIM3*/
    if (PL1_SW_HIT && crosswalk1_red) {
      toggle_pedestrian(1);
      return;
    } else if (PL2_SW_HIT && crosswalk2_red) {
      toggle_pedestrian(2);
      return;
    }

    /* Crosswalk is green, turn of blue indicator lights */
    if (PL1_SW_HIT && crosswalk1_green) {
      clear_pin(PL1_Blue);
      PL1_SW_HIT = 0;

      /* Stop and reset the 125ms timer (TIM3) */
      __HAL_TIM_SetCounter(&htim3, 0);
      __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
      HAL_TIM_Base_Stop_IT(&htim3);
      return;
    }

    /* Crosswalk is green, turn of blue indicator lights */
    if (PL2_SW_HIT && crosswalk2_green) {
      clear_pin(PL2_Blue);
      PL2_SW_HIT = 0;
      
      /* Stop and reset the 125ms timer (TIM3) */
      __HAL_TIM_SetCounter(&htim3, 0);
      __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
      HAL_TIM_Base_Stop_IT(&htim3);
    }
  }

  /* Ensure the pedestrian lights stays green for 'walking_Delay' seconds*/
  if (htim->Instance == TIM5) {
    if (crosswalk1_green && intersection1_green) {
      stop_pedestrian(1);

      /* Clear 'walking_Delay' timer */
      HAL_TIM_Base_Stop_IT(&htim5);
      __HAL_TIM_SetCounter(&htim5, 0);
      __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_UPDATE);
      return;
    } else if (crosswalk2_green && intersection2_green) {
      stop_pedestrian(2);

      /* Clear 'walking_Delay' timer */
      HAL_TIM_Base_Stop_IT(&htim5);
      __HAL_TIM_SetCounter(&htim5, 0);
      __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_UPDATE);
      return;
    }
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */