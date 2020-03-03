#include "init.h"


void init_RCC_Max_Speed (void)
{

	RCC->CR |= RCC_CR_HSEON;											// enable HSE
	while (!(RCC->CR & RCC_CR_HSERDY));									// wait until HSE start

	FLASH->ACR |= (FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1); 			// FLASH latency: "Two wait states"

	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;									// AHB = SYSCLK\1
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;									// APB1 = HCLK\2 = 36 (max 36MHz)
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;									// APB2 = HCLK\1
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV8;									// ADC = APB2\8 = 16 (14MHz max)


	RCC->CFGR &= ~RCC_CFGR_PLLMULL;										// clear PLL multiplex
	RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;									// clear HSE divider for PLL entry
	RCC->CFGR &= ~RCC_CFGR_PLLSRC;										// clear PLL entry clock source
	RCC->CFGR &= ~RCC_CFGR_SW;											// clear SYSCLK source

	RCC->CFGR |= RCC_CFGR_PLLSRC;										// PLL source = HSE
	RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE;									// HSE \ 1 = 8 MHz
	RCC->CFGR |= RCC_CFGR_PLLMULL16;										// PLLsrcMUL"X9": 8 x 9= 72 MHz

	RCC->CR |= RCC_CR_PLLON;											// PLL ON
	while ((RCC->CR & RCC_CR_PLLRDY)==0);								// wait until PLL is ready

	RCC->CFGR |= RCC_CFGR_SW_PLL;										// SYSCLK Source: PLL
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);				// wait until SW is ready

}


/*void MCO_Output (void){

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;									// Port A CLK Enable
	GPIOA->CRH &= ~GPIO_CRH_CNF8_0;										// Enable Alternate function output Push-pull
	GPIOA->CRH |= GPIO_CRH_CNF8_1;										// Enable Alternate function output Push-pull
	GPIOA->CRH |= GPIO_CRH_MODE8;										// Max speed 50 MHz

	RCC->CFGR |= RCC_CFGR_MCO_PLLCLK_DIV2;								// MCO: PLL\2
}*/


void init_GPIO (void){

	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;									// enable clock PORT C


/************************* PC 13 - LED for tests *******************/
	GPIOC->CRH &= ~GPIO_CRH_CNF13;							// GPIO Push-Pull
	GPIOC->CRH |= GPIO_CRH_MODE13; 							// max speed 50 MHz
	GPIOC->ODR |= GPIO_ODR_ODR13;  // led off

	GPIOA->CRH |= GPIO_CRH_CNF10_1;							// input pull-up PA10
	GPIOA->CRH &= ~GPIO_CRH_CNF10_0;
	GPIOA->CRH &= ~GPIO_CRH_MODE10;  						// MODE = 00
	GPIOA->BSRR |= GPIO_BSRR_BS10;							// pull-up


	//-------- STOP/START Button----------
	GPIOA->CRH |= GPIO_CRH_CNF8_1;							// input pull-up PA10
	GPIOA->CRH &= ~GPIO_CRH_CNF8_0;
	GPIOA->CRH &= ~GPIO_CRH_MODE8;  						// MODE = 00
	//GPIOA->BSRR |= GPIO_BSRR_BS8;							// pull-up

	GPIOB->CRL &= ~GPIO_CRL_CNF2;							// GPIO Push-Pull
	GPIOB->CRL |= GPIO_CRL_MODE2; 							// max speed 50 MHz
	//GPIOB->ODR |= GPIO_ODR_ODR2;  // led off



}

void init_TIM1 (void){

	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;			// enable TIM 1 clk

	TIM1->PSC = 128-1;						// tim_CK = 128/72 = 1000 êHz
	TIM1->ARR = 2500;							// Period: 2000/1000 = 2 Hz (500 mS)

	TIM1->DIER |= TIM_DIER_UIE;					// Enable UPDATE INTERRUPT

	TIM1->CR1 |= TIM_CR1_ARPE;					// Enable Auto-reload preload (ARR register is buffered)
	TIM1->CR1 |= TIM_CR1_CEN;					// Enable TIM1

	NVIC_EnableIRQ(TIM1_UP_IRQn);				// Enable TIM1 UPDATE interrupt

}

