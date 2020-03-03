#ifndef CODE_INC_LIBSPI_H_
#define CODE_INC_LIBSPI_H_

#include "stm32f1xx.h"


typedef unsigned short seq_t;
void SPI1_IRQHandler (void);

void SPI1_Transmit (uint16_t data);
void SPI1_Transmit_Leds (uint16_t  data);

#endif



