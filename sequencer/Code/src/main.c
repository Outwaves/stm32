#include "main.h"

	typedef unsigned short seq_t;

/************************* sequence attributes ****************************/

	seq_t sequence[8][512] = {};		// Array of channels and their values
	seq_t lengthOfSeq[8] = {};			// Array of channel length
	seq_t channelDivider[8] = {};		// Array of dividers
	seq_t generalBPM = 60;
	seq_t cvBPM = 15;
	char gateStates[8][512] = {};		// Array of on\off values for playing notes ( 0-32, 32 - full, 0 - empty)
	char channelDirection[8] = {};		// array of channel position counter direction ( == 0  forward, != 0 - backward)

	// -------- COPY-PASTE VAR ---------
	seq_t copySeqBuffer[256] = {};		// max size 128 main positions
	char copyGateBuffer[256] = {};		// max size 128 main positions
	unsigned char copyNumCopy = 0; 		// 0 = copy 1, 255 = copy 256
	unsigned char copyStartPosition = 0; 	// when copy button pressed
	unsigned char copyEndPosition = 0; 	// when copy button released
	char copyReverseRotate = 0;		// when start pos > end pos

	// -------- SHIFTING VAR ---------
	char cursorPosBeforeShifting = 0;

	// -------- COUNTERS ---------
	seq_t positionArray[8] = {};		// Array of current position on channel
	seq_t gateCounter[8] = {};			//Array of gate on/of counters
	seq_t channelDividerCounter[8] = {}; // number of noteDivider repeats

	// -------- PERMISSIONS ---------
	char cvOnChannel[8] = { };			// array of channel CV permission (== 0 - no CV, != 0 - CV)
	char cvStepOnChannel[8] = {};		// array of channel CV/step permission (== 0 - no CV, != 0 - CV)
	char resetOnChannel[8] = {};

	char pauseOnChannel[8] = {};
	char pauseOnChannelPermission[8] = {};

	char recordPermission = 0;
	char recordPrepare = 0;
	/************************* temporary ****************************/

	volatile seq_t clockOutCounter = 0;
	char clockType = 0;
	seq_t clockCounterV = 63;


	uint32_t addressInSwitch = 0;
	char savingNow = 0;
	char loadingNow = 0;
	
	//---------- CV temporary -----------
	char cvTemp = 0;		// to start ADC every 4 TIM1 counter cycle


	seq_t gateTxNumber = 0;
	seq_t gateTxNumberLast = 0;
	
	// --------- LCD ---------
	seq_t lcdTimerCounter = 0;

	seq_t cursorPos = 0;
	seq_t cursorPosBeforeGatePage = 0;

	seq_t currentChannel = 0;

	char copyingNow = 0;	// for LCD
	char shiftingNow = 0;
	char gateChangigngNow = 0;

	// --------- Note change ---------
	char previousPos = 0;

	// --------- Current page ---------
	unsigned char mainPage = 255;
	unsigned char gatePage = 0;
	unsigned char menuPage = 0;

	unsigned char menuPageNum = 0;


	//-------- GATE PAGE ---------
	unsigned char cvChanging = 255;
	unsigned char gateChanging = 0;



/************************* temporary end ****************************/

	int main (void) {

	init_RCC_Max_Speed();
	init_TIM4();

	init_lcd();
	writeCustomSymb();

	lcdSendString("LOADING...");

     init_GPIO();
     init_TIM2_encoder();
     init_TIM3_encoder();
	init_SPI1_Master();
	init_ADC_PA0_to_PA3_injected();

	// permission to CV on all channels
	// permission to CV/step on all channels
	// set mult to 1 on all channels
	for (int channel = 0; channel < 8; ++channel)
	{
		cvOnChannel[channel] = 1;
		cvStepOnChannel[channel] = 0;
		channelDivider[channel] = 1;
		lengthOfSeq[channel] = 15;
		channelDirection[channel] = 0;

		resetOnChannel[channel] = 1;

		pauseOnChannel[channel] = 0;
		pauseOnChannelPermission[channel] = 1;

		// set seq values to zero
		for (int pos = 0; pos < 512; ++pos)
		{
			seqSetVal(channel, pos, 0);
			gateStates[channel][pos] = 0;
		}
		SPI1_Transmit_Leds(1 << channel);
	}

	clock_select(internal);
	init_TIM1();
	timerSetBPM(generalBPM);

/*
	lcdSendFullByte(0, data); // GATE 1
	lcdSendFullByte(1, data); // GATE 0
	lcdSendFullByte(2, data); // cursor on GATE 1
	lcdSendFullByte(3, data); // cursor on GATE 0
	lcdSendFullByte(6, data); // noSEQ
	lcdSendFullByte(5, data); // cursor on noSEQ
	lcdSendFullByte(4, data); // position on GATE 1
	lcdSendFullByte(0xFF, data); // position on GATE 0
*/

	init_EXTI_B9_B8_B7_B6_B1_B0();
	init_EXTI_C14_C15();

	while (1)
	{

		if (menuPage != 0)
		{
			showMenuPage(currentChannel, TIM2->CNT);
		}
		else
		{
			if (mainPage != 0)
			{
				if (shiftingNow == 0 && gateChangigngNow == 0)
				{
					cursorPos = TIM2->CNT;
				}
				showMainSequence(currentChannel, cursorPos);
				showNote(currentChannel, cursorPos);
			}
			if (gatePage != 0)
			{
				gatePageShow(currentChannel, cursorPosBeforeGatePage,
						TIM2->CNT);
				showNoteGatePage(currentChannel, cursorPosBeforeGatePage,
						TIM2->CNT);
			}
		}

		if (GPIOA->IDR & GPIO_IDR_IDR8)
		{
			for (seq_t channel = 0; channel < 8; ++channel)
			{
				if (resetOnChannel[channel] != 0)
				{
					gateCounter[channel] = 0;
					positionArray[channel] = 0;
				}
			}
		}

		if (GPIOA->IDR & GPIO_IDR_IDR9)
		{
			for (seq_t channel = 0; channel < 8; ++channel)
			{
				if (pauseOnChannelPermission[channel] != 0)
				{
					pauseOnChannel[channel] = 1;
				}
				else
				{
					pauseOnChannel[channel] = 0;
				}
			}
		}
		else
		{
			for (seq_t channel = 0; channel < 8; ++channel)
			{
				pauseOnChannel[channel] = 0;
			}
		}
	}
}