void init_TIM4 (void){

	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;			// enable TIM 4 clk

	TIM4->PSC = 64000-1;						// tim_CK = 128/64 = 2 kHz
	TIM4->ARR = 10000;							// Period: 2000/10000 = 0.2 Hz (5 S)

	TIM4->CR1 |= TIM_CR1_ARPE;					// Enable Auto-reload preload (ARR register is buffered)
	TIM4->CNT = 0;
	TIM4->CR1 |= TIM_CR1_CEN;	// Enable TIM 4
}

void clock_select (CLOCK type)
{
	if(type == 1)
	{
		NVIC_DisableIRQ(TIM1_UP_IRQn);				// Disable INTERNAL CLOCK
		//TIM1->CR1 &= ~TIM_CR1_CEN;				// Disable INTERNAL CLOCK
		NVIC_EnableIRQ(EXTI15_10_IRQn);				// Enable EXTERNAL CLOCK
	}
	else
	{
		NVIC_DisableIRQ(EXTI15_10_IRQn);			// Disable EXTERNAL CLOCK
		NVIC_EnableIRQ(TIM1_UP_IRQn);				// Enable INTERNAL CLOCK
		//TIM1->CR1 |= TIM_CR1_CEN;					// Enable INTERNAL CLOCK
	}
}

void init_TIM2_encoder (void){

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;									// enable clock PORT A
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;									// enable clock PORT B
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;									// enable clock alternate function IO

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;									// enable CLK for TIM2

	GPIOB->CRL |= GPIO_CRL_CNF3_1;										// input pull-up PA3
	GPIOB->CRL &= ~GPIO_CRL_CNF3_0;
	GPIOB->CRL &= ~GPIO_CRL_MODE3;// MODE = 00
	//GPIOB->BSRR |= GPIO_BSRR_BS3;										// pull-up

	GPIOA->CRH |= GPIO_CRH_CNF15_1;										// input pull-up PA3
	GPIOA->CRH &= ~GPIO_CRH_CNF15_0;
	GPIOA->CRH &= ~GPIO_CRH_MODE15;  // MODE = 00
	//GPIOA->BSRR |= GPIO_BSRR_BR15;										// pull-up

	// remap to PA15, PB3
	AFIO->MAPR |= AFIO_MAPR_TIM2_REMAP_FULLREMAP;
	AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG_0;
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;
	AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG_2;



	TIM2->PSC = 4-1;
	TIM2->ARR = 255;													// Count to

	// Tdts/4
	TIM2->CR1 |= TIM_CR1_CKD_1;
	TIM2->CR1 &= ~TIM_CR1_CKD_0;
	// Filter: Fsampl = Fdts / 32, N=8
	TIM2->CCMR1 |= (TIM_CCMR1_IC1F | TIM_CCMR1_IC2F);

	// IC1 is mapped on TI1, IC2 is mapped on TI2
	TIM2->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;

	// capture is done on a rising edge of IC1, IC2 (Polarity: front)
	TIM2->CCER &=  ~(TIM_CCER_CC1P | TIM_CCER_CC2P);

	// Change on both channels
	TIM2->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;

	/**************COMPARE MOD***************/
	TIM2->DIER |= TIM_DIER_CC3IE | TIM_DIER_CC4IE; //enable Compare 4/3 interrupt
	TIM2->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC4M);

	TIM2->CNT = 0;

	TIM2->CCR3 = (TIM3->CNT)-1;
	TIM2->CCR4 = (TIM3->CNT)+1;

	TIM2->CR1 |= TIM_CR1_CEN;										// Enable TIM 2

	TIM2->EGR |= TIM_EGR_UG; 	// generate an update to the registers

	NVIC_EnableIRQ(TIM2_IRQn);

	}

