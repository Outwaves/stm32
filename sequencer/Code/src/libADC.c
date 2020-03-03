#include "libADC.h"


void init_ADC (void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;		// enable clock PORT A
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;		// enable clock ADC 1

	ADC1->CR2 |= ADC_CR2_ADON;				// enable ADC1
	// analog input PA0
	GPIOA->CRL &= ~GPIO_CRL_CNF0;			// input
	GPIOA->CRL &= ~GPIO_CRL_MODE0;			// mode 00

	ADC1->CR2 |= ADC_CR2_CAL;				// adc calibration start
	while (!(ADC1->CR2 & ADC_CR2_CAL));		// wait till calibration ends

	ADC1->CR2 &= ~ADC_CR2_CONT; 			// single conversion mode

	ADC1->CR2 |= ADC_CR2_EXTSEL;			// SWSTART (software start)
	ADC1->CR2 |= ADC_CR2_EXTTRIG;			// enable start conversion external signal

	ADC1->SMPR2 |= ADC_SMPR2_SMP0;		// 71.5 + 12.5 = 190476 Sa/s
	ADC1->SMPR2 &= ~ADC_SMPR2_SMP0_0;

	ADC1->SQR1 &= ~ADC_SQR1_L; 				// Regular channel sequence length 0000 = 1 conversion
	ADC1->SQR3 &= ~ADC_SQR3_SQ1;			// 1 in queue channel "0"

}

void init_ADC_PA0_to_PA3_injected (void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;		// enable clock PORT A
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;		// enable clock ADC 1


	// analog input PA0
	GPIOA->CRL &= ~GPIO_CRL_CNF0;			// input
	GPIOA->CRL &= ~GPIO_CRL_MODE0;			// mode 00

	// analog input PA1
	GPIOA->CRL &= ~GPIO_CRL_CNF1;			// input
	GPIOA->CRL &= ~GPIO_CRL_MODE1;			// mode 00

	// analog input PA2
	GPIOA->CRL &= ~GPIO_CRL_CNF2;			// input
	GPIOA->CRL &= ~GPIO_CRL_MODE2;			// mode 00

	// analog input PA3
	GPIOA->CRL &= ~GPIO_CRL_CNF3;			// input
	GPIOA->CRL &= ~GPIO_CRL_MODE3;			// mode 00


	ADC1->CR2 |= ADC_CR2_CAL;				// adc calibration start
	while (!(ADC1->CR2 & ADC_CR2_CAL));		// wait till calibration ends

	ADC1->SMPR2 |= ADC_SMPR2_SMP0;			// 239.5 + 12.5 = 63492 sa/s
	ADC1->SMPR2 |= ADC_SMPR2_SMP1;			// 239.5 + 12.5 = 63492 sa/s
	ADC1->SMPR2 |= ADC_SMPR2_SMP2;			// 239.5 + 12.5 = 63492 sa/s
	ADC1->SMPR2 |= ADC_SMPR2_SMP3;			// 239.5 + 12.5 = 63492 sa/s

	ADC1->CR2 |= ADC_CR2_JEXTSEL;			// (software start) External event select for injected group
	ADC1->CR2 |= ADC_CR2_JEXTTRIG;			// enable start conversion by external signal
	ADC1->CR2 &= ~ADC_CR2_CONT; 			// single conversion mode
	ADC1->CR1 |=  ADC_CR1_SCAN;
	ADC1->CR1 |= ADC_CR1_JAUTO;

	ADC1->JSQR |= ADC_JSQR_JL_0;			//  4 conversions
	ADC1->JSQR |= ADC_JSQR_JL_1;			//  4 conversions


	ADC1->JSQR &= ~ADC_JSQR_JSQ4;		// first goes PA0 (00000)
	ADC1->JSQR |= ADC_JSQR_JSQ3_0;		// first goes PA1 (00001)
	ADC1->JSQR |= ADC_JSQR_JSQ2_1;		// first goes PA2 (00010)

	ADC1->JSQR |= ADC_JSQR_JSQ1_0;		// first goes PA3 (00011)
	ADC1->JSQR |= ADC_JSQR_JSQ1_1;


	ADC1->CR2 |= ADC_CR2_ADON;				// enable ADC1
}


uint16_t ADC_getData (void)
{
	ADC1->CR2 |= ADC_CR2_SWSTART;
	while (!(ADC1->SR & ADC_SR_EOC));
	return (ADC1->DR);
}

void ADC_start_injected (void)
{
	ADC1->CR2 |= ADC_CR2_JSWSTART;
	while (!(ADC1->SR & ADC_SR_JEOC));
}
