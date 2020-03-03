#include "sequence.h"

void seqSetVal(seq_t channel, seq_t position, seq_t value)
{
	sequence[channel][position] = (value | (channel << 12));
}

void seqClearAllValues(seq_t channel)
{
	for (int position = 0; position < 64; ++position)
	{
		sequence[channel][position] = (0 | (channel << 12));
	}
}

seq_t seqGetEmptyValue (seq_t channel)
{
	return (channel << 12);
}

seq_t seqGetValue(seq_t channel, seq_t position)
{
	return sequence[channel][position] & 4095; // &= 4095 cleans three channel bits
}

seq_t seqGetData(seq_t channel, seq_t position)
{
	return sequence[channel][position];
}

void seqSetLength(seq_t channel, seq_t length)
{
	lengthOfSeq[channel] = length;
}

seq_t seqGetLength(seq_t channel)
{
	return lengthOfSeq[channel];
}

void seqSetNote(seq_t channel, seq_t position, seq_t noteNumber)
{
	sequence[channel][position] = ((noteNumber*68) | (channel << 12));
}

seq_t seqGetNoteNumber (seq_t channel, seq_t position)
{
	return (sequence[channel][position] & 4095) / 68; // &= 4095 cleans three channel bits
}