void init_TIM3_encoder (void){

	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;									// enable clock PORT B
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;									// enable clock alternate function IO

	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;									// enable CLK for TIM3

	GPIOB->CRL |= GPIO_CRL_CNF4_1;										// input pull-up PB4
	GPIOB->CRL &= ~GPIO_CRL_CNF4_0;
	GPIOB->CRL &= ~GPIO_CRL_MODE4;										// MODE = 00

	GPIOB->CRL |= GPIO_CRL_CNF5_1;										// input pull-up PB5
	GPIOB->CRL &= ~GPIO_CRL_CNF5_0;
	GPIOB->CRL &= ~GPIO_CRL_MODE5;										// MODE = 00


	// remap to PB4-5 (TIM3_REMAP[1:0] = “10” (partial remap))
	AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_PARTIALREMAP;

	// JTAG-DP Disabled and SW-DP Enabled (PB4/NJTRST)
	AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG_0;
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;
	AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG_2;

	TIM3->PSC = 4-1;
	TIM3->ARR = 60;		// Count to

	// Tdts/4
	TIM3->CR1 |= TIM_CR1_CKD_1;
	TIM3->CR1 &= ~TIM_CR1_CKD_0;

	// Filter: Fsampl = Fdts / 32, N=8
	TIM3->CCMR1 |= (TIM_CCMR1_IC1F | TIM_CCMR1_IC2F);

	// IC1 is mapped on TI1, IC2 is mapped on TI2
	TIM3->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;

	// capture is done on a rising edge of IC1, IC2 (Polarity: front)
	TIM3->CCER &=  ~(TIM_CCER_CC1P | TIM_CCER_CC2P);

	// Change on both channels
	TIM3->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;

	/**************COMPARE MOD***************/
	TIM3->DIER |= TIM_DIER_CC3IE | TIM_DIER_CC4IE; 	//enable Compare 4,3 ch interrupt
	TIM3->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC4M);

	TIM3->CNT = 0;

	TIM3->CCR3 = (TIM3->CNT)-1;	// update for normal interrupting
	TIM3->CCR4 = (TIM3->CNT)+1;

	TIM3->CR1 |= TIM_CR1_CEN;										// Enable TIM 2

	TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers

	NVIC_EnableIRQ(TIM3_IRQn);
}

void init_SPI1_Master (void) {

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;						// enable clock PORT A
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;						// enable alternate I\O clock

	/**********SCK-GPIO Configuration**********/

	GPIOA->CRL |= GPIO_CRL_CNF5_1;							// alternate function Push-Pull
	GPIOA->CRL &= ~GPIO_CRL_CNF5_0;
	GPIOA->CRL |= GPIO_CRL_MODE5;							// Max speed 50 MHz

	/**********MOSI-GPIO Configuration**********/

	GPIOA->CRL |= GPIO_CRL_CNF7_1;							// alternate function Push-Pull
	GPIOA->CRL &= ~GPIO_CRL_CNF7_0;
	GPIOA->CRL |= GPIO_CRL_MODE7;							// Max speed 50 MHz

	/********** NSS-GPIO Configuration**********/
	/********** PA4 - chip select for DAC **********/
	GPIOA->CRL &= ~GPIO_CRL_CNF4;							// out Push-Pull
	GPIOA->CRL |= GPIO_CRL_MODE4;							// Max speed 50 MHz

	/********** PA6 - chip select for LEDs **********/
	GPIOA->CRL &= ~GPIO_CRL_CNF6;							// out Push-Pull
	GPIOA->CRL |= GPIO_CRL_MODE6;							// Max speed 50 MHz

	/**********SPI Configuration**********/
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;						// enable clock SPI (1)

	SPI1->CR1 |= SPI_CR1_BR_0;								// SPI rate, APB2\4 = 18 MHz
	SPI1->CR1 &= ~SPI_CR1_CPHA;								// SCK phase = first clock transition is the first data capture edge
	SPI1->CR1 &= ~SPI_CR1_CPOL;								// SCK polarity non inverted
	SPI1->CR1 |= SPI_CR1_DFF;								// 16-bit format
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;							// MSB-First transmit
	SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;					// Software slave management enabled
	SPI1->CR1 |= SPI_CR1_MSTR;								// Master config

	SPI1->CR2 |= SPI_CR2_RXNEIE;							// Enable interrupt TX buffer empty
	SPI1->CR1 |= SPI_CR1_SPE;								// Enable SPI

//	NVIC_SetPriority(SPI1_IRQn, 29);
	//NVIC_EnableIRQ(SPI1_IRQn);								// interrupt NSS function

}