//********************************* LEFT ENCODER ROTATION *********************************
void TIM3_IRQHandler (void)
{
	TIM3->SR &= ~TIM_SR_UIF;
	if (TIM3->SR & TIM_SR_CC3IF)
	{
		TIM3->SR &= ~TIM_SR_CC3IF;


		if ((mainPage != 0) && (menuPage == 0))
		{
			for (int smallPosition = (2 * TIM2->CNT);
					smallPosition < (2 + 2 * TIM2->CNT); ++smallPosition)
			{
				seqSetNote(currentChannel, smallPosition, TIM3->CNT);
				if (TIM3->CNT == 0)
				{
					gateStates[currentChannel][smallPosition] = 0;
				}
				else
				{
					gateStates[currentChannel][smallPosition] = 32;
				}
			}
			TIM3->CNT = seqGetNoteNumber(currentChannel, TIM2->CNT * 2);
		}


	}
	if (TIM3->SR & TIM_SR_CC4IF)
	{
		TIM3->SR &= ~TIM_SR_CC4IF;

		if ((mainPage != 0) && (menuPage == 0))
		{
			for (int smallPosition = (2 * TIM2->CNT);
					smallPosition < (2 + 2 * TIM2->CNT); ++smallPosition)
			{
				seqSetNote(currentChannel, smallPosition, TIM3->CNT);
				if (TIM3->CNT == 0)
				{
					gateStates[currentChannel][smallPosition] = 0;
				}
				else
				{
					gateStates[currentChannel][smallPosition] = 32;
				}
			}
			TIM3->CNT = seqGetNoteNumber(currentChannel, TIM2->CNT * 2);
		}
	}

	//********************************* MENU LEFT ROTATION **********************************
	if (menuPage != 0)
	{
		if(menuPageNum == 0)
		{
			currentChannel = TIM3->CNT;
		}
		if (menuPageNum == 1)
		{
			generalBPM = TIM3->CNT;
			if (generalBPM <= 15) generalBPM = 15;
			if (generalBPM > 300) generalBPM = 300;
			timerSetBPM(generalBPM);
			 TIM3->CNT = generalBPM;
		}
		if (menuPageNum == 2)
		{
			channelDivider[currentChannel] = TIM3->CNT;
			TIM3->CNT = channelDivider[currentChannel];
		}
		if (menuPageNum == 3)
		{
			channelDirection[currentChannel] = TIM3->CNT;
			TIM3->CNT = channelDirection[currentChannel];
		}
		if (menuPageNum == 4)
		{
			cvOnChannel[currentChannel] = TIM3->CNT;
			TIM3->CNT = cvOnChannel[currentChannel];
		}
		if (menuPageNum == 5)
		{
			cvStepOnChannel[currentChannel] = TIM3->CNT;
			TIM3->CNT = cvStepOnChannel[currentChannel];
		}
		if (menuPageNum == 6)
		{
			resetOnChannel[currentChannel] = TIM3->CNT;
			TIM3->CNT = resetOnChannel[currentChannel];
		}
		if (menuPageNum == 7)
		{
			pauseOnChannelPermission[currentChannel] = TIM3->CNT;
			TIM3->CNT = pauseOnChannelPermission[currentChannel];
		}
		if (menuPageNum == 11)
		{
			clockType = TIM3->CNT;
			TIM3->CNT = clockType;
			switch (clockType) {
			case 0:
				clockCounterV = 15;
				break;
			case 1:
				clockCounterV = 31;
				break;
			case 2:
				clockCounterV = 63;
				break;
			case 3:
				clockCounterV = 127;
				break;
			case 4:
				clockCounterV = 255;
				break;
			case 5:
				clockCounterV = 511;
				break;
			case 6:
				clockCounterV = 1023;
				break;
			}

			clockOutCounter = 0;
			for (seq_t var = 0; var < 8; ++var)
			{
				positionArray[var] = 0;
				gateCounter[var] = 0;
			}
		}

	}
	else
	{
		//********************************* MAIN PAGE LEFT ROTATION **********************************
		/*if (mainPage != 0)
		{


			for (int smallPosition = (2 * TIM2->CNT);
					smallPosition < (2 + 2 * TIM2->CNT); ++smallPosition)
			{
				seqSetNote(currentChannel, smallPosition, TIM3->CNT);
				if (TIM3->CNT == 0)
				{
					gateStates[currentChannel][smallPosition] = 0;
				}
				else
				{
					gateStates[currentChannel][smallPosition] = 32;
				}
			}
			TIM3->CNT = seqGetNoteNumber(currentChannel, TIM2->CNT * 2);


		}
*/
		//********************************* GATE PAGE LEFT ROTATION **********************************
		if (gatePage != 0)
		{
			if (cvChanging != 0)
			{
				seqSetNote(currentChannel,
						cursorPosBeforeGatePage * 2 + TIM2->CNT,
						TIM3->CNT);
			}
			if (gateChanging != 0)
			{
				if (TIM3->CNT > 32)
					TIM3->CNT = 32;
				gateStates[currentChannel][cursorPosBeforeGatePage * 2
						+ TIM2->CNT] =
				TIM3->CNT;
			}
		}
	}

	TIM3->CCR3 = (TIM3->CNT) - 1;
	TIM3->CCR4 = (TIM3->CNT) + 1;
}


