#include <lcd1602.h>
#include <stm32f103xb.h>


#define jackBPM GPIO_IDR_IDR10

const char customSymbols[64] = {
		// Full
		0b00000,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b00000,
		// Empty
		0b00000,
		0b11111,
		0b10001,
		0b10001,
		0b10001,
		0b10001,
		0b11111,
		0b00000,
		// Cursor on full
		0b11111,
		0b10001,
		0b01110,
		0b01110,
		0b01110,
		0b01110,
		0b10001,
		0b11111,
		// Cursor on empty
		0b11111,
		0b10001,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b10001,
		0b11111,
		// Position on full
		0b11111,
		0b11111,
		0b10001,
		0b10001,
		0b10001,
		0b10001,
		0b11111,
		0b11111,
		// Cursor on Empty step
		0b11111,
		0b10001,
		0b00000,
		0b00100,
		0b00100,
		0b00000,
		0b10001,
		0b11111,
		// Empty step
		0b00000,
		0b00000,
		0b00000,
		0b00100,
		0b00100,
		0b00000,
		0b00000,
		0b00000,
		// Cursor on edited
		0b11111,
		0b10001,
		0b01010,
		0b00100,
		0b00100,
		0b01010,
		0b10001,
		0b11111
	};

//***************************** LOW LEVEL FUNCTIONS ****************************


void init_lcd(void)
{
	//**************************** GPIO INIT *********************************

	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;			// enable clock PORT B

	// GPIO Push-Pull, max speed 10 MHz

	GPIOB->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10_1 |
	GPIO_CRH_CNF11 | GPIO_CRH_MODE11_1 |
	GPIO_CRH_CNF12 | GPIO_CRH_MODE12_1 |
	GPIO_CRH_CNF13 | GPIO_CRH_MODE13_1 |
	GPIO_CRH_CNF14 | GPIO_CRH_MODE14_1 |
	GPIO_CRH_CNF15 | GPIO_CRH_MODE15_1);
	GPIOB->CRH |= (GPIO_CRH_MODE10_0 |
	GPIO_CRH_MODE11_0 |
	GPIO_CRH_MODE12_0 |
	GPIO_CRH_MODE13_0 |
	GPIO_CRH_MODE14_0 |
	GPIO_CRH_MODE15_0);

	//**************************** LCD INIT *********************************
	//for (int var = 0; var < 164000; ++var); // Delay ~38mS

	for (int var = 0; var < 22222; ++var)
		; 	// Delay ~15 mS (160000)
	lcdSendLSByte(0b00000011);
	for (int var = 0; var < 3888; ++var)
		;	// Delay ~4 mS (28000)
	lcdSendLSByte(0b00000011);
	for (int var = 0; var < 83; ++var)
		;	// Delay ~ 100 uS (600)
	lcdSendLSByte(0b00000011);
	for (int var = 0; var < 830; ++var)
		;	// Delay ~ 1 mS (6000)

	lcdSendLSByte(0b00000010);				// 4 Bit-mode

	for (int var = 0; var < 830; ++var)
		;			// Delay ~ 100 uS (6000)

	lcdSendFullByte(0b00101000, instruction);			// 2 lines 5x8 dots

	for (int var = 0; var < 83; ++var)
		;			// Delay ~ 100 uS (600)

	lcdSendFullByte(0b00001110, instruction);		// display on, cursor on, Blinking

	for (int var = 0; var < 83; ++var)
		;			// Delay ~ 100 uS (600)

	lcdSendFullByte(0b00000110, instruction);			// cursor increment

	for (int var = 0; var < 83; ++var)
		;			// Delay ~ 100 uS (600)
	lcdClear();
}

void lcdSendLSByte(unsigned char dataByte)
{
	unsigned char LSB = dataByte << 4;

	GPIOB->BSRR |= GPIO_BSRR_BS10;  			// Set E
	for (int var = 0; var < 42; ++var); 		// Delay ~ 40 uS (300)
	GPIOB->ODR &= ~(0xF000); 				    // Reset data pins

	GPIOB->ODR |= (LSB << 8);					// Set 4 MSB
	GPIOB->BSRR |= GPIO_BSRR_BR10; 				// reset E
	for (int var = 0; var < 42; ++var); 		// Delay ~ 40 uS (300)
}