/*
void init_DMA_SPI1 (void){

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;				// Enable DMA1 clock

	DMA1_Channel3->CPAR = (uint32_t)&SPI1->DR;		// Send data to SPI DR reg
	//DMA1_Channel3->CMAR = (uint16_t)arr[0];			// Tx buffer
	DMA1_Channel3->CNDTR = 8;						// Size of Tx buffer

	DMA1_Channel3->CCR &= ~DMA_CCR_CIRC;			// Circular mode disabled
	DMA1_Channel3->CCR &= ~DMA_CCR_PINC;			// Peripheral increment mode disabled

	DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0;			// Memsize 16 it
	DMA1_Channel3->CCR |= DMA_CCR_PSIZE_0;			// Peripheral size 16 bit

	DMA1_Channel3->CCR |= DMA_CCR_DIR;				// Read from memory
	DMA1_Channel3->CCR |= DMA_CCR_MINC;				// Memory increment mode enabled

	SPI1->CR2 |= SPI_CR2_TXDMAEN;					// Tx buffer DMA enabled
}

void SPIWriteDMA (void){

	DMA1_Channel3->CCR &= ~DMA_CCR_EN;				// Disable DMA

	DMA1_Channel3->CNDTR = 7;						// Count to 7
	DMA1->IFCR |=	DMA_IFCR_CTCIF3;				// Clear the corresponding TCIF flag in the DMA_ISR register

	DMA1_Channel3->CCR |= DMA_CCR_EN;				// Enable DMA
}
*/

/*void init_GPIO_external_clock (void){

	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN ;	// enable clock PORT B

	GPIOB->CRH &= ~GPIO_CRH_CNF12_0;							// CNF INPUT PullDown
	GPIOB->CRH |= GPIO_CRH_CNF12_1;								// CNF INPUT PullDown
	GPIOB->CRH &= ~GPIO_CRH_MODE12; 							// Mode input 00
	GPIOB->ODR &= ~GPIO_ODR_ODR12;								// output 0 (PD)

	AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI12_PB;					// Port B, pin 12
	EXTI->RTSR |= EXTI_RTSR_TR12;								// Rising edge
	EXTI->FTSR |= EXTI_FTSR_TR12;								// Falling edge

	EXTI->IMR |= EXTI_IMR_MR12;									// Interrupt request from Line 12 is not masked

	//NVIC_EnableIRQ(EXTI15_10_IRQn);								// interrupt enable function

}*/

/*void init_EXTI1_PB1_SizeButton (void)
{

	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;						// enable clock PORT B
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;						// enable clock AFIO PORT B

	GPIOB->CRL &= ~GPIO_CRL_CNF1_0;							// CNF INPUT Pull-UP-Down
	GPIOB->CRL |= GPIO_CRL_CNF1_1;							// CNF INPUT Pull-UP-Down
	GPIOB->CRL &= ~GPIO_CRL_MODE1; 							// Mode input 00
	GPIOB->ODR |= GPIO_ODR_ODR1;							// output 1 (PU)

	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI1_PB;				// enable EXTI - (Register[1])Port B, pin 1

	EXTI->FTSR |= EXTI_FTSR_TR1;							// Falling edge enabled
	EXTI->RTSR |= EXTI_RTSR_TR1;							// Rising edge enabled

	EXTI->IMR |= EXTI_IMR_MR1;								// Interrupt request from Line 1 is not masked

	NVIC_SetPriority(EXTI1_IRQn, 50);
	//NVIC_EnableIRQ(EXTI1_IRQn);							// interrupt enable function
}*/


