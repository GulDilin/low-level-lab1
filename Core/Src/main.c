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
#include "usart.h"
#include "gpio.h"
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
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

struct ModeState states_5[] = { {}, {}, {}, {}, {}, {}, {}, {} };
struct ModeState states_6[] = { {}, {}, {}, {}, {}, {}, {}, {} };
struct ModeState states_7[] = { {}, {}, {}, {}, {}, {}, {}, {} };
struct ModeState states_8[] = { {}, {}, {}, {}, {}, {}, {}, {} };
struct ModeState states_new[] = { {}, {}, {}, {}, {}, {}, {}, {} };

const struct Mode mode_5 = {
	.states = states_5,
	.length = 0
};

const struct Mode mode_6 = {
	.states = states_6,
	.length = 0
};

const struct Mode mode_7 = {
	.states = states_7,
	.length = 0
};

const struct Mode mode_8 = {
	.states = states_8,
	.length = 0
};

struct Mode MODES[] = { mode_1, mode_2, mode_3, mode_4, mode_5, mode_6, mode_7, mode_8 };
struct Mode created_mode  = { .states = states_new, .length = 0 };
struct State saved_states[] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} };
uint32_t exceeded_time = 0;
uint32_t mode_start_time = 0;
uint32_t button_click_time = 0;
uint32_t cur_mode_num = 0;
uint32_t initialized = 0;
uint32_t n_modes = 4;

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
	saved_states[cur_mode_num].exceeded = exceeded_time - mode_start_time;
}


void next_mode() {
	cur_mode_num = (cur_mode_num + 1) % n_modes;
	mode_start_time = exceeded_time - saved_states[cur_mode_num].exceeded;
}

void apply_state() {
	struct ModeState state = MODES[cur_mode_num].states[saved_states[cur_mode_num].n_mode_state];
	set_red(state.R);
	set_green(state.G);
	set_yellow(state.Y);
}