//********************************* RIGHT ENCODER ROTATION *********************************
void TIM2_IRQHandler (void)
{
	TIM2->SR &= ~TIM_SR_UIF;
	if (TIM2->SR & TIM_SR_CC3IF)
	{
		// left rotate
		TIM2->SR &= ~TIM_SR_CC3IF;


		if ((mainPage != 0) && (shiftingNow != 0))
		{
			seqShift(currentChannel, 0);
		}

		if ((mainPage != 0) && (gateChangigngNow != 0))
		{
			seqSetGate (TIM2->CNT, cursorPosBeforeShifting);
		}

	}
	if (TIM2->SR & TIM_SR_CC4IF)
	{
		// right rotate
		TIM2->SR &= ~TIM_SR_CC4IF;


		if ((mainPage != 0) && (shiftingNow != 0))
		{
			seqShift(currentChannel, 1);
		}

		if ((mainPage != 0) && (gateChangigngNow != 0))
		{

			seqSetGate (TIM2->CNT, cursorPosBeforeShifting);
		}


	}

//********************************* MENU RIGHT ROTATION **********************************
	if (menuPage != 0)
	{
		menuPageNum = TIM2->CNT;
		// тут можно было использовать case
		if (menuPageNum == 0)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 7; 				// Count from 1 to 8 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
			TIM3->CNT = currentChannel;
		}
		if (menuPageNum == 1)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 301; 				// Count from 15 to 301 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
			TIM3->CNT = generalBPM;
		}
		if (menuPageNum == 2)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 64; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
			TIM3->CNT = channelDivider[currentChannel];
		}
		if (menuPageNum == 3)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 1; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
			TIM3->CNT = channelDirection[currentChannel];
		}
		if (menuPageNum == 4)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 1; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
			TIM3->CNT = cvOnChannel[currentChannel];
		}
		if (menuPageNum == 5)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 1; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
			TIM3->CNT = cvStepOnChannel[currentChannel];
		}

		if (menuPageNum == 6)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 1; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
			TIM3->CNT = resetOnChannel[currentChannel];
		}
		if (menuPageNum == 7)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 1; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
			TIM3->CNT = pauseOnChannelPermission[currentChannel];
		}
		if (menuPageNum == 8)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 2; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
		}
		if (menuPageNum == 9)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 2; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
		}
		if (menuPageNum == 10)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 7; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
		}
		if (menuPageNum == 11)
		{
			// update regs to avoid errors after moving cursor
			TIM3->ARR = 6; 				// Count from 0 to 64 channel
			TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
		}

		// update regs to avoid errors after moving
		TIM3->CCR3 = (TIM3->CNT) - 1;
		TIM3->CCR4 = (TIM3->CNT) + 1;
	}
	else
	{

		//********************************* MAIN PAGE RIGHT ROTATION **********************************
		if (mainPage != 0)
		{
			/*
			 * TIM3 interrupts by:
			 * 	Capture/Compare 4,3 chan. interrupt
			 * 	Update interrupt
			 */
			NVIC_DisableIRQ(TIM3_IRQn);


			TIM3->CNT = seqGetNoteNumber(currentChannel, TIM2->CNT * 2);	// update note number in left encoder
			TIM3->CCR3 = (TIM3->CNT) - 1; 		// update regs to avoid problems with next encoder turn
			TIM3->CCR4 = (TIM3->CNT) + 1;

			TIM3->SR &= ~TIM_SR_UIF; 		// clear interrupt bits
			TIM3->SR &= ~TIM_SR_CC4IF;
			TIM3->SR &= ~TIM_SR_CC3IF;

			//TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers

			NVIC_EnableIRQ(TIM3_IRQn);

		}

		//********************************* GATE PAGE RIGHT ROTATION **********************************
		if (gatePage != 0)
		{
			if (cvChanging != 0)
			{
				TIM3->CNT = seqGetNoteNumber(currentChannel,
						cursorPosBeforeGatePage * 2 + TIM2->CNT);
			}
			if (gateChanging != 0)
			{
				TIM3->CNT = gateStates[currentChannel][cursorPosBeforeGatePage
						* 2 + TIM2->CNT];
			}
			// update regs to avoid errors after moving cursor
			TIM3->CCR3 = (TIM3->CNT) - 1;
			TIM3->CCR4 = (TIM3->CNT) + 1;
		}
	}

	TIM2->CCR3 = (TIM2->CNT) - 1;
	TIM2->CCR4 = (TIM2->CNT) + 1;
}