void lcdSendFullByte(unsigned char data, LCD data_instruction)
{
	unsigned char temp = 0;
	if (data_instruction == 0)
	{
		GPIOB->BSRR |= GPIO_BSRR_BR11; 			// Reset RS
	}
	else
	{
		GPIOB->BSRR |= GPIO_BSRR_BS11; 			// Set RS
	}
	temp = (data >> 4); 						// 10101111 -> 00001010
	lcdSendLSByte(temp);						// Send MSB
	lcdSendLSByte(data);						// Send LSB
}

void lcdCursorSetPos(unsigned char x, unsigned char y)
{
	unsigned char address = 0;
	address = (0x40 * y + x) | 0b10000000;		// 2-line address starts on 0x40
	lcdSendFullByte(address, instruction);
}

void lcdSendString(const char string[]) {
	char xx = strlen(string);
	for (unsigned char pos = 0; pos < xx; pos++)
		lcdSendFullByte(string[pos], data);
}

void lcdClear(void) {
	lcdSendFullByte(0b00000001, instruction);
	for (int var = 0; var < 2777; ++var)
		;		// Delay ~ 2 mS (20000)
}

//***************************** CUSTOM SYMBOLS ****************************

void writeCustomSymb(void)
{
	SetCgramAddress(0x00);
	for (uint8_t i = 0; i < 64; i++)
	{
		lcdSendFullByte(customSymbols[i], data);
	}
	SetDdramAddress(0x00);
}

void SetDdramAddress(uint8_t address)
{
	lcdSendFullByte(address | 0x80, instruction);
	for (int var = 0; var < 83; ++var);			// Delay ~ 100 uS (600)
}

void SetCgramAddress(uint8_t address)
{
	lcdSendFullByte(address | 0x40, instruction);
	for (int var = 0; var < 83; ++var);			// Delay ~ 100 uS (600)
}


//***************************** SHOW FUNCTIONS ****************************

void showMainSequence (seq_t channel, seq_t cursorPosition)
{
	seq_t lengthSeq = lengthOfSeq[channel];
	seq_t position = positionArray[channel];
	lcdCursorSetPos(0, 0);
	/*********** check section 0-15 **********/
	char cursorSection = 0;
	if ((cursorPosition >= 0) && (cursorPosition < 16))
		cursorSection = 0;
	for (char pos = 0; pos < 16; ++pos)
	{
		if ((cursorPosition > (15 + (pos - 1) * 16))
				&& (cursorPosition < (16 * (pos + 1))))
		{
			cursorSection = pos;
		}
	}
	cursorPosition *= 2;
	/*********** sequence show section 0-127, 128-255, 256-383, 384-512 **********/
	for (int mainPosition = 0 + (32 * cursorSection);
			mainPosition < (32 + (32 * cursorSection)); mainPosition += 2)
	{
		if (mainPosition < lengthSeq)
		{
			char mainPositionEdited = 0;
			// if smallSquares != each other, then send '*'
			for (int smallPosition = mainPosition;
					smallPosition < 2 + mainPosition; ++smallPosition)
			{

				uint16_t seq1 = sequence[channel][mainPosition] & 4095;
				uint16_t seq2 = sequence[channel][smallPosition] & 4095;


				if (checkEdited(seq1, seq2,
						gateStates[channel][mainPosition], gateStates[channel][smallPosition]))
				{

					if (mainPosition == position
							|| mainPosition + 1 == position)
					{
						lcdSendFullByte(4, data); // position on FILLED 1
						smallPosition += 32; // to exit from cycle
						mainPositionEdited = 1; // edited section flag
					}
					else
					{
						if (cursorPosition == mainPosition)
						{
							lcdSendFullByte(7, data); // position on EDITED
							smallPosition += 32; // to exit from cycle
							mainPositionEdited = 1; // edited section flag
						}
						else
						{
							lcdSendFullByte('*', data);
							smallPosition += 32; // to exit from cycle
							mainPositionEdited = 1; // edited section flag
						}
					}
				}
			}
			if (mainPositionEdited == 0) // if this mainPosition not edited
			{
				//if FILLED GATE > 0, NOTE > 0
				if (gateStates[channel][mainPosition] > 0
						&& seqGetNoteNumber(channel, mainPosition) > 0)
				{
					if (mainPosition == position || mainPosition + 1 == position)
					{
						lcdSendFullByte(4, data); // position on FILLED 1
					}
					else if (cursorPosition == mainPosition)
					{
						lcdSendFullByte(2, data); // cursor on FILLED 1
					}
					else
					{
						lcdSendFullByte(0, data); // FILLED 1
					}
				}
				else //if EMPTY GATE = 0, NOTE = 0
				{
					if (mainPosition == position || mainPosition + 1 == position )
					{
						lcdSendFullByte(0xFF, data); // position on FILLED 0
					}
					else
					{
						if (cursorPosition == mainPosition)
						{
							lcdSendFullByte(3, data); // cursor on FILLED 0
						}
						else
						{
						lcdSendFullByte(1, data); // FILLED 0
						}
					}
				}
			}
		}
		else
		{
			if (cursorPosition == mainPosition)
			{
				lcdSendFullByte(5, data); // cursor on noSEQ
			}
			else
			{
				lcdSendFullByte(6, data); // noSEQ
			}
		}
	}

	lcdCursorSetPos(0, 1);
	if (copyingNow == 1)
	{
		lcdSendString("    Copying     ");
	}
	else if (shiftingNow == 1)
	{
		lcdSendString(" << Shifting >> ");
	}
	else if (recordPrepare != 0)
	{
		lcdCursorSetPos(0, 1);
		lcdSendString(" Release to REC ");
	}
	else if (recordPermission != 0)
	{
		lcdCursorSetPos(0, 1);
		lcdSendString("   RECORDING    ");
	}
	else if (gateChangigngNow == 1)
	{

		lcdSendString("Gate: ");

		char cell1 = gateStates[currentChannel][cursorPos * 2];
		char cell2 = gateStates[currentChannel][cursorPos * 2 + 1];
		char gate = 0;

		if (cell1 == 32)
		{
			gate = 32 + cell2;
		}
		else
		{
			gate = cell1;
		}
		lcdSendFullByte(0x30 + gate / 10, data);
		lcdSendFullByte(0x30 + gate % 10, data);
		lcdSendString("     ");
	}
	else
	{
		lcdSendFullByte(0x23, data);
		lcdSendFullByte(0x31 + channel, data);
		lcdSendFullByte(' ', data);
		lcdSendFullByte(4, data); // position on FILLED 1
		position += 2;
		lcdSendFullByte(0x30 + (position / 2) / 100, data);
		lcdSendFullByte(0x30 + (position / 2) % 100 / 10, data);
		lcdSendFullByte(0x30 + (position / 2) % 10, data);

		lcdSendFullByte(' ', data);
		lcdSendFullByte(3, data);
		cursorPosition += 2;
		lcdSendFullByte(0x30 + cursorPosition / 2 / 100, data);
		lcdSendFullByte(0x30 + cursorPosition / 2 % 100 / 10, data);
		lcdSendFullByte(0x30 + cursorPosition / 2 % 10, data);
		lcdSendFullByte(' ', data);
	}


}

