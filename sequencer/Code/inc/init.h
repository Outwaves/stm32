

#ifndef CODE_INC_SEQUENCE_H_
#define CODE_INC_SEQUENCE_H_

#include "stm32f1xx.h"
#include "sequence.h"

typedef enum settingsTIM1 {
	internal = 0, external = 1
} CLOCK;

#define INT_FLASH_KEY1 	((uint32_t)0x45670123)
#define INT_FLASH_KEY2 	((uint32_t)0xCDEF89AB)


void init_RCC_Max_Speed (void);
void MCO_Output (void);
void init_GPIO (void);
void init_SPI1_Master (void);

void init_TIM1 (void);
void init_TIM2_encoder (void);
void init_TIM3_encoder (void);
void init_TIM4(void);

void init_EXTI_B9_B8_B7_B6_B1_B0(void);
void init_EXTI_C14_C15(void);

void EXTI15_10_IRQHandler (void);
void init_GPIO_external_clock (void);
void clock_select(CLOCK type);
//void init_EXTI1_PB1_SizeButton (void);

void flash_unlock (void);
void flash_lock (void);
void flash_write (uint32_t address, uint32_t data);
uint32_t flash_read (uint32_t address);
void flash_page_erase (uint32_t address);
#endif