//-------------------------------------------------- ENCODER BUTTONS ---------------------------------------------------
void EXTI9_5_IRQHandler (void)
{
	//********************************************** RIGHT BUTTON **********************************************
	if (EXTI->PR & EXTI_PR_PR6)
	{
		//---------PUSH--------
		if (GPIOB->IDR & GPIO_IDR_IDR6)
		{
			TIM4->CNT = 0;
			if (menuPageNum == 8)
			{
				savingNow = 1;
			}
			if (menuPageNum == 9)
			{
				loadingNow = 1;
			}

		}
		//---------RELEASE--------
		if (!(GPIOB->IDR & GPIO_IDR_IDR6))
		{
			if (menuPage != 0)
			{
				if (menuPageNum == 8)
				{
					//-------- short PRESS (SAVE SEQ)--------
					if (TIM4->CNT < 10000)
					{// 08007000, 0800A000, 0800D000

						TIM1->CCER &= ~TIM_CR1_CEN;
						TIM1->CNT = 0;
						clockOutCounter = 0;
						for (seq_t var = 0; var < 8; ++var)
						{
						positionArray[var] = 0;
						gateCounter[var] = 0;
						}

						switch (TIM3->CNT)
						{
							case 0: addressInSwitch = 0x08007000; break;
							case 1: addressInSwitch = 0x0800A000; break;
							case 2: addressInSwitch = 0x0800D000; break;
						}


						eraseSeqValues(addressInSwitch);
						saveSeqValues(addressInSwitch);

						switch (TIM3->CNT)
						// 08006C00, 08006800, 08006400
						{
							case 0: addressInSwitch = 0x08006C00; break;
							case 1: addressInSwitch = 0x08006800; break;
							case 2: addressInSwitch = 0x08006400; break;
						}

						flash_unlock();
						flash_page_erase(addressInSwitch);
						flash_lock();
						saveSeqPref(addressInSwitch);

						savingNow = 0;

						TIM1->CCER |= TIM_CR1_CEN;

					}
					savingNow = 0;
				}
				if (menuPageNum == 9)
				{
					//-------- short PRESS (LOAD SEQ)--------
					if (TIM4->CNT < 10000)
					{ 		// 08007000, 0800A000, 0800D000

						TIM1->CCER &= ~TIM_CR1_CEN;
												TIM1->CNT = 0;
												clockOutCounter = 0;
												for (seq_t var = 0; var < 8; ++var)
												{
												positionArray[var] = 0;
												gateCounter[var] = 0;
												}

						switch (TIM3->CNT)
						{
						case 0:
							addressInSwitch = 0x08007000;
							break;
						case 1:
							addressInSwitch = 0x0800A000;
							break;
						case 2:
							addressInSwitch = 0x0800D000;
							break;
						}

						loadSeqValues(addressInSwitch);

						switch (TIM3->CNT)
						// 08006C00, 08006800, 08006400
						{
							case 0: addressInSwitch = 0x08006C00; break;
							case 1: addressInSwitch = 0x08006800; break;
							case 2: addressInSwitch = 0x08006400; break;
						}

						loadSeqPef(addressInSwitch);

						loadingNow = 0;
					}
					loadingNow = 0;
					TIM1->CCER |= TIM_CR1_CEN;

				}
				if (menuPageNum == 10)
				{
					seqClearAllChannel(TIM3->CNT);
				}
				savingNow = 0;
				loadingNow = 0;
			}
			else
			{

				if (mainPage != 0)
				{
					//-------- LONG PRESS (CHANGE SEQ LENGTH)--------
					if (TIM4->CNT > 1000)
					{

						lengthOfSeq[currentChannel] = 1 + TIM2->CNT * 2;
					}

					//-------- SHORT PRESS (CLEAR CELL)--------
					if (TIM4->CNT > 50 && TIM4->CNT < 1500)
					{
						if (mainPage != 0)
						{
							//-------- CLEAR IN MAIN functions --------

							for (int smallPosition = (2 * TIM2->CNT);
									smallPosition < (2 + 2 * TIM2->CNT);
									++smallPosition)
							{
								seqSetNote(currentChannel, smallPosition, 0);
								gateStates[currentChannel][smallPosition] = 0;
							}

							// update regs to avoid errors after cleaning
							TIM3->CNT = seqGetNoteNumber(currentChannel,
							TIM2->CNT * 2);
							TIM3->CCR3 = (TIM3->CNT) - 1;
							TIM3->CCR4 = (TIM3->CNT) + 1;
						}
					}

				}
				if (gatePage != 0)
				{

					if (TIM4->CNT > 100 && TIM4->CNT < 1000)
					{
						//-------- CHANGING CV or GATE --------
						gateChanging = !gateChanging;
						cvChanging = !cvChanging;
						if (cvChanging > 0)
						{
							// update regs to avoid errors after cleaning
							TIM3->CNT = seqGetNoteNumber(currentChannel,
									cursorPosBeforeGatePage * 2 + TIM2->CNT);
							TIM3->CCR3 = (TIM3->CNT) - 1;
							TIM3->CCR4 = (TIM3->CNT) + 1;
						}
						if (gateChanging > 0)
						{
							TIM3->CNT =
									gateStates[currentChannel][cursorPosBeforeGatePage
											* 2 + TIM2->CNT];
							// update regs to avoid errors after moving cursor
							TIM3->CCR3 = (TIM3->CNT) - 1;
							TIM3->CCR4 = (TIM3->CNT) + 1;
						}
					}
					if (TIM4->CNT > 1000)
										{
					//-------- CLEAR IN EDIT functions --------
					seqSetNote(currentChannel,
							cursorPosBeforeGatePage * 2 + TIM2->CNT, 0);
					gateStates[currentChannel][cursorPosBeforeGatePage * 2
							+ TIM2->CNT] = 0;

					// update regs to avoid errors after cleaning
					TIM3->CNT = seqGetNoteNumber(currentChannel,
							cursorPosBeforeGatePage * 2 + TIM2->CNT);
					TIM3->CCR3 = (TIM3->CNT) - 1;
					TIM3->CCR4 = (TIM3->CNT) + 1;
										}
				}

			}
		}

		EXTI->PR |= EXTI_PR_PR6; // clear pending bit
	}

	//********************************************** LEFT BUTTON **********************************************
	if (EXTI->PR & EXTI_PR_PR7)
	{
		//---------PUSH--------
		if ((GPIOB->IDR & GPIO_IDR_IDR7))
		{
			TIM4->CNT = 0;
		}
		//---------RELEASE--------
		if (!(GPIOB->IDR & GPIO_IDR_IDR7))
		{

			//--------- EDIT --------
			if (menuPage != 0)
			{

			}
			else
			{
				mainPage = !mainPage;
				gatePage = !gatePage;
				if (mainPage != 0)
				{
					TIM2->ARR = 255; // Count to
					TIM2->EGR |= TIM_EGR_UG; // generate an update to the registers
					TIM2->CNT = cursorPosBeforeGatePage;
				}
				if (gatePage != 0)
				{
					cursorPosBeforeGatePage = TIM2->CNT;
					TIM2->ARR = 1; 	// Count to
					TIM2->EGR |= TIM_EGR_UG; // generate an update to the registers
				}
			}
		}

		EXTI->PR |= EXTI_PR_PR7; // clear pending bit
	}

	//********************************************** COPY BUTTON **********************************************
	if (EXTI->PR & EXTI_PR_PR8)
	{
		//---------PUSH--------
		if (GPIOB->IDR & GPIO_IDR_IDR8)
		{
			if (mainPage != 0)
			{
				copyStartPosition = TIM2->CNT;
				copyingNow = 1;
			}
		}
		//---------RELEASE--------
		if (!(GPIOB->IDR & GPIO_IDR_IDR8))
		{
			if (mainPage != 0)
			{
				copyEndPosition = TIM2->CNT;
				if (copyStartPosition > copyEndPosition)
				{
					copyReverseRotate = 1;
				}
				else
				{
					copyReverseRotate = 0;
					copyStartPosition *= 2;
					copyEndPosition = copyEndPosition*2+2;
					copyNumCopy = (copyEndPosition - copyStartPosition);
					if (copyNumCopy <= 256)
					{
						seqCopy();
					}
				}
				copyingNow = 0;
			}
		}

		EXTI->PR |= EXTI_PR_PR8; // clear pending bit
	}

	//********************************************** PASTE BUTTON **********************************************
	if (EXTI->PR & EXTI_PR_PR9)
	{
		//---------PUSH--------
		if (GPIOB->IDR & GPIO_IDR_IDR9)
		{
			if (mainPage != 0)
			{


			}

		}
		//---------RELEASE--------
		if (!(GPIOB->IDR & GPIO_IDR_IDR9))
		{
			if (mainPage != 0)
			{
				if (copyReverseRotate != 1)
				{
					seqPaste ();
				}

			}
		}

		EXTI->PR |= EXTI_PR_PR9; // clear pending bit
	}

}

