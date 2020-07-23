/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "scpi/scpi.h"
#include "scpi-def.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticTimer_t osStaticTimerDef_t;
osThreadId_t defaultTaskHandle;
osThreadId_t taskParseCmdHandle;
osThreadId_t taskFeedWDHandle;
osMessageQueueId_t cmdRxQueueHandle;
uint8_t cmdRxQueueBuffer[ 512 * sizeof( uint8_t ) ];
osStaticMessageQDef_t cmdRxQueueControlBlock;
osMessageQueueId_t cmdTxQueueHandle;
uint8_t cmdTxQueueBuffer[ 512 * sizeof( uint8_t ) ];
osStaticMessageQDef_t cmdTxQueueControlBlock;
osTimerId_t cmdReceiveTimeoutHandle;
osStaticTimerDef_t cmdReceiveTimeoutControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskParseCmd(void *argument);
void StartTaskFeedWD(void *argument);
void CmdReceiveTimeoutCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */
osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of cmdReceiveTimeout */
  const osTimerAttr_t cmdReceiveTimeout_attributes = {
    .name = "cmdReceiveTimeout",
    .cb_mem = &cmdReceiveTimeoutControlBlock,
    .cb_size = sizeof(cmdReceiveTimeoutControlBlock),
  };
  cmdReceiveTimeoutHandle = osTimerNew(CmdReceiveTimeoutCallback, osTimerOnce, NULL, &cmdReceiveTimeout_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of cmdRxQueue */
  const osMessageQueueAttr_t cmdRxQueue_attributes = {
    .name = "cmdRxQueue",
    .cb_mem = &cmdRxQueueControlBlock,
    .cb_size = sizeof(cmdRxQueueControlBlock),
    .mq_mem = &cmdRxQueueBuffer,
    .mq_size = sizeof(cmdRxQueueBuffer)
  };
  cmdRxQueueHandle = osMessageQueueNew (512, sizeof(uint8_t), &cmdRxQueue_attributes);

  /* definition and creation of cmdTxQueue */
  const osMessageQueueAttr_t cmdTxQueue_attributes = {
    .name = "cmdTxQueue",
    .cb_mem = &cmdTxQueueControlBlock,
    .cb_size = sizeof(cmdTxQueueControlBlock),
    .mq_mem = &cmdTxQueueBuffer,
    .mq_size = sizeof(cmdTxQueueBuffer)
  };
  cmdTxQueueHandle = osMessageQueueNew (512, sizeof(uint8_t), &cmdTxQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 128
  };
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* definition and creation of taskParseCmd */
  const osThreadAttr_t taskParseCmd_attributes = {
    .name = "taskParseCmd",
    .priority = (osPriority_t) osPriorityHigh,
    .stack_size = 1024
  };
  taskParseCmdHandle = osThreadNew(StartTaskParseCmd, NULL, &taskParseCmd_attributes);

  /* definition and creation of taskFeedWD */
  const osThreadAttr_t taskFeedWD_attributes = {
    .name = "taskFeedWD",
    .priority = (osPriority_t) osPriorityRealtime7,
    .stack_size = 128
  };
  taskFeedWDHandle = osThreadNew(StartTaskFeedWD, NULL, &taskFeedWD_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(100);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTaskParseCmd */
/**
* @brief Function implementing the taskParseCmd thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskParseCmd */
void StartTaskParseCmd(void *argument)
{
  /* USER CODE BEGIN StartTaskParseCmd */
  /* Infinite loop */  
	uint8_t data;
	osStatus_t status;
	SCPI_Init(&scpi_context,
					scpi_commands,
					&scpi_interface,
					scpi_units_def,
					SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
					scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
					scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);	
	for(;;)
  {
		do
		{
			status = osMessageQueueGet(cmdRxQueueHandle, &data, NULL, 0U);
			if (status == osOK)
			{
				SCPI_Input(&scpi_context, &data, 1);
			}
		}while(status == osOK);
    osDelay(1);
  }
  //process_command_task();
  /* USER CODE END StartTaskParseCmd */
}

/* USER CODE BEGIN Header_StartTaskFeedWD */
/**
* @brief Function implementing the taskFeedWD thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskFeedWD */
void StartTaskFeedWD(void *argument)
{
  /* USER CODE BEGIN StartTaskFeedWD */
  /* Infinite loop */
  for(;;)
  {
		FeedWDG();
    osDelay(1);
  }
  /* USER CODE END StartTaskFeedWD */
}

/* CmdReceiveTimeoutCallback function */
void CmdReceiveTimeoutCallback(void *argument)
{
  /* USER CODE BEGIN CmdReceiveTimeoutCallback */
	
  /* USER CODE END CmdReceiveTimeoutCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
