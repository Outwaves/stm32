#include "libTIM1.h"
#include "sequence.h"
#include "lcd1602.h"
#include "libADC.h"

#define jackBPM GPIO_IDR_IDR10

void TIM1_UP_IRQHandler(void)
{
	if (TIM1->SR & TIM_SR_UIF)		// check interrupt by Update Interrupt
	{
		TIM1->SR &= ~TIM_SR_UIF;
		for (seq_t channel = 0; channel < 8; ++channel)
		{
			//------------TRANSMITTING-----------
			if ((channel == 7) && (gateTxNumberLast != gateTxNumber))
			{
				SPI1_Transmit_Leds(gateTxNumber);
				gateTxNumberLast = gateTxNumber;
			}
			if (gateCounter[channel] == 0)
			{
				uint16_t position = positionArray[channel];

				//------------GATES ON-OFF on EDGES-----------
				if ((gateStates[channel][position] != 0))
				{
					gateTxNumber |= (1 << channel);
				}
				else
				{
					gateTxNumber &= ~(1 << channel);
				}
				//------------ TRANSMITTING GATES -----------
				if (channel == 7)
				{
					SPI1_Transmit_Leds(gateTxNumber);
				}
				//------------ TRANSMITTING CV -----------
				ADC_start_injected();
				uint16_t adcValue = (4096- ADC1->JDR1);
				uint16_t seqValue = sequence[channel][position] & 4095;

				if (cvOnChannel[channel] != 0)
				{
					if (seqValue != 0)
					{

						if (adcValue + seqValue > 4095)
						{
							SPI1_Transmit((4095 | (channel << 12)));
						}
						else
						{
							SPI1_Transmit(sequence[channel][position] + adcValue);
						}
					}
					else
					{
						SPI1_Transmit(sequence[channel][position]);
					}
				}
				else
				{
					SPI1_Transmit(sequence[channel][position]);
				}
			}
			if ((gateCounter[channel] / channelDivider[channel])
					== gateStates[channel][positionArray[channel]])
			{
				gateTxNumber &= ~(1 << channel);
			}
		}
		/***************************** COUNTERS - POSITIONS******************************/
		for (seq_t channel = 0; channel < 8; ++channel)
		{
			cvStep(channel, positionArray[channel]);
			if (channel == currentChannel)
			{
				recordCv(currentChannel);
			}
			gateCounter[channel]++;
			if (gateCounter[channel] >= (32 * channelDivider[channel]))
			{
				gateCounter[channel] = 0;

				if (pauseOnChannel[channel] == 0)
				{
					positionChange(channel, channelDirection[channel]);
				}
			}
		}
		cvControl();

	if (clockOutCounter >= clockCounterV)
	{
		clockOutCounter = 0;
	}
	else
	{
		clockOutCounter++;
	}

	if (clockOutCounter >= (clockCounterV/2))
	{
		GPIOB->BSRR |= GPIO_BSRR_BR2;
	}
	else
	{
		GPIOB->BSRR |= GPIO_BSRR_BS2;
	}

	}


}


void positionChange (seq_t channel, char direction)
{
	if (cvStepOnChannel[channel] != 0)
	{

	}
	else
	{


		//------------positions FORWARD -----------
		if (direction == 0)
		{
			if (positionArray[channel] < lengthOfSeq[channel])
			{
				positionArray[channel]++;
			}
			else
			{
				positionArray[channel] = 0;

				if (positionArray[currentChannel] == 0)
				{
				recordPermission = 0;
				}
			}
		}
		else
		{
			//------------positions BACKWARD -----------
			if (positionArray[channel] == 0)
			{
				positionArray[channel] = lengthOfSeq[channel];
				if (positionArray[currentChannel] == lengthOfSeq[currentChannel])
				{
					recordPermission = 0;
				}
			}
			else
			{
				positionArray[channel]--;
			}
		}
	}
}


void cvControl (void)
{
	if (cvTemp == 4)
	{

		ADC_start_injected();
		uint16_t adcValue = (4096- ADC1->JDR1);

		for (seq_t channel = 0; channel < 8; ++channel)
		{
			uint16_t position = positionArray[channel];
			uint16_t seqValue = sequence[channel][position] & 4095;

			if (cvOnChannel[channel] != 0)
			{
				if (seqValue != 0)
				{
					if (adcValue + seqValue > 4095)
					{
						SPI1_Transmit((4095 | (channel << 12)));
					}
					else
					{
						SPI1_Transmit(sequence[channel][position] + adcValue);
					}
				}
				else
				{
					SPI1_Transmit(sequence[channel][position]);
				}
			}
			else
			{
				SPI1_Transmit(sequence[channel][position]);
			}

		}
		cvTemp = 0;

		if (!(jackBPM & GPIOA->IDR))
		{
			cvBPM = (4096 - ADC1->JDR2)/13;
			if (cvBPM <= 15) cvBPM = 15;
			if (cvBPM > 300) cvBPM = 300;

			timerSetBPM(cvBPM);
		}
		else
		{
			timerSetBPM(generalBPM);
		}

	}
	cvTemp++;
}



void timerSetBPM (uint16_t bpm)
{
	if (bpm <= 15) bpm = 15;
	if (bpm > 300) bpm = 300;
	TIM1->ARR = (1000000 / (bpm*8*32/60));
}

void cvStep(seq_t channel, seq_t position)
{

	if (cvStepOnChannel[channel] != 0)
	{

		seq_t step = (4096 - ADC1->JDR3) / (4096 / (lengthOfSeq[channel]));
		positionArray[channel] = step;

		uint16_t adcValue = (4096 - ADC1->JDR1);
		uint16_t seqValue = sequence[channel][position] & 4095;

		if (cvOnChannel[channel] != 0)
		{
			if (seqValue != 0)
			{

				if (adcValue + seqValue > 4095)
				{
					SPI1_Transmit((4095 | (channel << 12)));
				}
				else
				{
					SPI1_Transmit(sequence[channel][position] + adcValue);
				}
			}
			else
			{
				SPI1_Transmit(sequence[channel][position]);
			}
		}
		else
		{
			SPI1_Transmit(sequence[channel][position]);
		}
	}

}

void recordCv (seq_t channel)
{
	if (recordPermission != 0)
	{
		ADC_start_injected();
		uint16_t adcValue = (4096 - ADC1->JDR4);
		seqSetNote(channel, positionArray[currentChannel], adcValue / 69);

		if ((adcValue/69) > 0)
		{
			gateStates[currentChannel][positionArray[currentChannel]] = 32;
		}
		else
		{
			gateStates[currentChannel][positionArray[currentChannel]] = 0;
		}
	}
}