char checkEdited (uint16_t seqVal1, uint16_t seqVal2, uint16_t gateVal1, uint16_t gateVal2)
{
	if (seqVal1 == seqVal2)
	{
		if (gateVal1 == gateVal2)
		{
			if (gateVal1 == 32)
			{
				if (seqVal1 == 0)
				{
					return 1;
				}
				else
					return 0;
			}
			else
			{
				if (gateVal1 == 0)
				{
					return 0;
				}
				else return 1;
			}
		}
		else
			return 1;
	}
	else
		return 1;

}

void showNote (seq_t channel, seq_t cursorPosition)
{
//	lcdCursorSetPos(13, 1);

	if (shiftingNow == 1)
	{
		lcdCursorSetPos(0, 1);
		lcdSendString(" << Shifting >> ");
	}
	else if (gateChangigngNow == 1)
	{
		lcdCursorSetPos(0, 1);
		lcdSendString("Gate: ");

		char cell1 = gateStates[currentChannel][cursorPos * 2];
		char cell2 = gateStates[currentChannel][cursorPos * 2 + 1];
		char gate = 0;

		if (cell1 == 32)
		{
			gate = 32 + cell2;
		}
		else
		{
			gate = cell1;
		}

		lcdSendFullByte(0x30 + gate / 10, data);
		lcdSendFullByte(0x30 + gate % 10, data);
		lcdSendString("        ");
	}
	else if (recordPrepare != 0)
	{
		lcdCursorSetPos(0, 1);
		lcdSendString(" Release to REC ");
	}
	else if (recordPermission != 0)
	{
		lcdCursorSetPos(0, 1);
		lcdSendString("   RECORDING    ");
	}
	else if (copyingNow != 1)
	{

		seq_t note = seqGetNoteNumber(channel, cursorPosition * 2);

		if (note == 0)
		{
			lcdSendFullByte('N', data);
			lcdSendFullByte('/', data);
			lcdSendFullByte('S', data);
		}
		else
		{
			char octave = note / 12;
			char noteNum = note % 12;

			switch (noteNum)
			{
			case 0:
				lcdSendFullByte(' ', data);
				lcdSendFullByte('B', data);
				break;
			case 11:
				lcdSendFullByte('A', data);
				lcdSendFullByte(0x23, data);
				break;
			case 10:
				lcdSendFullByte(' ', data);
				lcdSendFullByte('A', data);
				break;
			case 9:
				lcdSendFullByte('G', data);
				lcdSendFullByte(0x23, data);
				break;
			case 8:
				lcdSendFullByte(' ', data);
				lcdSendFullByte('G', data);
				break;
			case 7:
				lcdSendFullByte('F', data);
				lcdSendFullByte(0x23, data);
				break;
			case 6:
				lcdSendFullByte(' ', data);
				lcdSendFullByte('F', data);
				break;
			case 5:
				lcdSendFullByte(' ', data);
				lcdSendFullByte('E', data);
				break;
			case 4:
				lcdSendFullByte('D', data);
				lcdSendFullByte(0x23, data);
				break;
			case 3:
				lcdSendFullByte(' ', data);
				lcdSendFullByte('D', data);
				break;
			case 2:
				lcdSendFullByte('C', data);
				lcdSendFullByte(0x23, data);
				break;
			case 1:
				lcdSendFullByte(' ', data);
				lcdSendFullByte('C', data);
				break;

			}
			if (noteNum == 0)
			{
				lcdSendFullByte(0x30 + octave - 1, data);
			}
			else
			{
				lcdSendFullByte(0x30 + octave, data);
			}

		}
	}
}

