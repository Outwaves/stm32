#ifndef CODE_INC_LIBTIM1_H_
#define CODE_INC_LIBTIM1_H_

#include "stm32f1xx.h"
#include "libSPI.h"


typedef unsigned short seq_t;
extern unsigned char timerTemp; 			// temp position in timer

extern seq_t positionArray[8];
extern seq_t channelDivider[8];	// Array of dividers
extern seq_t channelDividerCounter[8]; // number of noteDivider repeats
extern unsigned char seqTrigger[8][64];	// Array of on\off values for playing notes (triggers)
extern char channelDirection[8];		// array of channel position counter direction ( == 0  forward, != 0 - backward)
extern short timerCounter;
extern char cvTemp;
extern seq_t cvBPM;
extern seq_t gateLength[8];			// Array of gate length
extern char gateStates[8][512];		// Array of on\off values for playing notes (trig/gate)
extern seq_t gateCounter[8];			//Array of gate on/of counters
extern char cvOnChannel[8];			// array of channel CV permission
extern seq_t gateTxNumber;
extern seq_t gateTxNumberLast;
extern char channelDirection[8];		// array of channel position counter direction
extern seq_t lcdTimerCounter;
extern seq_t cursorPos;

extern volatile seq_t clockOutCounter;
extern seq_t clockCounterV;


extern char recordPermission;
extern char pauseOnChannel[8];
extern char pauseOnChannelPermission[8];


void timerSetBPM (uint16_t bpm);
void TIM1_UP_IRQHandler (void);
void positionChange (seq_t channel, char direction);
void cvControl (void);

void cvStep(seq_t channel, seq_t position);
void recordCv (seq_t channel);

#endif
