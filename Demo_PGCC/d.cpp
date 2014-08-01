#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <aygshell.h>
#include <math.h>
#include "t.h"
//#include "m.h"
#define _Bool	bool
#define numofbuf	      	10
#define echobuf		7
#define bits		16
#define stereo		1
WAVEFORMATEX wavefmtx, pwfx;
HWAVEOUT hWaveOut;
WAVEHDR WaveHeader, out[numofbuf];
char *b1, version = 4;
int bpm = 150;
int frequency = 22000;
#define  maxseq 53
#define maxinstr 6
char patterns[][16] = { {33, 0, 29, 0, 33, 0, 29, 0, 32, 39, 26, 38, 0, 26, 0, 0}, {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9}, {9, 9, 9, 9, 13, 13, 13, 13, 15, 15, 15, 15, 16, 17, 17, 17}, {19, 19, 19, 19, 19, 20, 20, 20, 21, 21, 21, 21, 23, 23, 23, 23}, {23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 22, 22, 23, 23}, {24, 24, 24, 24, 27, 27, 27, 27, 28, 30, 31, 32, 34, 34, 34, 34}, {35, 35, 35, 35, 36, 37, 38, 39, 41, 41, 41, 43, 44, 45, 48, 49}, {68, 68, 68, 68, 69, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70}, {79, 80, 80, 80, 80, 80, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81}, {33, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 32, 0, 32, 0, 0}, {0, 0, 118, 0, 0, 0, 118, 0, 0, 118, 118, 0, 0, 0, 118, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, {26, 38, 26, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {21, 0, 21, 21, 0, 21, 21, 0, 21, 21, 21, 21, 21, 21, 21, 21}, {33, 0, 0, 0, 33, 0, 0, 0, 33, 0, 26, 0, 38, 0, 26, 0}, {33, 36, 33, 36, 33, 36, 33, 36, 37, 39, 37, 39, 37, 39, 37, 39}, {35, 31, 35, 31, 35, 31, 35, 31, 32, 27, 32, 27, 32, 27, 32, 27}, {24, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0}, {39, 0, 43, 0, 0, 43, 0, 0, 0, 39, 36, 0, 37, 0, 39, 0}, {39, 0, 43, 0, 0, 42, 0, 0, 38, 0, 39, 0, 43, 0, 44, 0}, {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 20, 20, 20, 20} };
char seq[][9] = { {1, 2, 10, 11, 0, 0, 0, 0, 2}, {1, 2, 10, 11, 0, 0, 0, 0, 2}, {1, 3, 10, 11, 0, 0, 0, 0, 2}, {1, 4, 10, 11, 0, 0, 0, 0, 2}, {1, 5, 10, 11, 0, 0, 0, 0, 2}, {1, 5, 10, 11, 0, 0, 0, 0, 2}, {1, 6, 10, 11, 0, 0, 0, 0, 2}, {1, 7, 10, 11, 0, 0, 0, 0, 2}, {1, 8, 10, 11, 12, 0, 0, 0, 21}, {1, 8, 10, 11, 12, 0, 0, 0, 21}, {1, 9, 10, 11, 12, 0, 0, 0, 21}, {1, 9, 10, 11, 12, 0, 0, 0, 21}, {13, 9, 14, 14, 12, 0, 0, 0, 21}, {15, 0, 0, 0, 0, 16, 0, 0, 2}, {15, 8, 0, 0, 0, 17, 0, 0, 2}, {15, 0, 0, 0, 0, 16, 0, 0, 2}, {15, 0, 10, 0, 12, 17, 0, 0, 21}, {15, 0, 0, 11, 0, 16, 0, 0, 21}, {15, 0, 10, 11, 0, 17, 0, 0, 2}, {15, 0, 10, 11, 0, 16, 2, 0, 2}, {15, 0, 10, 11, 0, 17, 3, 0, 2}, {15, 0, 0, 0, 0, 16, 4, 0, 2}, {15, 0, 0, 0, 0, 17, 5, 0, 2}, {15, 0, 0, 0, 0, 16, 6, 0, 2}, {15, 6, 0, 0, 0, 17, 7, 0, 2}, {15, 7, 0, 0, 0, 16, 8, 0, 2}, {15, 8, 0, 0, 0, 17, 9, 0, 2}, {15, 9, 0, 0, 0, 16, 0, 0, 2}, {15, 9, 0, 0, 0, 17, 0, 0, 2}, {0, 0, 18, 11, 0, 0, 0, 0, 2}, {0, 0, 18, 11, 0, 0, 0, 0, 2}, {0, 0, 18, 11, 0, 15, 0, 0, 2}, {0, 0, 18, 11, 0, 15, 0, 0, 2}, {0, 0, 16, 16, 0, 15, 0, 0, 2}, {0, 0, 17, 17, 0, 15, 0, 0, 2}, {0, 0, 0, 0, 0, 0, 0, 19, 2}, {0, 0, 0, 0, 0, 0, 0, 20, 2}, {0, 0, 0, 0, 0, 0, 0, 19, 2}, {0, 0, 0, 0, 0, 0, 0, 20, 2}, {0, 0, 0, 0, 0, 0, 0, 19, 2}, {20, 0, 0, 0, 0, 0, 0, 20, 2}, {19, 0, 0, 0, 0, 0, 0, 19, 2}, {16, 0, 0, 0, 0, 0, 0, 20, 2}, {17, 0, 0, 0, 0, 0, 0, 0, 2}, {16, 0, 0, 0, 0, 0, 0, 0, 2}, {17, 0, 0, 0, 0, 0, 0, 0, 2}, {16, 8, 0, 0, 0, 0, 0, 0, 2}, {17, 7, 0, 0, 0, 0, 0, 0, 2}, {16, 6, 0, 0, 0, 0, 0, 0, 2}, {17, 5, 0, 0, 0, 0, 0, 0, 2}, {16, 2, 10, 0, 0, 0, 0, 0, 2}, {17, 2, 10, 0, 0, 0, 0, 0, 21}, {16, 2, 10, 0, 0, 0, 0, 0, 21}, {17, 2, 10, 0, 0, 0, 0, 0, 21} };
char instr[][16] = { {2, -1, 0, 0, 11, 15, 4, 11, 21, 9, 53, 4, 101, -2, 116, 0}, {-5, -3, 9, 126, 47, -9, 0, 0, 21, 8, 0, 0, 0, 0, 0, 0}, {3, -4, 0, 126, 11, 3, 0, 54, 71, 0, 2, 2, 126, 0, 126, 0}, {2, -6, 0, 114, 12, 20, 0, 14, 31, -7, 12, 3, 101, -7, 101, 0}, {2, -8, 0, 126, 13, 24, 0, 0, 11, 7, 0, 0, 0, 115, 110, 0}, {2, -8, 0, 5, 22, 14, 0, 0, 12, 0, 0, 0, 0, 112, 112, 0} };



int beat, maxsamples = 0, editinstr = 0, playseq = 0, maxpatnames, notenum, maxtrack;
signed char editinst[16];
HWND hwnd;
short *reverbuf;
ULONG BufferSize, fulb, fillpoint, beatlen;
int indx, fillcell, playcell, erasecell, played;
#define GETRAWFRAMEBUFFER 0x00020001
#define FORMAT_565 1
#define FORMAT_555 2
#define FORMAT_OTHER 3
typedef struct _RawFrameBufferInfo {
	WORD wFormat;
	WORD wBPP;
	VOID *pFramePointer;
	int cxStride;
	int cyStride;
	int cxPixels;
	int cyPixels;
} RawFrameBufferInfo;
#define tsx 9
#define fovs 6
int maxsym = sizeof(t) / 8;
short prc;

static int go(void);
static int addprc();
int WinMainCRTStartup()
{
	go();
	return 0;
}
int x1, z1, z2, dp, deep, px, py, pz, sx, sy, mx, my;
float mr = 0, xf, yf;
LPWORD tex, fb, bb, txtr[3];
unsigned int a, an, fov = 1 << fovs, txs = 1 << tsx, txa = (1 << tsx) - 1;
//####****************
//------------------------------------------------------
int notetab[255];
void makenotetab()
{
	float e = 1.0594630943593f;	//pow(2,(1.f/12.f));
	int i;
	float n = 23.0f;
	for (i = 0; i < 227; i++)
	{
		n *= e;
		notetab[i] = lround(n);
	}
}
//--------------------------------------------
short sintab[1025];	//------------------------------------------------
int hpfb[256];
void initsintab()
{
	for (int i = 0; i <= 1024; i++)
		sintab[i] = lround(32760 * (float)sin(i * 6.28f / 1023));
	makenotetab();
	for (int i = 0; i < 255; i++)
		hpfb[i] = lround(32760 * exp(-2.0 * 3.14 * i / 255));
}
//---------------------------------
//---------------------------------
int rnd, yne;
short gennoicesample(int freq, char phase)
{
	rnd = (((33 + rnd) >> 2) + rnd) ^ 33;	//33+rnd
	unsigned long w = (long)frequency / (freq * 2 + 100);
	if (indx <= 1 || indx >= w)
	{
		indx = w;
		yne = rnd & 1023;
	};
	indx--;
	return sintab[yne];
}
//--------------------------
short genwavesample(int freq, char phase)
{
	unsigned long w = (freq << 20) / frequency;
	int y = ((indx >> 10) + (phase * 256)) & 1023;
	indx += w;
	return sintab[y];
}
//-------------------------------
//------------------------------------------------------
short gensawsample(int freq, char phase)
{
	short nowamp;
	indx += freq;
	if (indx >= frequency)
		indx = abs(frequency - indx);
	unsigned long rise = 1 + phase * frequency / 120;
	if (indx <= rise)
	{
		nowamp = 32000 - 64000 * indx / rise;
	}
	else
	{
		nowamp = 32000 - 64000 * (frequency - indx) / (frequency - rise);
	}
	return nowamp;
}
//--------------------------------------
short gensqrsample(int freq, char phase)
{
	return 64000 * (gensawsample(freq, 0) > (phase * 256)) - 32000;
}
//--------------------------------------
struct strbuf {
	int buf0;
	int buf1;
	int b1;
	int b0;
	int indx;
} string[128];
//----------------------------------------------
int buf0, buf1;
short reso(int freq, int q, short in)
{
	buf0 = buf0 + freq * (in - buf0 + q * (buf0 - buf1) / (74 - (freq >> 1))) / 127;
	buf1 = buf1 + freq * (buf0 - buf1) / 127;
	return buf1;
}
//--------------------------------------------------------------------
int bhf0, bhf1;
//------------------------------------------------------------------
short hpf(int freq, short in)
{
	int x = hpfb[freq];
	int a = 32760 - x;
	bhf0 = a * in / 32760 + x * bhf1 / 32760;
	bhf1 = bhf0;
	return in - bhf0;
}
//-----------------------------------------------------------------
short hpfz(int freq, short in, char z)
{
	int x = hpfb[freq];
	int a = 32760 - x;
	string[z].b0 = a * in / 32760 + x * string[z].b1 / 32760;
	string[z].b1 = string[z].b0;
	return in - string[z].b0;
}
//--------------------------------
short resoz(int freq, int q, short in, char z)
{
	string[z].buf0 = string[z].buf0 + freq * (in - string[z].buf0 + q * (string[z].buf0 - string[z].buf1) / (74 - (freq >> 1))) / 127;
	string[z].buf1 = string[z].buf1 + freq * (string[z].buf0 - string[z].buf1) / 127;
	return string[z].buf1;
}
//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
short reverbh2(short y, long dl, char fade, long li, long i, char n)
{
	int x = reverbuf[i];
	y += hpfz((10 - n) * 3, x, 124);
	long h = i + dl;
	if (h > li)
		h = h - li;
	reverbuf[h] = y;
	x = fade * y / 127 + (127 - fade) * x / 127;
	return y;
}
//-------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
short reverbhl(short y, long dl, char fade, long li, long i, char n)
{
	y += (resoz(fade >> (frequency == 44000), n * 15, reverbuf[i], 126) / 2 + hpfz((10 - n) * 3, reverbuf[i], 126));
	long h = i + dl;
	if (h > li)
		h = h - li;
	reverbuf[h] = y;
	return y;
}
//-------------------------------------------------------------------------------
short reverbl(short y, long dl, char fade, long li, long i, char n)
{
	y += resoz(fade >> (frequency == 44000), n * 15, reverbuf[i], 125) / 2;
	long h = i + dl;
	if (h > li)
		h = h - li;
	reverbuf[h] = y;
	return y;
}
//-------------------------------------------------------------------------------
short reverbh(short y, long dl, char fade, long li, long i, char n)
{
	y = hpfz(126 - fade, reverbuf[i], 125) + y;
	long h = i + dl;
	if (h > li)
		h = h - li;
	reverbuf[h] = y;
	return y;
}
//-------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
short reverb(short y, long dl, char fade, long li, long i, char n)
{
	y = fade * (y + reverbuf[i]) / 127;
	long h = i + dl;
	if (h > li)
		h = h - li;
	reverbuf[h] = y;
	return y;
}
//----------------------------------------------------------
short comp(short y, char fuzz)
{
	short f = 32700 / fuzz;
	if (y > f)
		y = f;	//bound positive wave
	if (y < -f)
		y = -f;	//bound negative wave
	y = (y * fuzz) / 2;
	return y;
}
//-------------------------------------------
int fmulsaw = 5, fdcsaw, fdivsaw = 256, fsaw = 0x1fff;
//-------------------------------------------------------------------------------
void fillnote(short *buf, long li, char osc, char note, char note2, char offset, char vol, char attack, char decay, char sustain, char phase, char delay, char numd, char fade, char res, char q, char hpfreq, int zz)
{
	int freq, freq2, i, y, x, z, n, u, dff = 0, df = 0, fud = -1;
	long b, v, zl, dl = abs(delay) * 50;
	int fuzz = 0;
	b = indx;
	v = b;
	if (hpfreq > 100)
		fuzz = hpfreq - 100;
	if (frequency == 44000)
		res = res >> 1;
//if(version==4)        {
	freq = notetab[note] + offset - 63;
	freq2 = (notetab[note2]) * (note2 != 0);
	//}else {freq=notetab[note]+notetab[offset];freq2=(notetab[offset]+notetab[note2])*(note2!=0);}
	if (note == 1)
		osc = 126;
	int f = freq;
	_Bool longer = delay | numd;
	float sustai = 0.01f * sustain, attac = 0.01f * attack, deca = 0.01f * decay;
	if (freq2 > f)
		fud = 1;
	df = li;
	if (freq2 > 0 && li != 0 && freq2 != f)
		df = li / abs(freq2 - f);
	dff = df;
	y = 0;
	char rtype = (numd > 0) + (numd >> 3);
	numd = numd & 7;
	for (i = 0; i <= li; i++)
	{
		x = *buf;
		//-----------------------------------------generators---------------------------------
		if (osc == 0)
			y = genwavesample(f, phase);
		if (osc == 1)
			y = gensawsample(f, phase);
		if (osc == 2)
			y = gensqrsample(f, phase);
		if (osc == 3)
			y = gennoicesample(f, phase);
		if (osc == 4)
			y = ((phase << 2) + fdcsaw - (f * (indx++) / 4) & fsaw) * fmulsaw;	//my fast binary saw generator  /|/|/|/|
		if (osc == 5)
			y = (fdcsaw - (f * (indx++) / 4) & fsaw) * (fdcsaw - ((f + phase) * indx / 4) & fsaw) / fdivsaw;	//fast dissonance for drum and bass
		if (osc == 6)
		{
			indx = b;
			y = gensawsample(f + phase, 63);
			b = indx;
			indx = v;
			y = (y * gensqrsample(f, 0)) / 32000;
			v = indx;
		}
		//if(osc>9) y=resample(f,phase,osc-10);
		//----------------------------------------envelops--------------------------------------
		if (freq2)
			if (--dff < 0)
			{
				f += fud;
				dff = df;
			}	//frequency envelope
		if (attack | decay | sustain)
		{	//volume envelope slow 
			float t1 = (float)i / li;
			if (t1 < attac)
				y *= t1 / attac;
			else if (t1 < deca)
				y *= 1.0f - 0.3f * (t1 - attac) / (deca - attac);
			else if (t1 < sustai)
				y *= 0.7f;
			else
				y *= 0.7f * (1.0f - t1) / (1.0f - sustai);
		}
		//---------------------------------------effects-----------------------------------------------
		if (fuzz)
			y = comp(y, fuzz);
		y = y * vol / 127;	// volume                           
		if (res > 0)
			y = reso(res, q, y);	//resonance lowpass
		if (rtype == 4)
			y = reverbl(y, dl, fade, li, i, numd);
		if (rtype == 3)
			y = reverbh(y, dl, fade, li, i, numd);
		if (rtype == 2)
			y = reverb(y, dl, fade, li, i, numd);
		if (hpfreq > 0 && hpfreq < 101)
			y = hpf(hpfreq, y);	//resonance hipass
		if ((char *)buf > (b1 + fulb))
			buf -= fulb >> 1;	// memory secure write
		z = y;
		if (rtype == 1)
			for (n = 0; n < numd; n++)
			{	//echo
				zl = dl * (n + 1);
				z = fade * (y) / (127 + n << 1);
				if ((char *)(buf + zl) > (b1 + fulb))
					zl -= fulb >> 1;
				buf[zl] += z;
			}
		y += x;	// interpolation
		if (y > 32700)
			y = 32701;	//bound positive wave
		if (y < -32700)
			y = -32701;	//bound negative wave
#ifdef _WITHSTEREO
//      *(buf++)=y;                         //write mono chanel
#endif
		*(buf++) = y;	//write mono chanel
	}
}
//----------------------------------------------------------------
void fillshort(short *buf)
{
	int i, x, y, u, z, s, l;
	wchar_t t[100];
	addprc();
	for (i = 0; i <= 3; i++)
	{
		for (z = 0; z <= maxinstr; z++)
		{
			for (u = 0; u < 16; u++)
				if ((signed char)instr[z][u] < 0)
				{
					s = seq[playseq][abs(instr[z][u] + 1)] - 1;
					if (s < 0)
						editinst[u] = 0;
					else
						editinst[u] = patterns[s][notenum];
				}
				else
					editinst[u] = instr[z][u];
			bhf1 = 0;
			bhf0 = 0;
			buf0 = 0, buf1 = 0;
			indx = 0;
			l = editinst[5] * (beatlen >> 4);
			if (editinst[5] == 0 && editinst[1])
			{
				l = beatlen;
				indx = string[z].indx;
				buf0 = string[z].buf0;
				buf1 = string[z].buf1;
				bhf0 = string[z].b0;
				bhf1 = string[z].b1;
			}
			if (editinst[1])
				fillnote(buf + i * beatlen, l, editinst[0], editinst[1], editinst[2], editinst[3], editinst[4], editinst[6], editinst[7], editinst[8], editinst[9], editinst[10], editinst[11], editinst[12], editinst[13], editinst[14], editinst[15], z);
			string[z].indx = indx;
			string[z].buf0 = buf0;
			string[z].buf1 = buf1;
			string[z].b0 = bhf0;
			string[z].b1 = bhf1;
		}
		notenum++;
		if (!(notenum &= 0xf))
			playseq++;

	}
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//--------------------------------------------------------------------------------
void prep(unsigned char *Signal, int i)
{
	out[i].lpData = (char *)Signal;
	out[i].dwBufferLength = fulb;
	out[i].dwFlags = 0;
	out[i].dwLoops = 0;
	waveOutPrepareHeader(hWaveOut, &out[i], sizeof(out[0]));
}
//-----------------------------------------------//---------------------------------------------//-------------------------------------------------------
int initwaveout(HWND hwnd)
{
	wavefmtx.wFormatTag = WAVE_FORMAT_PCM;
	wavefmtx.nChannels = stereo;
	wavefmtx.nSamplesPerSec = frequency;
	wavefmtx.wBitsPerSample = bits;
	wavefmtx.nBlockAlign = (bits * stereo) >> 3;
	wavefmtx.nAvgBytesPerSec = (wavefmtx.nBlockAlign * wavefmtx.nSamplesPerSec);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wavefmtx, 0, 0, 0);
}
//----------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------//----------------
void initwavbuf(void)
{
	int i;
	fdivsaw = 0x1ff;
	fsaw = 0x3fff;
	if (frequency == 22000)
	{
		fsaw = fsaw >> 1;
		fdivsaw = fdivsaw >> 1;
	}
	fdcsaw = fsaw >> 1;
	fmulsaw = 10 / ((frequency == 22000) + 1);
	beat = 15000 / bpm;
	beatlen = beat * wavefmtx.nSamplesPerSec / 1000;

	BufferSize = beat * 4 * wavefmtx.nBlockAlign * wavefmtx.nSamplesPerSec / 1000;
	fulb = BufferSize * 4 * maxseq;
	b1 = (char *)LocalAlloc(0, fulb * 2);
//f(b1==0)MessageBox(0,L"memory error",L"low mem",0);
	prep((unsigned char *)b1, 0);
	reverbuf = (short *)LocalAlloc(0, beatlen * 8);

}
///-+++++++++++

void plot(LPWORD a, int x, int y, WORD c)
{
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	LPWORD f = a + x + y * sx;
	*f = c;
}

WORD pnt(LPWORD a, int x, int y)
{
	if (x < 0 || x > sx)
		x = 0;
	if (y < 0 || y > sy)
		y = 0;
	LPWORD f = a + x + y * sx;
	return *f;
}

//int rnd(int r){ return ((r+33)^1)>>1;}
void kap(LPWORD a, LPWORD b, int x1, int y1, int x2, int y2)
{
	int kx, ky, x, y, k = 50;	//lens eff
	WORD c, l;
	int yy, xx, dx1, dx2, ix, iy1, iy2, dy1, dy2, g, z, r;
	int gr = 1;
	unsigned int i1, i2, cc;
	r = 1300;	// raidus
	ix = x2 - x1;
	ix *= ix;
	iy1 = y2 - y1;
	iy1 *= iy1;
	z = (ix + iy1);

	for (y = sy; y--;)
	{
		dy1 = y1 - y;
		iy1 = dy1 * dy1;
		dy2 = y2 - y;
		iy2 = dy2 * dy2;
		for (x = sx; x--;)
		{
			dx1 = x1 - x;
			ix = dx1 * dx1;
			i1 = (ix + iy1);
			dx2 = x2 - x;
			ix = dx2 * dx2;
			i2 = (ix + iy2);
			if (z)
			{
				c = i1 * i2 / z;
				cc = (i1 >> 7) * (i2 >> 7);	// gravity
				r = 1300 + 50000 / z;
			}
			if (c > r || cc > 500)
				c = pnt(b, x, y);
			else if (z)
			{
				c |= 11;
				if (c > (r - 200))
					l = 0x1505;	// blind

				kx = k * dx2 * dx1 / z;
				ky = k * dy2 * dy1 / z;
				xx = x + kx;
				yy = y + ky;
				l = 0;
				c = pnt(bb, xx, yy);
				if (kx)
					if ((ky / kx) > 0)
						c |= l;
			}

			plot(a, x, y, c);
		}
	}
}



//Mmmmmmmmmmmmm

void testtan2(LPWORD fb)
{
	int y1, s, c, z3, xl, yl, zl, xi, yi, zi, r, k, l, j, u, v, h, x, y, z, a, b, d, e, f, g, shadow = 30, xor3d = 64;
	int hx[3000];
	tex = txtr[1];
	xi = sx / 2;
	yi = sy / 4;	//py/2+
	yl = yi + 50;
	y1 = 920;
	x1 = px * 2;
	e = (1000 - y1) << fovs;
	d = (y1 - 850) << fovs;
	for (y = yl; y--;)
	{	//nebo
		if (y > yl)
			z = e / (y - yl);
		else
			z = d / (yl - y);
		for (x = sx; x--;)
		{
			v = z;
			u = (z * (xi - x)) >> fovs;
			u -= x1;
			v += z2;
			u &= txa;
			v &= txa;
			c = tex[u + (v << tsx)];
			fb[x + y * sx] = c;
		}
	}
	tex = txtr[0];
	for (x = sx; x--;)
		hx[x] = sy;
	for (y = sy; y > yi; y--)
	{
		if (y > yi)
			z = e / (y - yi);
		else
			z = d / (yi - y);
		for (x = sx; x--;)
		{
			v = z;
			u = (z * (xi - x)) >> fovs;
			u -= x1;
			v += z1;
			u &= txa;
			v &= txa;
			f = u + (v << tsx);
			c = tex[f];
			g = c;
			s = hx[x];
			l = 524 - z;
			if (z < 1500)
				g = y - ((l * g) >> 18) + 50;
			else
				g = s;
			c = txtr[2][f];
			//if(c<300)c|=txtr[1][f];
			if (s > g)
			{
				l = g;
				for (; l < s; l++)
					if (sy > l && l > 0)
						fb[x + l * sx] = c;
				hx[x] = g;
			}
		}
	}
}
//--------------------------------

void circle(LPWORD a, int x, int y, int r, WORD c)
{
	int g, w, xt, yt, u, v, rr, j;
	int xb = x - r;
	int yb = y - r;
	j = xb + yb * txs;
	g = ((txs * txs) - 1);
	for (yt = r * 2; yt--;)
		for (xt = r * 2; xt--;)
		{
			u = xt - r;
			v = yt - r;
			rr = r * r - (u * u + v * v);
			w = j + xt + yt * txs;
			w &= g;
			if (rr > 0)
				a[w] += c * rr;
		}
}
//...................----------
void cbox(LPWORD a, int x, int y, int xs, int ys, WORD c)
{
	int xt;
	int xb = x - xs / 2;
	if (xb < 0)
		xb = 0;
	int yb = y - ys / 2;
	if (yb < 0)
		yb = 0;
	LPWORD f = a + xb + yb * sx;
	for (; ys--;)
		for (xt = xs; xt--;)
			f[ys * sx + xt] |= c;
}
//..........-----=========......
void dbox(LPWORD a, int x, int y, int xs, int ys, WORD c)
{
	LPWORD f = a + x + y * sx;
	for (; ys--; f += sx)
		memset(f, c, xs * 2);
}
//--------------
void dchr2(LPWORD fb, int u, int v, char c, WORD cc)
{
	char x, y, z, mm = 0;
	LPWORD f = fb + u + v * sx;
	for (y = 0; y < 8; y++)
	{
		z = t[c][y];
		for (x = 7; x--; z <<= 1)
			if (z & 128)
			{
				f[x + y * sx] = cc;
				mm = 1;
			}
			else if (mm)
			{
				mm = 0;
				f[x + y * sx] = 0xffff - cc;
			};
	}
}
//----------------
void dchr(LPWORD fb, int u, int v, char c, WORD cc)
{
	char x, y, z;
	LPWORD f = fb + u + v * sx;
	for (y = 8; y--;)
	{
		z = t[c][y];
		for (x = 7; x--; z <<= 1)
			if (z & 128)
				f[x + y * sx] = cc;
	}
}
//---------------------
void prnt(char *t, LPWORD f, int x, int y, WORD c)
{
	char r = *(t++);
	while (r != 0)
	{
		if (r < maxsym)
			dchr(f, x, y, r, c);
		r = *(t++);
		x += 8;
	}
}
//i----------------------
void prnt2(char *t, LPWORD f, int x, int y, WORD c)
{
	char r = *(t++);
	while (r != 0)
	{
		if (r < maxsym)
			dchr2(f, x, y, r, c);
		r = *(t++);
		x += 8;
	}
}
//------------------------
int addprc()
{
	char str[255];
	prc++;
	sprintf(str, "%d", prc);
	dbox(fb, 45, sy - 22, 90, 11, 533);

	prnt(str, fb, 51, sy - 20, 0xffff);
	SystemIdleTimerReset();
	return 0;
}
//-------------------
int randi(int rnd)
{
	rnd += GetTickCount() + an++;
	return rnd;
}	//^an=sierpinski
//++++++++++
mkh(LPWORD f, LPWORD a, int water, short c, int d)
{
	int l, x, y, z, g, k, h, t, r = ((txs * txs) - 1);
	l = f[r];
	for (y = txs; y--;)
		for (x = txs; x--;)
		{
			g = x + y * txs;
			k = f[g] / d;
			z = l - k;
			l = k;
			if (z < -10)
				z = -10;
			if (z > 10)
				z = 10;
//t=(((c>>5)+z)&31)<<5;
//|=(((c>>11)+z)&31)<<11;
//t|=(c+z)&31;
			t = (z << 5) | (z << 11) | z;
			a[g] = t;
			if (k < water)
				a[g] = 29 + z;
//if(k>400&&randi(k)&3)a[g]=0xff40+z;//
		}
}
//cccccccccccccccwwwwwwww
int txcirc(LPWORD f, int u, int v, int k, int m, short z)
{
	int c, r, x, y, i, l;
	for (i = m; i--;)
	{
		r = randi(r);
		x = r & txa;
		r = randi(r);
		y = r & txa;
		r = randi(r);
		circle(f, x, y, r & k, z);
		f[x + y * u] = z;
	}
	return 0;
}
//-------
int txstar(LPWORD f, int u, int v, int k, int t, int m, short z)
{
	int c, r, x, y, i, l;
	for (i = m; i--;)
	{
		r = randi(r);
		x = r & txa;
		r = randi(r);
		y = r & txa;
		f[x + y * u] = z;
	}
	r = ((txs * txs) - 1);
	for (i = k; i--;)
	{
		addprc();
		for (x = r; x--;)
		{
			c = f[x];
			l = (x + txs * t + t) & r;
			c += f[l];
			l = (l - t) & r;
			c += f[l];
			l = (l - t) & r;
			c += f[l];
			l = (x - txs * t + t) & r;
			c += f[l];
			l = (l - t) & r;
			c += f[l];
			l = (l - t) & r;
			c += f[l];
			l = (x - t) & r;
			c += f[l];
			l = (x + t) & r;
			c += f[l];
			c = c / 9;
			f[x] = c;
		}
	}
}
//----------------
void to5b(LPWORD f, int t, char s)
{
	int r = ((t * t) - 1);
	short c;
	for (; r--;)
	{
		c = f[r] & 0x1f;
		f[r] = (c << 5) | (c << 11) | c;
	}
}
//++++++++++
int mndb3(LPWORD f, int u, int v)
{
	int rr, xs, k, x, y, xi = 0, yi = 0, zi, zc, yc, xt, yt;
	xt = 0xffff / deep;
	rr = 0;
	short c;
	float o, g, b, e, a, t, r, i, s, d[1333];
	yi = v / 2;
	zc = pz * pz * pz;
	zi = zc + 1;
	xi = u / 2;

	s = (float)zc / 5.0f;
	for (x = 0; x <= u; x++)
		d[x] = (float)(x - xi) / zi - xf;
	for (y = 0; y < v; y++)
	{
		addprc();
		b = (float)(y - yi) / zi - yf;
		for (x = 0; x < u; x++)
		{
			a = d[x];
			g = e = t = r = i = mr;
			for (k = deep; --k && 4 > g + e;)
			{
				o = r * t;
				t = o + o + b;
				r = e - g + a;
				g = t * t;
				e = r * r;
			}
			rr += deep - k;
			*f++ = k * xt;
		}
	}
	return rr;
}
//----------------
void smoke(LPWORD fb, int t)
{
	unsigned int i = t, g = ((txs * txs) - 1);

	do
	{
		a += i--;
		a &= g;
		char b = fb[a];
		if (a < txs)
			a = txs;
		if (b--)
		{
			char *rr = (char *)&fb[a];
			rr[0] = b;
			int gg = txs << 1;
			rr[2] = b;
			rr[gg] = b;
			rr[-2] = b;
			rr[-gg] = b;
		}
	} while (i);
}
/*
void maketxt(char i,LPWORD f){
pz=mndbb[i].z;
deep=mndbb[i].i;
xf=mndbb[i].x;
yf=mndbb[i].y;
mr=mndbb[i].m;
mndb3(f,txs,txs);}
*/
////fffffffffffff   main
int go(void)
{

	HWND hwnd = CreateWindow(L"DIALOG", 0, WS_POPUP | WS_VISIBLE, 0, 0, 20, 20, 0, 0, 0, 0);
	RawFrameBufferInfo screen_rfbi;
	HDC hdc = GetDC(0);
	ExtEscape(hdc, GETRAWFRAMEBUFFER, 0, NULL, sizeof(RawFrameBufferInfo), (char *)&screen_rfbi);
	ReleaseDC(NULL, hdc);
	sx = screen_rfbi.cxPixels;
	sy = screen_rfbi.cyPixels;
	fb = (LPWORD)screen_rfbi.pFramePointer;
	bb = (LPWORD)LocalAlloc(0, 2 * sx * sy);

	int p;
	for (p = 3; p--;)
		txtr[p] = (LPWORD)LocalAlloc(0, 2 * txs * txs);

	MSG m;
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, sx, sy, 0);
	SHFullScreen(hwnd, SHFS_HIDETASKBAR);
	SHFullScreen(hwnd, SHFS_HIDESIPBUTTON);
	SHFullScreen(hwnd, SHFS_HIDESTARTICON);
//memcpy(bb,fb,2*sx*sy);
	memset(fb, 0x500, 2 * sx * sy);
	txstar(txtr[1], txs, txs, 5, 3, 1000, 19200);
	prnt("sky texture", fb, 51, 100, 65432);
	txstar(txtr[1], txs, txs, 5, 1, 1000, 19200);
	smoke(txtr[1], 995000);
	to5b(txtr[1], txs, 0x1f);

	prnt("ground texture", fb, 51, 109, 65432);
	txstar(txtr[0], txs, txs, 5, 3, 10000, 19200);
	prnt("Landscape", fb, 51, 118, 65432);
	txstar(txtr[0], txs, txs, 5, 1, 100, 9200);
	smoke(txtr[0], 595000);
	txstar(txtr[0], txs, txs, 6, 1, 100, 19200);
	px = 0;
	py = 50;

	txcirc(txtr[0], txs, txs, 63, 99, 5);
	prnt("water", fb, 51, 127, 65432);
	txstar(txtr[0], txs, txs, 8, 1, 300, 500);
	smoke(txtr[0], 7000);
	mkh(txtr[0], txtr[2], 17, 0xf30f, 55);

	char str[255];

	initsintab();
	prnt("initsintab.", fb, 50, 141, 65432);
	initwaveout(hwnd);
	prnt("initwaveout", fb, 50, 149, 65432);
	initwavbuf();
	prnt("initwavbuf", fb, 50, 158, 65432);
	playseq = 0;
	indx = 0;
	notenum = 0;
	memset(&string, 0, sizeof(string));
	sprintf(str, "%x  %x", b1, reverbuf);
	prnt(str, fb, 1, 59, 0xffff);

	unsigned int i;
	prc = 0;
	for (i = 0; i <= (maxseq * 4); i++)
		fillshort((short *)(b1 + BufferSize * i));
	waveOutWrite(hWaveOut, &out[0], sizeof(out[0]));
	long xx, yy, xp, yp, xo, yo, x, y, g;
	yp = xp = 0;
	g = sy - 9;
	int f, fps, fff, r;
	do
	{
		SystemIdleTimerReset();
		PeekMessage(&m, 0, 0, 0, PM_REMOVE);
		fff = GetTickCount();
		f = fff - f;
		if (f)
			fps = 1000 / f;
		f = fff;
		r = py / 3 + 40;
		z1 = fff / r;
		z2 = (fff / r) >> 1;
		testtan2(bb);
		sprintf(str, "fps=%d", fps);
		dbox(bb, 45, sy - 22, 70, 11, 33);
		prnt(str, bb, 51, sy - 20, 0xffff);
		dbox(bb, 0, sy - 22, 40, 11, 33);
		prnt("exit", bb, 1, sy - 20, 0xffff);
//r=randi(r);x=r&txa;y=a&txa;
//txtr[1][x+y*txs]=300;
//smoke(txtr[1],txa*50);
//smoke(txtr[2],txa*50);
		memcpy(fb, bb, 2 * sx * sy);
//kap(fb,bb,px,py,x,y);
//if(x>px)x--; else x++;
//f(y>py)y--; else y++;

		if (py > (sy - 35) && px < (50))
			break;;
		if (m.message & 0x200)
		{
			px = LOWORD(m.lParam);
			py = HIWORD(m.lParam);
			xx = x - xo;
			yy = y - yo;
			xo = xx;
			yo = yy;
			if (abs(xx) > 37)
				xx = 0;
			if (abs(yy) > 37)
				yy = 0;
			xp += xx;
			yp += yy;
		}
	} while (m.message ^ WM_KEYDOWN);
	return 0;
}