void gatePageShow (seq_t channel, seq_t entryPosition, seq_t cursorPosition)
{
	lcdCursorSetPos(0, 0);
	lcdSendFullByte(' ', data);
	lcdSendFullByte(' ', data);
	lcdSendFullByte(' ', data);
	for (int currentSmallPos = 0 + entryPosition*2;
			currentSmallPos < (entryPosition*2+2); ++currentSmallPos)
	{
	//if FILLED GATE > 0, NOTE > 0
	if (gateStates[channel][currentSmallPos] > 0
			|| seqGetNoteNumber(channel, currentSmallPos) > 0)
	{
		if (cursorPosition == currentSmallPos-2*entryPosition)
		{
			lcdSendFullByte(2, data); // cursor on FILLED 1
		}
		else
		{
			lcdSendFullByte(0, data); // FILLED 1
		}
	}
	else //if EMPTY GATE = 0, NOTE = 0
	{
			if (cursorPosition == currentSmallPos-2*entryPosition)
			{
				lcdSendFullByte(3, data); // cursor on FILLED 0
			}
			else
			{
				lcdSendFullByte(1, data); // FILLED 0
			}
	}
	}
	lcdSendFullByte(' ', data);
	lcdSendFullByte(' ', data);
	if (cvChanging != 0)
	{
		lcdSendFullByte(0x7E, data);
	}
	else
	{
		lcdSendFullByte(' ', data);
	}
	lcdSendString("CV:  ");

	lcdCursorSetPos(0, 1);
	for (int var = 0; var < 7; ++var) {
		lcdSendFullByte(' ', data);
	}
	if (gateChanging != 0)
	{
		lcdSendFullByte(0x7E, data);
	}
	else
	{
		lcdSendFullByte(' ', data);
	}
	lcdSendString("Gate:");
	lcdSendFullByte(' ', data);
	lcdSendFullByte(0x30 + gateStates[channel][entryPosition*2+cursorPosition]/10, data);
	lcdSendFullByte(0x30 + gateStates[channel][entryPosition*2+cursorPosition]%10, data);

}