void EXTI15_10_IRQHandler(void)
{
	//*************************************** GATE button ***************************************
	if (EXTI->PR & EXTI_PR_PR14)
	{
		//--------- PUSH --------
		if (GPIOC->IDR & GPIO_IDR_IDR14)
		{
			if (mainPage != 0)
			{
				gateChangigngNow = 1;

				cursorPosBeforeShifting = TIM2->CNT;

				char cell1 = gateStates[currentChannel][TIM2->CNT * 2];
				char cell2 = gateStates[currentChannel][TIM2->CNT * 2 + 1];


				TIM2->ARR = 64; // Count to



				if (cell1 == 32)
				{
					TIM2->CNT = 32 + cell2;
				}
				else
				{
					TIM2->CNT = cell1;
				}

				TIM2->CCR3 = (TIM2->CNT) - 1;
				TIM2->CCR4 = (TIM2->CNT) + 1;
			}
		}
		//--------- RELEASE --------
		if (!(GPIOC->IDR & GPIO_IDR_IDR14))
		{
			if (mainPage != 0)
			{
				gateChangigngNow = 0;

				TIM2->ARR = 255; // Count to

				TIM2->CNT = cursorPosBeforeShifting;

				TIM2->CCR3 = (TIM2->CNT) - 1;
				TIM2->CCR4 = (TIM2->CNT) + 1;
			}
		}
		EXTI->PR |= EXTI_PR_PR14; // clear pending bit
	}

	//*************************************** MENU button ***************************************
	if (EXTI->PR & EXTI_PR_PR15)
	{

		//--------- PUSH --------
		if (GPIOC->IDR & GPIO_IDR_IDR15)
		{
			TIM4->CNT = 0;
		}

		//--------- RELEASE --------
		if (!(GPIOC->IDR & GPIO_IDR_IDR15))
		{
			if (TIM4->CNT > 75 && TIM4->CNT < 1000)
			{


				menuPage = !menuPage;
				if (menuPage != 0)
				{
					TIM2->ARR = 11; // Count to
					TIM2->EGR |= TIM_EGR_UG; // generate an update to the registers
					TIM2->CNT = 0;
					TIM2->CCR3 = (TIM2->CNT) - 1;
					TIM2->CCR4 = (TIM2->CNT) + 1;

					// first in menu is channel change page
					menuPageNum = 0;
					// update regs to avoid errors
					TIM3->ARR = 7; 				// Count from 1 to 8 channel
					TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
					TIM3->CNT = currentChannel;
					// update regs to avoid errors
					TIM3->CCR3 = (TIM3->CNT) - 1;
					TIM3->CCR4 = (TIM3->CNT) + 1;
				}
				else
				{
					// Back to main page
					gatePage = 0;
					mainPage = 255;
					TIM2->ARR = 255; // Count to
					TIM2->EGR |= TIM_EGR_UG; // generate an update to the registers
					TIM2->CNT = cursorPos;
					TIM2->CCR3 = (TIM2->CNT) - 1;
					TIM2->CCR4 = (TIM2->CNT) + 1;

					TIM3->ARR = 60; 			// Count from 0 to 60 note
					TIM3->EGR |= TIM_EGR_UG; 	// generate an update to the registers
					TIM3->CNT = seqGetNoteNumber(currentChannel, TIM2->CNT * 2);
					// update regs to avoid errors
					TIM3->CCR3 = (TIM3->CNT) - 1;
					TIM3->CCR4 = (TIM3->CNT) + 1;
				}
			}
		}

		EXTI->PR |= EXTI_PR_PR15; // clear pending bit
	}
}