void init_EXTI_B9_B8_B7_B6_B1_B0(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;				// enable clock PORT B
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;				// enable clock AFIO PORT B


	// port b pin 0 (SHIFT button)
	GPIOB->CRL &= ~GPIO_CRL_CNF0_0;					// CNF INPUT Pull-UP-Down
	GPIOB->CRL |= GPIO_CRL_CNF0_1;					// CNF INPUT Pull-UP-Down
	GPIOB->CRL &= ~GPIO_CRL_MODE0; 					// Mode input 00
	GPIOB->ODR |= GPIO_ODR_ODR0;					// output 1 (PU)

	// port b pin 1 (REC button)
	GPIOB->CRL &= ~GPIO_CRL_CNF1_0;					// CNF INPUT Pull-UP-Down
	GPIOB->CRL |= GPIO_CRL_CNF1_1;					// CNF INPUT Pull-UP-Down
	GPIOB->CRL &= ~GPIO_CRL_MODE1; 					// Mode input 00
	GPIOB->ODR |= GPIO_ODR_ODR1;					// output 1 (PU)

	// port b pin 6 (right button)
	GPIOB->CRL &= ~GPIO_CRL_CNF6_0;					// CNF INPUT Pull-UP-Down
	GPIOB->CRL |= GPIO_CRL_CNF6_1;					// CNF INPUT Pull-UP-Down
	GPIOB->CRL &= ~GPIO_CRL_MODE6; 					// Mode input 00
	GPIOB->ODR |= GPIO_ODR_ODR6;					// output 1 (PU)

	// port b pin 7	(left button)
	GPIOB->CRL &= ~GPIO_CRL_CNF7_0;					// CNF INPUT Pull-UP-Down
	GPIOB->CRL |= GPIO_CRL_CNF7_1;					// CNF INPUT Pull-UP-Down
	GPIOB->CRL &= ~GPIO_CRL_MODE7; 					// Mode input 00
	GPIOB->ODR |= GPIO_ODR_ODR7;					// output 1 (PU)

	// port b pin 8 (copy button)
	GPIOB->CRH &= ~GPIO_CRH_CNF8_0;					// CNF INPUT Pull-UP-Down
	GPIOB->CRH |= GPIO_CRH_CNF8_1;					// CNF INPUT Pull-UP-Down
	GPIOB->CRH &= ~GPIO_CRH_MODE8; 					// Mode input 00
	GPIOB->ODR |= GPIO_ODR_ODR8;					// output 1 (PU)

	// port b pin 9 (copy button)
	GPIOB->CRH &= ~GPIO_CRH_CNF9_0;					// CNF INPUT Pull-UP-Down
	GPIOB->CRH |= GPIO_CRH_CNF9_1;					// CNF INPUT Pull-UP-Down
	GPIOB->CRH &= ~GPIO_CRH_MODE9; 					// Mode input 00
	GPIOB->ODR |= GPIO_ODR_ODR9;					// output 1 (PU)

	// enable EXTI - (Register 1) Port B, pin 0,1
	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PB | AFIO_EXTICR1_EXTI1_PB;
	// enable EXTI - (Register 2) Port B, pin 6,7
	AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI6_PB | AFIO_EXTICR2_EXTI7_PB;
	// enable EXTI - (Register 3) Port B, pin 8,9
	AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI8_PB | AFIO_EXTICR3_EXTI9_PB;

	EXTI->FTSR |= EXTI_FTSR_TR0 | EXTI_FTSR_TR1 |EXTI_FTSR_TR6 | EXTI_FTSR_TR7 | EXTI_FTSR_TR8 |EXTI_FTSR_TR9;		// Falling edge enabled
	EXTI->RTSR |= EXTI_RTSR_TR0 | EXTI_RTSR_TR1 |EXTI_RTSR_TR6 | EXTI_RTSR_TR7 | EXTI_FTSR_TR8 |EXTI_FTSR_TR9;	   	// Rising edge enabled

	// Interrupt request from Line 0, 1, 6,7,8,9 is not masked
	EXTI->IMR |= EXTI_IMR_MR0 | EXTI_IMR_MR1 | EXTI_IMR_MR6 | EXTI_IMR_MR7 | EXTI_IMR_MR8 | EXTI_IMR_MR9;

	NVIC_EnableIRQ(EXTI9_5_IRQn);					// interrupt enable function

	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_SetPriority(EXTI0_IRQn, 22);
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_SetPriority(EXTI1_IRQn, 39);
}