void showNoteGatePage (seq_t channel, seq_t entryPosition, seq_t cursorPosition)
{
	lcdCursorSetPos(13, 0);
	seq_t note = seqGetNoteNumber(channel, entryPosition*2+cursorPosition);
	if (note == 0)
	{
		lcdSendFullByte('N', data);
		lcdSendFullByte('/', data);
		lcdSendFullByte('S', data);
	}
	else
	{
		char octave = note / 12;
		char noteNum = note % 12;

		switch (noteNum)
		{
		case 0:
			lcdSendFullByte(' ', data);
			lcdSendFullByte('B', data);
			break;
		case 11:
			lcdSendFullByte('A', data);
			lcdSendFullByte(0x23, data);
			break;
		case 10:
			lcdSendFullByte(' ', data);
			lcdSendFullByte('A', data);
			break;
		case 9:
			lcdSendFullByte('G', data);
			lcdSendFullByte(0x23, data);
			break;
		case 8:
			lcdSendFullByte(' ', data);
			lcdSendFullByte('G', data);
			break;
		case 7:
			lcdSendFullByte('F', data);
			lcdSendFullByte(0x23, data);
			break;
		case 6:
			lcdSendFullByte(' ', data);
			lcdSendFullByte('F', data);
			break;
		case 5:
			lcdSendFullByte(' ', data);
			lcdSendFullByte('E', data);
			break;
		case 4:
			lcdSendFullByte('D', data);
			lcdSendFullByte(0x23, data);
			break;
		case 3:
			lcdSendFullByte(' ', data);
			lcdSendFullByte('D', data);
			break;
		case 2:
			lcdSendFullByte('C', data);
			lcdSendFullByte(0x23, data);
			break;
		case 1:
			lcdSendFullByte(' ', data);
			lcdSendFullByte('C', data);
			break;
		}

		if (noteNum == 0)
		{
		lcdSendFullByte(0x30+octave-1, data);
		}
		else
		{
			lcdSendFullByte(0x30+octave, data);
		}
	}
}

void showMenuPage(seq_t channel, char rightEncoderValue)
{

	if (rightEncoderValue == 0)
	{
		lcdCursorSetPos(0, 0);
		lcdSendString("Current channel:");
		lcdCursorSetPos(0, 1);
		lcdSendString("       ");
		lcdSendFullByte(0x31 + channel, data);
		lcdSendString("        ");
	}

	if (rightEncoderValue == 1)
	{
			lcdCursorSetPos(0, 0);
		// when jack plugged
		if (!(jackBPM & GPIOA->IDR))
		{
			lcdSendString("CV / BPM:       ");
			lcdCursorSetPos(0, 1);
			lcdSendString("      ");
			lcdSendFullByte(0x30 + cvBPM / 100, data);
			lcdSendFullByte(0x30 + cvBPM % 100 / 10, data);
			lcdSendFullByte(0x30 + cvBPM % 10, data);
			lcdSendString("       ");
		}
		else
		{
			lcdSendString("General BPM:    ");
			lcdCursorSetPos(0, 1);
			lcdSendString("      ");
			lcdSendFullByte(0x30 + generalBPM / 100, data);
			lcdSendFullByte(0x30 + generalBPM % 100 / 10, data);
			lcdSendFullByte(0x30 + generalBPM % 10, data);
			lcdSendString("       ");
		}
	}

	if (rightEncoderValue == 2)
	{
		lcdCursorSetPos(0, 0);
		lcdSendString("Period multipl. ");
		lcdCursorSetPos(0, 1);
		lcdSendString("       ");
		lcdSendFullByte(0x30 + channelDivider[channel] % 100 / 10, data);
		lcdSendFullByte(0x30 + channelDivider[channel] % 10, data);
		lcdSendString("       ");
	}

	if (rightEncoderValue == 3)
	{
		lcdCursorSetPos(0, 0);
		lcdSendString("Direction:      ");
		lcdCursorSetPos(0, 1);
		if (channelDirection[channel] == 0)
		{
			lcdSendString("     Forward    ");
		}
		else
		{
			lcdSendString("     Backward   ");
		}
	}

	if (rightEncoderValue == 4)
	{
		lcdCursorSetPos(0, 0);
		lcdSendString("CV control:     ");
		lcdCursorSetPos(0, 1);
		if (cvOnChannel[channel] == 0)
		{
			lcdSendString("       No       ");
		}
		else
		{
			lcdSendString("       Yes      ");
		}
	}

	if (rightEncoderValue == 5)
	{
		lcdCursorSetPos(0, 0);
		lcdSendString("CV/step:        ");
		lcdCursorSetPos(0, 1);
		if (cvStepOnChannel[channel] == 0)
		{
			lcdSendString("       No       ");
		}
		else
		{
			lcdSendString("       Yes      ");
		}
	}

	if (rightEncoderValue == 6)
	{
		lcdCursorSetPos(0, 0);
		lcdSendString("Reset on channel");
		lcdCursorSetPos(0, 1);
		if (resetOnChannel[channel] == 0)
		{
			lcdSendString("       No       ");
		}
		else
		{
			lcdSendString("       Yes      ");
		}
	}

	if (rightEncoderValue == 7)
	{
		lcdCursorSetPos(0, 0);
		lcdSendString("Pause on channel");
		lcdCursorSetPos(0, 1);
		if (pauseOnChannelPermission[channel] == 0)
		{
			lcdSendString("       No       ");
		}
		else
		{
			lcdSendString("       Yes      ");
		}
	}



	if (rightEncoderValue == 8)
	{
		if (savingNow != 1)
		{
			lcdCursorSetPos(0, 0);
			lcdSendString("Save to project:");
			lcdCursorSetPos(0, 1);
			lcdSendString("        ");
			lcdSendFullByte(0x31 + TIM3->CNT, data);
			lcdSendString("         ");
		}
		if (savingNow == 1)
			{
				lcdCursorSetPos(0, 0);
				lcdSendString("Saving...       ");
			}
	}

	if (rightEncoderValue == 9)
	{
		if (loadingNow != 1)
		{
			lcdCursorSetPos(0, 0);
			lcdSendString("Load project:   ");
			lcdCursorSetPos(0, 1);
			lcdSendString("        ");
			lcdSendFullByte(0x31 + TIM3->CNT, data);
			lcdSendString("         ");
		}
		if (loadingNow == 1)
			{
				lcdCursorSetPos(0, 0);
				lcdSendString("Loading...      ");
			}
	}

	if (rightEncoderValue == 10)
	{
		lcdCursorSetPos(0, 0);
		lcdSendString("CLEAR ");
		lcdSendFullByte(0x31 + TIM3->CNT, data);
		lcdSendString(" Channel?");
		lcdCursorSetPos(0, 1);
		lcdSendString("                ");
	}

	if (rightEncoderValue == 11)
	{
		lcdCursorSetPos(0, 0);
		lcdSendString("Clock out:      ");
		lcdCursorSetPos(0, 1);
		switch (clockType) {
			case 0:
				lcdSendString("      1/64      ");
				break;
			case 1:
				lcdSendString("      1/32      ");
				break;
			case 2:
				lcdSendString("      1/16      ");
				break;
			case 3:
				lcdSendString("      1/8       ");
				break;
			case 4:
				lcdSendString("      1/4       ");
				break;
			case 5:
				lcdSendString("      1/2       ");
				break;
			case 6:
				lcdSendString("      1/1       ");
				break;
		}
	}


}

