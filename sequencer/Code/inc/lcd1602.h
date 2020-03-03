#ifndef CODE_INC_LCD1602_H_
#define CODE_INC_LCD1602_H_

#include "stm32f1xx.h"
#include "string.h"
#include "sequence.h"
typedef unsigned short seq_t;
typedef enum txLCD {
	instruction = 0, data = 1
} LCD;

extern seq_t cursorPos;

extern seq_t sequence[8][512];
extern seq_t lengthOfSeq[8];
extern seq_t channelDivider[8];		// Array of dividers

extern seq_t cvBPM;
extern seq_t generalBPM;
extern seq_t positionArray[8];			// Array of current position on channel
extern seq_t cursorPosition;
extern seq_t currentChannel;
extern char gateStates[8][512];

extern unsigned char cvChanging;
extern unsigned char gateChanging;
extern char channelDirection[8];		// array of channel position counter direction ( == 0  forward, != 0 - backward)
extern char cvOnChannel[8];			// array of channel CV permission (== 0 - no CV, != 0 - CV)
extern char cvStepOnChannel[8];		// array of channel CV/step permission (== 0 - no CV, != 0 - CV)

extern char copyingNow;	// for LCD - while copying
extern char shiftingNow; // for LCD - while shifting
extern char gateChangigngNow; // for LCD - while edit gate
extern char savingNow;
extern char loadingNow;
extern char clockType;



extern char recordPermission;
extern char recordPrepare;
extern char pauseOnChannelPermission[8];
extern char resetOnChannel[8];

//************** LOW LEVEL FUNCTIONS ***************
void init_lcd (void);
void lcdSendLSByte(unsigned char dataByte);
void lcdSendFullByte(unsigned char data, LCD data_instruction);
void lcdCursorSetPos(unsigned char x, unsigned char y);
void lcdSendString(const char string[]);
void lcdClear(void);

//************** SHOW CUSTOM SYMBOLS FUNCTIONS ***************
void SetCgramAddress (uint8_t address);
void SetDdramAddress (uint8_t address);
void writeCustomSymb (void);

//******************** SHOW FUNCTIONS ************************
void showMainSequence (seq_t channel, seq_t cursorPosition);
void showNote (seq_t channel, seq_t cursorPosition);

char checkEdited (uint16_t seqVal1, uint16_t seqVal2, uint16_t gateVal1, uint16_t gateVal2);

void gatePageShow (seq_t channel, seq_t entryPosition, seq_t cursorPosition);

void showNoteGatePage (seq_t channel, seq_t entryPosition, seq_t cursorPosition);
void showMenuPage(seq_t channel, char rightEncoderValue);

void testADC (uint16_t adcValue1, uint16_t adcValue2, uint16_t adcValue3, uint16_t adcValue4, uint16_t adcValue5);
#endif
