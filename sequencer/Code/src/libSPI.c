#include "libSPI.h"

void SPI1_IRQHandler (void){
	if (SPI1->SR & SPI_SR_RXNE)
	{
	uint16_t tamp = SPI1->DR;								// enable read flag
	GPIOA->BSRR |= GPIO_BSRR_BS4;							// chip select ON

	}

}

void SPI1_Transmit (seq_t  data) {

	while(SPI1->SR & SPI_SR_BSY);		// wait till SPI ready
	GPIOA->BSRR |= GPIO_BSRR_BR4;		// chip select OFF for DAC
	SPI1->DR = data;
	while(SPI1->SR & SPI_SR_BSY);		// wait till SPI ready
	GPIOA->BSRR |= GPIO_BSRR_BS4;		// chip select ON for DAC
}

void SPI1_Transmit_Leds (seq_t  data) {

	while(SPI1->SR & SPI_SR_BSY);		// wait till SPI ready
	SPI1->DR = data;
	while(SPI1->SR & SPI_SR_BSY);		// wait till SPI ready
	GPIOA->BSRR |= GPIO_BSRR_BS6;		// chip select ON for LEDs
	GPIOA->BSRR |= GPIO_BSRR_BR6;		// chip select OFF for LEDs
}