void init_EXTI_C14_C15 (void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;				// enable clock PORT C
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;				// enable clock AFIO

	// port C pin 14
	GPIOC->CRH &= ~GPIO_CRH_CNF14_0;				// CNF INPUT Pull-UP-Down
	GPIOC->CRH |= GPIO_CRH_CNF14_1;
	GPIOC->CRH &= ~GPIO_CRH_MODE14; 				// Mode input 00
	GPIOC->ODR &= ~GPIO_ODR_ODR14;					// output 1 (PD)

	// port C pin 15
	GPIOC->CRH &= ~GPIO_CRH_CNF15_0;				// CNF INPUT Pull-UP-Down
	GPIOC->CRH |= GPIO_CRH_CNF15_1;
	GPIOC->CRH &= ~GPIO_CRH_MODE15; 				// Mode input 00
	GPIOC->ODR &= ~GPIO_ODR_ODR15;					// output 1 (PD)

	// enable EXTI - (Register 4) Port C, pin 14-15
	AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI14_PC | AFIO_EXTICR4_EXTI15_PC;

	EXTI->FTSR |= EXTI_FTSR_TR14 | EXTI_FTSR_TR15;		// Falling edge enabled
	EXTI->RTSR |= EXTI_RTSR_TR14 | EXTI_RTSR_TR15;		// Rising edge enabled

	// Interrupt request from Line 14-15 is not masked
	EXTI->IMR |= EXTI_IMR_MR14 | EXTI_IMR_MR15;

	NVIC_EnableIRQ(EXTI15_10_IRQn); // interrupt enable function
}

void flash_unlock (void)
{
	FLASH -> KEYR = INT_FLASH_KEY1;
	FLASH -> KEYR = INT_FLASH_KEY2;
}

void flash_lock (void)
{
	FLASH -> CR |= FLASH_CR_LOCK;
}

uint32_t flash_read (uint32_t address)
{
	return (*(__IO uint32_t*) address);
}

void flash_write (uint32_t address, uint32_t data)
{

	FLASH->CR |= FLASH_CR_PG;

	while((FLASH->SR & FLASH_SR_BSY)!=0);

	*(__IO uint16_t*)address = (uint16_t)data;

	while((FLASH->SR & FLASH_SR_BSY)!=0);

	address+=2;
	data>>=16;

	*(__IO uint16_t*)address = (uint16_t)data;

	while((FLASH->SR & FLASH_SR_BSY)!=0);

    FLASH->CR &= ~(FLASH_CR_PG);

}

void flash_page_erase (uint32_t address)
{


	FLASH->CR |= FLASH_CR_PER;
	while ((FLASH->SR & FLASH_SR_BSY) != 0);
	FLASH -> AR = address;
	FLASH->CR |= FLASH_CR_STRT;
	while ((FLASH->SR & FLASH_SR_BSY) != 0);
	FLASH->CR &= ~(FLASH_CR_PER);
}