void EXTI0_IRQHandler(void)
{
	if (EXTI->PR & EXTI_PR_PR0)
	{
		//--------- PUSH --------
		if (GPIOB->IDR & GPIO_IDR_IDR0)
		{
			recordPrepare = 1;

		}
		//--------- RELEASE --------
		if (!(GPIOB->IDR & GPIO_IDR_IDR0))
		{
			recordPrepare = 0;
			recordPermission = 1;
			gateCounter[currentChannel] = 0;
			positionArray[currentChannel] = 0;
		}
	}
	EXTI->PR |= EXTI_PR_PR0; // clear pending bit
}

void EXTI1_IRQHandler(void)
{
	//*************************************** SHIFT button ***************************************
	if (EXTI->PR & EXTI_PR_PR1)
	{
		//--------- PUSH --------
		if (GPIOB->IDR & GPIO_IDR_IDR1)
		{
			if (mainPage != 0)
			{
				cursorPosBeforeShifting = TIM2->CNT;
				shiftingNow = 1;

			}
		}
		//--------- RELEASE --------
		if (!(GPIOB->IDR & GPIO_IDR_IDR1))
		{
			if (mainPage != 0)
			{

				shiftingNow = 0;
				TIM2->CNT = cursorPosBeforeShifting;


				TIM2->CCR3 = (TIM2->CNT) - 1;
				TIM2->CCR4 = (TIM2->CNT) + 1;
			}
		}
	EXTI->PR |= EXTI_PR_PR1; // clear pending bit

	}
}

