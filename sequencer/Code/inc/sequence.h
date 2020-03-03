

typedef unsigned short seq_t;
extern seq_t sequence[8][512];
extern seq_t lengthOfSeq[8];
extern seq_t positionArray[8];

void seqClearAllValues (seq_t channel);
void seqSetLength (seq_t channel, seq_t length);
void seqSetVal(seq_t channel, seq_t position, seq_t value);
void seqSetNote(seq_t channel, seq_t position, seq_t noteNumber);

seq_t seqGetValue (	seq_t channel,	seq_t position);
seq_t seqGetLength (seq_t channel);
seq_t seqGetData (seq_t channel, seq_t position);
seq_t seqGetEmptyValue (seq_t channel);
seq_t seqGetNoteNumber (seq_t channel, seq_t position);