void apply_mode() {
	uint32_t n_state = saved_states[cur_mode_num].n_mode_state;
	if (saved_states[cur_mode_num].exceeded >  MODES[cur_mode_num].states[n_state].delay) {
		saved_states[cur_mode_num].n_mode_state = (saved_states[cur_mode_num].n_mode_state + 1) % MODES[cur_mode_num].length;
		saved_states[cur_mode_num].exceeded = 0;
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
	exceeded_time = HAL_GetTick();
	mode_start_time = HAL_GetTick();
	apply_state();
	initialized = 1;
}

//-----------------------------UART------------------------------------
#define CMD_BUFFER_SIZE 	255
#define MSG_BUFFER_SIZE 	1024
#define CHAR_EOF 			'\0'
#define CHAR_NEWLINE		'\n'
#define CHAR_BITEND			0x7F
#define CHAR_BACKSPACE  	'\b'
#define CHAR_RET			'\r'
#define STR_NEWLINE			"\r\n$>"
#define STR_NEWLINE_LEN 	4
#define STR_RET				"\r"
#define STR_INVALID_CMD 	"\r\nWrong command!!!!"

#define STR_CMD_INT_ON  		"set interrupt on"
#define STR_CMD_INT_OFF 		"set interrupt off"
#define STR_CMD_SET_START 		"set "
#define STR_CMD_SET_START_LEN 	4
#define STR_CMD_CREATE_START 	"new "
#define STR_CMD_NEW_START_LEN 	4

#define STR_INT_ENABLED 	"Interrupts was enabled"
#define STR_INT_DISABLED 	"Interrupts was disabled"
#define UART_TIMEOUT 		10

#define CMD_NOT_FOUND 		0
#define CMD_INT_ON 			1
#define CMD_INT_OFF 		2
#define CMD_CREATE 			3
#define CMD_CREATE_DELAY 	4
#define CMD_TIMEOUT			5
#define CMD_SET				6

#define MODE_N_STATES_MIN	2
#define MODE_N_STATES_MAX	8
#define N_ADD_MODES_MAX		7
#define N_ADD_MODES_MIN		4
#define MODE_CHAR_YELLOW	'y'
#define MODE_CHAR_RED		'r'
#define MODE_CHAR_GREEN		'g'
#define MODE_CHAR_EMPTY		'n'


static char 				received_char;
static bool 				is_char_received = false;
static bool 				is_transmitted = true;
static char 				cmd_buffer[CMD_BUFFER_SIZE];
static bool 				is_interrupt_enabled = false;
static uint32_t 			n_await_delays = 0;
static uint32_t 			n_next_mode = N_ADD_MODES_MIN;

static void send_uart_message(const char * content, uint32_t length) {
	if (is_interrupt_enabled) {
		while (!is_transmitted);

		is_transmitted = false;
		HAL_UART_Transmit_IT(&huart6, (void *) content, length);
	} else {
		HAL_UART_Transmit(&huart6, (void *) content, length, UART_TIMEOUT);
	}
}

static inline void send_cmd_msg(const char * message) {
	send_uart_message(message, strlen(message));
}

static void send_cmd_newline() {
	send_uart_message(STR_NEWLINE, STR_NEWLINE_LEN);
}

static void send_cmd_msg_newline(const char * message) {
	send_cmd_msg(message);
	send_cmd_newline();
}

static bool string_equals(const char * a, const char * b) {
	return strcmp(a, b) == 0;
}

static bool starts_with(const char * prefix, const char * str) {
	return strncmp(prefix, str, strlen(prefix)) == 0;
}

static void normalize_cmd_buffer() {
	uint32_t cmd_buffer_length = strlen(cmd_buffer);
	uint32_t j = 0;
	bool was_space_previous = true;
	for (uint32_t i = 0; i < cmd_buffer_length; ++i) {
		if (isspace((unsigned char)cmd_buffer[i])) {
			if (was_space_previous) continue;
			else {
				was_space_previous = true;
				cmd_buffer[i] = ' ';
			}
		} else {
			was_space_previous = false;
		}
		cmd_buffer[j++] = cmd_buffer[i];
	}

	if (was_space_previous) while (isspace((unsigned char) cmd_buffer[j - 1])) --j;
	cmd_buffer[j] = CHAR_EOF;
}

static int parse_cmd() {
	if (strlen(cmd_buffer) == 0) return CMD_NOT_FOUND;
	if (n_await_delays) return CMD_CREATE_DELAY;
	if (string_equals(STR_CMD_INT_ON, cmd_buffer)) return CMD_INT_ON;
	if (string_equals(STR_CMD_INT_OFF, cmd_buffer)) return CMD_INT_OFF;
	if (starts_with(STR_CMD_SET_START, cmd_buffer)) return CMD_SET;
	if (starts_with(STR_CMD_CREATE_START, cmd_buffer)) return CMD_CREATE;
	return CMD_NOT_FOUND;
}

static bool handle_int_on_command() {
	send_cmd_msg_newline(STR_INT_ENABLED);
	is_interrupt_enabled = true;
	return true;
}

static bool handle_int_off_command() {
	send_cmd_msg_newline(STR_INT_DISABLED);
	is_interrupt_enabled = true;
	return true;
}

static bool handle_set_command() {
	const char * const i_mode_buffer = cmd_buffer + STR_CMD_SET_START_LEN;
	uint32_t i_mode;
	if (sscanf(i_mode_buffer, "%lu", &i_mode) != 1) return false;
	if (i_mode < 1 || i_mode > n_modes) return false;
	cur_mode_num = i_mode - 1;
	return true;
}

static struct ModeState create_mode_state_from_char(char mode_char) {
	struct ModeState mode_state = {.R = 0, .G = 0, .Y = 0, .delay = 0};
	switch (mode_char) {
		case MODE_CHAR_EMPTY: break;
		case MODE_CHAR_RED:
			mode_state.R = 1;
			break;
		case MODE_CHAR_GREEN:
			mode_state.G = 1;
			break;
		case MODE_CHAR_YELLOW:
			mode_state.Y = 1;
			break;
	}
	return mode_state;
}

static bool handle_create_command() {
	const char * const pattern = cmd_buffer + STR_CMD_NEW_START_LEN;
	const uint32_t n_states = strlen(pattern);
	if (n_states < MODE_N_STATES_MIN || n_states > MODE_N_STATES_MAX) return false;
	for (uint8_t i = 0; i < n_states; ++i) {
		switch (pattern[i]) {
			case MODE_CHAR_EMPTY:
			case MODE_CHAR_RED:
			case MODE_CHAR_GREEN:
			case MODE_CHAR_YELLOW:
				created_mode.states[i] = create_mode_state_from_char(pattern[i]);
				break;
			default:
				return false;
		}
	}
	n_await_delays = n_states;
	created_mode.length = n_states;
	send_cmd_msg_newline("Please enter delays");
	return true;
}

static void copy_created_mode_to(uint8_t i_mode) {
	MODES[i_mode].length = created_mode.length;
	for (uint8_t i_state = 0; i_state < created_mode.length; ++i_state) {
		MODES[i_mode].states[i_state].R = created_mode.states[i_state].R;
		MODES[i_mode].states[i_state].G = created_mode.states[i_state].G;
		MODES[i_mode].states[i_state].Y = created_mode.states[i_state].Y;
		MODES[i_mode].states[i_state].delay = created_mode.states[i_state].delay;
	}
}

static bool handle_create_command_delay() {
	const uint8_t i_state = created_mode.length - n_await_delays;
	if (sscanf(cmd_buffer, "%lu", &(created_mode.states[i_state].delay)) != 1) return false;
	char buffer[256];
	n_await_delays--;
	if (n_await_delays == 0) {
		if (n_next_mode > N_ADD_MODES_MAX) n_next_mode = N_ADD_MODES_MIN;
		const uint8_t i_mode = n_next_mode;
		//memcpy(MODES + i_mode, &(created_mode), sizeof(created_mode));
		copy_created_mode_to(i_mode);
		if (n_modes < (N_ADD_MODES_MAX + 1)) n_modes++;
		n_next_mode++;
		memset(buffer, CHAR_EOF, sizeof(buffer));
		sprintf(buffer, "Written in mode  %d", i_mode + 1);
		send_cmd_msg_newline(buffer);
		return true;
	}
	memset(buffer, CHAR_EOF, sizeof(buffer));
	sprintf(buffer, "%lu delays remaining (ms):", n_await_delays);
	send_cmd_msg_newline(buffer);
	return true;
}

static bool handle_command_not_found() {
	send_cmd_msg_newline(STR_INVALID_CMD);
	return true;
}

static bool handle_cmd_command() {
	normalize_cmd_buffer();
	uint32_t cmd_code = parse_cmd(cmd_buffer);
	switch (cmd_code) {
		case CMD_NOT_FOUND: return handle_command_not_found();
		case CMD_INT_ON: return handle_int_on_command();
		case CMD_INT_OFF: return handle_int_off_command();
		case CMD_CREATE_DELAY: if(handle_create_command_delay()) return true;
		case CMD_CREATE: if(handle_create_command()) return true;
		case CMD_SET: if(handle_set_command()) return true;
	}
	send_cmd_msg_newline(STR_INVALID_CMD);
	return true;
}

static void reset_cmd_buffer() {
	memset(cmd_buffer, CHAR_EOF, sizeof(cmd_buffer));
}

static void handle_cmd_overflow() {
	const uint32_t cmd_buffer_length = strlen(cmd_buffer);
	if (cmd_buffer_length < sizeof(cmd_buffer) - 1) return;
	send_cmd_msg_newline(STR_INVALID_CMD);
	reset_cmd_buffer();
}

static void append_recieved_char() {
	const uint32_t cmd_buffer_length = strlen(cmd_buffer);
	cmd_buffer[cmd_buffer_length] = received_char;
}


static void handle_uart() {
    if (is_interrupt_enabled) {
        if (!is_char_received) {
            HAL_UART_Receive_IT(&huart6, (void *) &received_char, sizeof(received_char));
            return;
        }
    } else {
        switch (HAL_UART_Receive(&huart6, (void *) &received_char, sizeof(received_char), UART_TIMEOUT)) {
            case HAL_OK:
                break;

            case HAL_ERROR:
            case HAL_BUSY:
            case HAL_TIMEOUT:
                return;
        }
    }

	is_char_received = false;
    send_uart_message(&received_char, 1); //echo

    switch (received_char) {
    	case CHAR_BACKSPACE:
    	case CHAR_BITEND: {
    	    const uint32_t cmd_buffer_length = strlen(cmd_buffer);
    	    if (cmd_buffer_length > 0) cmd_buffer[cmd_buffer_length - 1] = CHAR_EOF;
    		return;
    	}
    	case CHAR_RET: {
    		send_cmd_newline();
    		handle_cmd_command();
        	memset(cmd_buffer, CHAR_EOF, sizeof(cmd_buffer));
    		return;
    	}
    }

    handle_cmd_overflow();
    append_recieved_char();
}

static void uart_init() {
	send_cmd_newline();
	reset_cmd_buffer();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    (void) huart;
    is_char_received = true;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    (void) huart;
    is_transmitted = true;
}
//-----------------------------UART-END------------------------------------
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
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  uart_init();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  handle_uart();
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