void seqShift(seq_t channel, char direction)
{
	seq_t length = lengthOfSeq[channel];

	if (direction == 1) // if direction is forward
	{
		seq_t lastSEQValue = sequence[channel][length];
		seq_t lastGATEValue = gateStates[channel][length];

		for (int var = length-1; -1 < var; --var)
		{
			sequence[channel][var+1] = sequence[channel][var];
			gateStates[channel][var+1] = gateStates[channel][var];
		}
		sequence[channel][0] = lastSEQValue;
		gateStates[channel][0] = lastGATEValue;
	}
	else
	{
		seq_t firstSEQValue = sequence[channel][0];
		seq_t firstGATEValue = gateStates[channel][0];
		for (int var = 0; var < length+1; ++var)
		{
			sequence[channel][var] = sequence[channel][var + 1];
			gateStates[channel][var] = gateStates[channel][var + 1];
		}
		sequence[channel][length] = firstSEQValue;
		gateStates[channel][length] = firstGATEValue;
	}
}

void seqCopy(void)
{
	// copying chosen data to buffer
	for (seq_t buffPos = 0, seqPos = copyStartPosition;
			(buffPos < copyNumCopy && seqPos < copyEndPosition);
			++buffPos, ++seqPos)
	{

		copySeqBuffer[buffPos] = seqGetValue(currentChannel, seqPos);
		copyGateBuffer[buffPos] = gateStates[currentChannel][seqPos];

		/* LAST
		copySeqBuffer[buffPos] = sequence[currentChannel][seqPos];
		copyGateBuffer[buffPos] = gateStates[currentChannel][seqPos];
		*/
	}

}
void seqPaste (void)
{
	// paste chosen data from buffer to SEQ
	for (seq_t buffPos = 0, seqPos = TIM2->CNT * 2;
			(buffPos < copyNumCopy && seqPos < copyNumCopy + seqPos);
			++buffPos, ++seqPos)
	{

		seqSetVal(currentChannel, seqPos, copySeqBuffer[buffPos]);
		gateStates[currentChannel][seqPos] = copyGateBuffer[buffPos];

		/* LAST
		sequence[currentChannel][seqPos] = copySeqBuffer[buffPos];
		gateStates[currentChannel][seqPos] = copyGateBuffer[buffPos];
		*/
	}
}

void seqSetGate (seq_t value, seq_t position)
{
	seq_t firstPositionInCell = (2 * position);
	if (value > 32)
	{
		gateStates[currentChannel][firstPositionInCell] = 32;
		gateStates[currentChannel][firstPositionInCell + 1] = value - 32;
	}
	else
	{
		gateStates[currentChannel][firstPositionInCell] = value;
		gateStates[currentChannel][firstPositionInCell + 1] = 0;
	}
}

void saveSeqValues ( uint32_t address)
{// 08007000, 0800A000, 0800D000

	uint32_t addr = address;
	uint32_t data = 0;
	// --------------- SAVE SEQUENCE VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{
		//address = address + (1024 * channel);
		for (seq_t position = 0; position < 512; position+=2)
		{
			data = sequence[channel][position];
			data |= sequence[channel][position + 1] << 16;

		flash_unlock();
			flash_write(addr, data);
		flash_lock();

			addr += 4;
		}
	}

// --------------- SAVE GATE VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{

		for (seq_t position = 0; position < 512; position+=4)
		{

			data = gateStates[channel][position];
			data |= gateStates[channel][position + 1] << 8;
			data |= gateStates[channel][position + 2] << 16;
			data |= gateStates[channel][position + 3] << 24;

		flash_unlock();
			flash_write(addr, data);
		flash_lock();

			addr += 4;
		}

	}

}

void eraseSeqValues (uint32_t address)
{
	// 08007000, 0800A000, 0800D000
	for (int add = 0; add < 12; ++add)
	{
		flash_unlock();
		flash_page_erase(address + (add * 1024));
		flash_lock();
	}
}

