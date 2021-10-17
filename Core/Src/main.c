/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define BUTTON_PIN GPIO_PIN_15
#define RED_PIN GPIO_PIN_15
#define YELLOW_PIN GPIO_PIN_14
#define GREEN_PIN GPIO_PIN_13
#define LOOP_DELAY 100
#define BUTTON_DELAY 5
#define BUTTON_INACTIVE_DELAY 2000

struct ModeState {
	uint32_t R;
	uint32_t Y;
	uint32_t G;
	uint32_t delay;
};

struct Mode {
	struct ModeState * states;
	uint32_t length;
};

struct State {
	uint32_t exceeded;
	uint32_t n_mode_state;
};

struct ModeState states_1[] = {
	{
		.R = 0,
		.Y = 1,
		.G = 0,
		.delay = 1000
	},
	{
		.R = 1,
		.Y = 0,
		.G = 0,
		.delay = 1000
	}
};

const struct Mode mode_1 = {
	.states = states_1,
	.length = 2
};

struct ModeState states_2[] = {
	{
		.R = 1,
		.Y = 0,
		.G = 1,
		.delay = 1000
	},
	{
		.R = 0,
		.Y = 1,
		.G = 1,
		.delay = 1000
	}
};

const struct Mode mode_2 = {
	.states = states_2,
	.length = 2
};

struct ModeState states_3[] = {
	{
		.R = 0,
		.Y = 0,
		.G = 1,
		.delay = 1000
	},
	{
		.R = 0,
		.Y = 1,
		.G = 0,
		.delay = 1000
	},
	{
		.R = 0,
		.Y = 0,
		.G = 1,
		.delay = 1000
	},
	{
		.R = 1,
		.Y = 0,
		.G = 0,
		.delay = 1000
	},
};

const struct Mode mode_3 = {
	.states = states_3,
	.length = 4
};

struct ModeState states_4[] = {
	{
		.R = 1,
		.Y = 0,
		.G = 1,
		.delay = 500
	},
	{
		.R = 0,
		.Y = 0,
		.G = 0,
		.delay = 300
	},
	{
		.R = 0,
		.Y = 1,
		.G = 1,
		.delay = 500
	},
	{
		.R = 0,
		.Y = 0,
		.G = 0,
		.delay = 300
	},
	{
		.R = 1,
		.Y = 0,
		.G = 0,
		.delay = 500
	},
	{
		.R = 0,
		.Y = 0,
		.G = 0,
		.delay = 300
	}
};

const struct Mode mode_4 = {
	.states = states_4,
	.length = 6
};

#define N_MODES 4
const struct Mode MODES[] = { mode_1, mode_2, mode_3, mode_4 };
struct State saved_states[] = { {0, 0}, {0, 0}, {0, 0}, {0, 0} };
uint32_t exceeded_time = 0;
uint32_t mode_start_time = 0;
uint32_t button_click_time = 0;
uint32_t n_mode = 0;
uint32_t initialized = 0;



void set_light(uint16_t pin, int value) {
	HAL_GPIO_WritePin(GPIOD, pin, value > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void set_red(int value) {
	set_light(RED_PIN, value);
}

void set_green(int value) {
	set_light(GREEN_PIN, value);
}

void set_yellow(int value) {
	set_light(YELLOW_PIN, value);
}

void save_state() {
	exceeded_time = HAL_GetTick();
	saved_states[n_mode].exceeded = exceeded_time - mode_start_time;
}

void next_mode() {
	n_mode = (n_mode + 1) % N_MODES;
	mode_start_time = exceeded_time - saved_states[n_mode].exceeded;
}

void apply_state() {
	struct ModeState state = MODES[n_mode].states[saved_states[n_mode].n_mode_state];
	set_red(state.R);
	set_green(state.G);
	set_yellow(state.Y);
}

void apply_mode() {
	uint32_t n_state = saved_states[n_mode].n_mode_state;
	if (saved_states[n_mode].exceeded >  MODES[n_mode].states[n_state].delay) {
		saved_states[n_mode].n_mode_state = (saved_states[n_mode].n_mode_state + 1) % MODES[n_mode].length;
		saved_states[n_mode].exceeded = 0;
		mode_start_time = exceeded_time;
		apply_state();
	}
}

void apply_button() {
	GPIO_PinState button_state = HAL_GPIO_ReadPin(GPIOC, BUTTON_PIN);
	if (button_state == GPIO_PIN_SET) return;
	HAL_Delay(BUTTON_DELAY);
	button_state = HAL_GPIO_ReadPin(GPIOC, BUTTON_PIN);
	if (button_state == GPIO_PIN_SET) return;
	if ((HAL_GetTick() - button_click_time) < BUTTON_INACTIVE_DELAY) return;
	button_click_time = HAL_GetTick();
	next_mode();
}

void prepare_default_states() {
	if (initialized > 0) return;
//	for (int i; i < N_MODES; i++) {
//		saved_states[i].n_mode_state = 0;
//	}
	exceeded_time = HAL_GetTick();
	mode_start_time = HAL_GetTick();
	apply_state();
	initialized = 1;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
//  prepare_default_states();
  //apply_state();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  prepare_default_states();
	  save_state();
	  apply_button();
	  apply_mode();
	  HAL_Delay(LOOP_DELAY);
  }
  /* USER CODE END 3 */
}

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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
