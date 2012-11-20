//__LIBRETRO__: Ditch WAV and AVI support, spu rate adjust

/*
	Copyright 2006 Theo Berkau
	Copyright (C) 2006-2010 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SPU_H
#define SPU_H

#include <iosfwd>
#include <string>
#include <assert.h>
#include "types.h"
#include "matrix.h"
#include "emufile.h"

extern void frontend_process_samples(u32 frames, const s16* data);

#define SNDCORE_DEFAULT         -1
#define SNDCORE_DUMMY           0

#define CHANSTAT_STOPPED          0
#define CHANSTAT_PLAY             1

//who made these static? theyre used in multiple places.
FORCEINLINE u32 sputrunc(float f) { return u32floor(f); }
FORCEINLINE u32 sputrunc(double d) { return u32floor(d); }
FORCEINLINE s32 spumuldiv7(s32 val, u8 multiplier) {
	assert(multiplier <= 127);
	return (multiplier == 127) ? val : ((val * multiplier) >> 7);
}

enum SPUInterpolationMode
{
	SPUInterpolation_None = 0,
	SPUInterpolation_Linear = 1,
	SPUInterpolation_Cosine = 2
};

struct channel_struct
{
	channel_struct()
	{}
	u32 num;
   u8 vol;
   u8 datashift;
   u8 hold;
   u8 pan;
   u8 waveduty;
   u8 repeat;
   u8 format;
   u8 keyon;
   u8 status;
   u32 addr;
   u16 timer;
   u16 loopstart;
   u32 length;
   u32 totlength;
   double double_totlength_shifted;
   double sampcnt;
   double sampinc;
   // ADPCM specific
   u32 lastsampcnt;
   s16 pcm16b, pcm16b_last;
   s16 loop_pcm16b;
   int index;
   int loop_index;
   u16 x;
   s16 psgnoise_last;
};

class SPUFifo
{
public:
	SPUFifo();
	void enqueue(s16 val);
	s16 dequeue();
	s16 buffer[16];
	s32 head,tail,size;
	void save(EMUFILE* fp);
	bool load(EMUFILE* fp);
	void reset();
};

class SPU_struct
{
public:
	SPU_struct(int buffersize);
   u32 bufpos;
   u32 buflength;
   s32 *sndbuf;
   s32 lastdata; //the last sample that a channel generated
   s16 *outbuf;
   u32 bufsize;
   channel_struct channels[16];

   //registers
   struct REGS {
	   REGS()
			: mastervol(0)
			, ctl_left(0)
			, ctl_right(0)
			, ctl_ch1bypass(0)
			, ctl_ch3bypass(0)
			, masteren(0)
			, soundbias(0)
	   {}

	   u8 mastervol;
	   u8 ctl_left, ctl_right;
	   u8 ctl_ch1bypass, ctl_ch3bypass;
	   u8 masteren;
	   u16 soundbias;

	   enum LeftOutputMode
	   {
		   LOM_LEFT_MIXER=0, LOM_CH1=1, LOM_CH3=2, LOM_CH1_PLUS_CH3=3
	   };

	   enum RightOutputMode
	   {
		   ROM_RIGHT_MIXER=0, ROM_CH1=1, ROM_CH3=2, ROM_CH1_PLUS_CH3=3
	   };

	   struct CAP {
		   CAP()
			   : add(0), source(0), oneshot(0), bits8(0), active(0), dad(0), len(0)
		   {}
		   u8 add, source, oneshot, bits8, active;
		   u32 dad;
		   u16 len;
		   struct Runtime {
			   Runtime()
				   : running(0), curdad(0), maxdad(0)
			   {}
			   u8 running;
			   u32 curdad;
			   u32 maxdad;
			   double sampcnt;
			   SPUFifo fifo;
		   } runtime;
	   } cap[2];
   } regs;

   void reset();
   ~SPU_struct();
   void KeyOff(int channel);
   void KeyOn(int channel);
   void KeyProbe(int channel);
   void ProbeCapture(int which);
   void WriteByte(u32 addr, u8 val);
   u8 ReadByte(u32 addr);
   u16 ReadWord(u32 addr);
   u32 ReadLong(u32 addr);
   void WriteWord(u32 addr, u16 val);
   void WriteLong(u32 addr, u32 val);
   
   //kills all channels but leaves SPU otherwise running normally
   void ShutUp();
};

int SPU_Init();
void SPU_Reset(void);
void SPU_KeyOn(int channel);
void SPU_WriteByte(u32 addr, u8 val);
void SPU_WriteWord(u32 addr, u16 val);
void SPU_WriteLong(u32 addr, u32 val);
u8 SPU_ReadByte(u32 addr);
u16 SPU_ReadWord(u32 addr);
u32 SPU_ReadLong(u32 addr);
void SPU_Emulate_core(void);

extern int spu_core_samples;

void spu_savestate(EMUFILE* os);
bool spu_loadstate(EMUFILE* is, int size);

// we should make this configurable eventually
// but at least defining it somewhere is probably a step in the right direction
#define DESMUME_SAMPLE_RATE 44100
//#define DESMUME_SAMPLE_RATE 48000

#endif