void loadSeqValues (uint32_t address)
{
	uint32_t addr = address;
	uint32_t data = 0;

	data = (*(__IO uint32_t*) addr);
	// --------------- LOAD SEQUENCE VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{
		for (seq_t position = 0; position < 512; position += 2)
		{
			sequence[channel][position] = data & 0xFFFF; // 0000data
			sequence[channel][position + 1] = data >> 16; // data0000 > 0000data

			addr += 4;
			data = (*(__IO uint32_t*) addr);
		}
	}
	// --------------- LOAD GATE VALUES ---------------------
		for (seq_t channel = 0; channel < 8; ++channel)
		{
			for (seq_t position = 0; position < 512; position+=4)
			{
				gateStates[channel][position] = data & 0xFF;
				gateStates[channel][position+1] = (data & 0xFF00) >> 8;
				gateStates[channel][position+2] = (data & 0xFF0000) >> 16;
				gateStates[channel][position+3] = (data & 0xFF000000) >> 24;

				addr += 4;
				data = (*(__IO uint32_t*) addr);
			}
		}
}

void saveSeqPref ( uint32_t address)
{// 08006C00, 08006800, 08006400
	uint32_t addr = address;
	uint32_t data = 0;

 	// --------------- SAVE DIRECTION VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{

		data = channelDirection[channel];

		flash_unlock();
		flash_write(addr, data);
		flash_lock();

		addr += 4;
	}
	// --------------- SAVE CV/CHANNEL VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{
		data = cvOnChannel[channel];

		flash_unlock();
		flash_write(addr, data);
		flash_lock();

		addr += 4;
	}
	// --------------- SAVE CV/STEP CHANNEL VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{
		data = cvStepOnChannel[channel];

		flash_unlock();
		flash_write(addr, data);
		flash_lock();
		addr += 4;
	}
	// --------------- SAVE RESET CHANNEL VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{

		data = resetOnChannel[channel];

		flash_unlock();
		flash_write(addr, data);
		flash_lock();

		addr += 4;
	}
	// --------------- SAVE DIVIDER CHANNEL VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{

		data = channelDivider[channel];

		flash_unlock();
		flash_write(addr, data);
		flash_lock();

		addr += 4;
	}
	// --------------- SAVE PAUSE CHANNEL VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{

		data = pauseOnChannelPermission[channel];

		flash_unlock();
		flash_write(addr, data);
		flash_lock();

		addr += 4;
	}
	// --------------- SAVE length CHANNEL VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{

		data = lengthOfSeq[channel];

		flash_unlock();
		flash_write(addr, data);
		flash_lock();

		addr += 4;
	}

	flash_unlock();
	flash_write(addr, generalBPM);
	flash_lock();

	addr += 4;

	flash_unlock();
	flash_write(addr, clockCounterV);
	flash_lock();
}

void loadSeqPef (uint32_t address)
{// 08006C00, 08006800, 08006400
	uint32_t addr = address;
	uint32_t data = 0;

	data = (*(__IO uint32_t*) addr);
	// --------------- LOAD SEQUENCE VALUES ---------------------
	for (seq_t channel = 0; channel < 8; ++channel)
	{
			channelDirection[channel] = data;
			addr += 4;
			data = (*(__IO uint32_t*) addr);
	}
	for (seq_t channel = 0; channel < 8; ++channel)
	{
			cvOnChannel[channel] = data;
			addr += 4;
			data = (*(__IO uint32_t*) addr);
	}
	for (seq_t channel = 0; channel < 8; ++channel)
	{
			cvStepOnChannel[channel] = data;
			addr += 4;
			data = (*(__IO uint32_t*) addr);
	}
	for (seq_t channel = 0; channel < 8; ++channel)
	{
			resetOnChannel[channel] = data;
			addr += 4;
			data = (*(__IO uint32_t*) addr);
	}
	for (seq_t channel = 0; channel < 8; ++channel)
	{
			channelDivider[channel] = data;
			addr += 4;
			data = (*(__IO uint32_t*) addr);
	}
	for (seq_t channel = 0; channel < 8; ++channel)
	{
			pauseOnChannelPermission[channel] = data;
			addr += 4;
			data = (*(__IO uint32_t*) addr);
	}
	for (seq_t channel = 0; channel < 8; ++channel)
	{
			lengthOfSeq[channel] = data;
			addr += 4;
			data = (*(__IO uint32_t*) addr);
	}
	generalBPM = data;
	addr += 4;
	data = (*(__IO uint32_t*) addr);
	clockCounterV = data;
}

void seqClearAllChannel (seq_t channel)
{
	for (int pos = 0; pos < 512; ++pos)
	{
		seqSetVal(channel, pos, 0);
		gateStates[channel][pos] = 0;
	}
	cvOnChannel[channel] = 1;
	cvStepOnChannel[channel] = 0;
	channelDivider[channel] = 1;
	lengthOfSeq[channel] = 15;
	channelDirection[channel] = 0;
	resetOnChannel[channel] = 1;
	pauseOnChannel[channel] = 0;
	pauseOnChannelPermission[channel] = 1;
}
