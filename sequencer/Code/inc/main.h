
#include "stm32f1xx.h"

#include "init.h"
#include "libSPI.h"
#include "sequence.h"
#include "lcd1602.h"
#include "libTIM1.h"
#include "libADC.h"

void seqShift(seq_t channel, char direction);
void seqPaste (void);
void seqCopy (void);
void seqSetGate (seq_t value, seq_t position);
void saveSeqValues ( uint32_t address);
void saveSeqPref ( uint32_t address);
void eraseSeqValues (uint32_t address);
void loadSeqValues (uint32_t address);
void loadSeqPef (uint32_t address);
void seqClearAllChannel (seq_t channel);
