/**
  ******************************************************************************
  * @file    Templates/Src/stm32f1xx.c
  * @author  MCD Application Team
  * @version V1.6.0
  * @date    12-May-2017
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef    TIM4_Handle;
//extern TIM_HandleTypeDef    TIM4_Handle;
extern RTC_HandleTypeDef 	RTC_Handle;
extern ADC_HandleTypeDef 	ADCHandle;
   
/** @addtogroup STM32F1xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f1xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles external lines 0 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void){
	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(BUTTON_MODE) != RESET){
		/* clear interrupt pending bits */
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON_MODE);
		/* call callback */
		HAL_GPIO_EXTI_Callback(BUTTON_MODE);
	}
}

/**
  * @brief  This function handles external lines 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void){
	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(BUTTON_PLUS) != RESET){
		/* clear interrupt pending bits */
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON_PLUS);
		/* call callback */
		HAL_GPIO_EXTI_Callback(BUTTON_PLUS);
	}
}

/**
  * @brief  This function handles external lines 2 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void){
	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(BUTTON_MINUS) != RESET){
		/* clear interrupt pending bits */
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON_MINUS);
		/* call callback */
		HAL_GPIO_EXTI_Callback(BUTTON_MINUS);
	}
}

/**
  * @brief  This function handles external lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void){
	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(BUTTON_SNOOZE) != RESET){
		/* clear interrupt pending bits */
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON_SNOOZE);
		/* call callback */
		HAL_GPIO_EXTI_Callback(BUTTON_SNOOZE);
	}
	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(SWITCH_ALARM) != RESET){
		/* clear interrupt pending bits */
		__HAL_GPIO_EXTI_CLEAR_IT(SWITCH_ALARM);
		/* call callback */
		HAL_GPIO_EXTI_Callback(SWITCH_ALARM);
	}
}

/**
  * @brief  This function handles Timer 2 interrupt requests.
  * @note   None
  * @retval None
  */
void TIM2_IRQHandler(void){
	WS2812_TIM2_callback();
}

/**
  * @brief  This function handles Timer 3 interrupt requests.
  * @param  None
  * @retval None
  */
void TIM4_IRQHandler(void){
	/* TIM4 Update event */
	if(__HAL_TIM_GET_FLAG(&TIM4_Handle, TIM_FLAG_UPDATE) != RESET){
		if(__HAL_TIM_GET_IT_SOURCE(&TIM4_Handle, TIM_IT_UPDATE) !=RESET){
			/* clear interrupt pending bits */
			__HAL_TIM_CLEAR_IT(&TIM4_Handle, TIM_IT_UPDATE);
			/* call callback */
			BUZZER_TIM4_callback();
		}
	}
}

/**
  * @brief  This function handles RTC wakeup interrupt request.
  * @param  None
  * @retval None
  */
void RTC_Alarm_IRQHandler(void){
	  if(__HAL_RTC_ALARM_GET_IT_SOURCE(&RTC_Handle, RTC_IT_ALRA))
	  {
	    /* Get the status of the Interrupt */
	    if(__HAL_RTC_ALARM_GET_FLAG(&RTC_Handle, RTC_FLAG_ALRAF) != (uint32_t)RESET)
	    {
	      /* Alarm callback */
	      RTC_AlarmEventCallback();

	      /* Clear the Alarm interrupt pending bit */
	      __HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handle,RTC_FLAG_ALRAF);
	    }
	  }

	  /* Clear the EXTI's line Flag for RTC Alarm */
	  __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();

	  /* Change RTC state */
	  RTC_Handle.State = HAL_RTC_STATE_READY;
}

/**
  * @brief  This function handles ADC interrupt request.
  * @param  None
  * @retval None
  */
void ADC1_2_IRQHandler(void){
	HAL_ADC_IRQHandler(&ADCHandle);
}

/**
* @brief  This function handles DMA interrupt request.
* @param  None
* @retval None
*/
void DMA1_Channel1_IRQHandler(void){
	HAL_DMA_IRQHandler(ADCHandle.DMA_Handle);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