void testADC (uint16_t adcValue1, uint16_t adcValue2, uint16_t adcValue3, uint16_t adcValue4, uint16_t adcValue5)
{
	lcdCursorSetPos(0,0);
	lcdSendFullByte(0x30 + adcValue1 / 1000, data);
	lcdSendFullByte(0x30 + adcValue1 % 1000 /100, data);
	lcdSendFullByte(0x30 + adcValue1 % 100 / 10, data);
	lcdSendFullByte(0x30 + adcValue1 % 10, data);
	lcdSendFullByte(' ', data);
	lcdSendFullByte(0x30 + adcValue2 / 1000, data);
	lcdSendFullByte(0x30 + adcValue2 % 1000 /100, data);
	lcdSendFullByte(0x30 + adcValue2 % 100 / 10, data);
	lcdSendFullByte(0x30 + adcValue2 % 10, data);
	lcdCursorSetPos(0,1);
		lcdSendFullByte(0x30 + adcValue3 / 1000, data);
		lcdSendFullByte(0x30 + adcValue3 % 1000 /100, data);
		lcdSendFullByte(0x30 + adcValue3 % 100 / 10, data);
		lcdSendFullByte(0x30 + adcValue3 % 10, data);
		lcdSendFullByte(' ', data);
		lcdSendFullByte(0x30 + adcValue4 / 1000, data);
		lcdSendFullByte(0x30 + adcValue4 % 1000 /100, data);
		lcdSendFullByte(0x30 + adcValue4 % 100 / 10, data);
		lcdSendFullByte(0x30 + adcValue4 % 10, data);
		lcdSendFullByte(' ', data);
	lcdSendFullByte(0x30 + adcValue5 / 1000, data);
	lcdSendFullByte(0x30 + adcValue5 % 1000 / 100, data);
	lcdSendFullByte(0x30 + adcValue5 % 100 / 10, data);
	lcdSendFullByte(0x30 + adcValue5 % 10, data);
}
