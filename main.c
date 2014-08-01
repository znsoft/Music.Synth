#define UNICODE
#define WINCE_DEFAULT_LIBS
#define WIN32_DEFAULT_LIBS
#include <windows.h>
#ifdef _WINCE
#include <winuserm.h>
#include <aygshell.h>
int frequency = 22000;
#else
int frequency = 44000;
#define VK_TSOFT2 VK_F2
#define VK_TSOFT1 VK_F1
#endif
#include <mmsystem.h>
#include <math.h>
#define numofbuf      	10
#define echobuf			7
#define bits				16
#define stereo			1
#define true 			TRUE
#define false 			FALSE

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

WAVEFORMATEX wavefmtx, pwfx;
HWAVEOUT hWaveOut;
WAVEHDR WaveHeader, out[numofbuf];
char *b1, *scope;
char snake[numofbuf + 1];
unsigned char bpm;
int tx, ty, version = 4;
char interpolation = 1;
#define maxsqy 255
char seq[maxsqy][maxsqy];	//трэки каналы , дорожки , последовательность паттернов.
struct charsnames {
	char p[16];
	wchar_t nme[20];
	short color;
} tmpins[255], instr[255], patterns[255];	//список инструментов , паттернов
struct sample {
	short *s;
	long l;
	_Bool loop;
	_Bool pingpong;
	_Bool voice;
	wchar_t name[20];
	char speak[127];
} smp[64];
int maxsamples = 0;
int maxinstr = 0, editinstr = 0, playseq = 0;
int maxpatnames, notenum, maxseq = 1, maxtrack;
wchar_t filelist[100][160];
int maxfilelist, patviewmode;
wchar_t editfile[255];
char editpattern = 0;	//редактируемый паттерн
struct buffers {
	char *bf;
} bf[numofbuf];	// audio buffers
struct mnu {
	wchar_t mnu[20];
} menu[] =
{
L" ", L"samples", L"scope", L"file", L"instruments", L"pattern", L"sequence"}, filemenu[] =
{
L"Exit", L"Load", L"Save", L"SaveAs", L"Export .wav", L"New", L"Merge", L"export .h", L"Random", L"export .mid", L"import .mid", L"pat view", L"samplerate", L"BPM"}, rndmnu[] =
{
L"type", L"note", L"width", L"numofpat", L"pauses", L"seed", L"minnotelen", L"repeat", L"random"};	// main menu
int filemcnt = sizeof(filemenu) / sizeof(filemenu[0]);
char onlyselectpattern = 0;
int xseq, yseq, xofs = 0, yofs = 0, taboffset = 0;
wchar_t selfdir[255];
struct mnu params[] = { L"osc", L"note", L"to note", L"noteoffset", L"volume", L"len ", L"attack", L"decay", L"sustain", L"phase", L"delay", L"echos", L"fade", L"reso", L"resolev", L"hpf" };
char editinst[16];
PVOID hwnd;
int paramscount = sizeof(params) / sizeof(params[0]);
int filemnucnt = sizeof(filemenu) / sizeof(filemenu[0]);
_Bool move;
short *reverbuf;
//DWORD ascgrp[256][8];

ULONG onesec, BufferSize, fulb, fillpoint, beatlen;	//buffer  vars
int editchar, gx, gy, octave, mode, menuitem, index, fillcell, playcell, erasecell, played, mnurows[10];	//vars
char upup;
_Bool keys[256];
_Bool notime, change, fire, showosc, play, showmenu;
//keyboard variables--------------------
int keysel = 12;
int keyx = 1;
int keytab[200];
int selcol = 0;
int paternnotes[100];
int patx[100];
const int maxnotepat = 16;
const int notincol = 4;
int sx, sy;	//размеры экрана -------------ниже "экранные" переменные 
BITMAP rDIB;
BYTE *dstDIBBits;
HDC hdcMem;
HBITMAP hDIBResult;
HBITMAP oldBmp;
long dstRowBytes;
BYTE *dstDIBBits;
//--------------------------------
void print(wchar_t *text, int x, int y)
{
	int left = x, top = y, right = sx, bottom = y + 20;
	RECT rc = { left, top, right, bottom };
	DrawText(hdcMem, text, -1, &rc, DT_LEFT | DT_NOCLIP | DT_NOPREFIX);
}
//------------------------------------------------------
int notetab[255];
void makenotetab()
{
	float e = pow(2, (1.f / 12.f));
	int i;
	float n = 23;
	for (i = 0; i < 227; i++)
		notetab[i] = n *= e;
}
//--------------------------------------------
short sintab[1025];	//------------------------------------------------
int hpfb[256];
void initsintab()
{
	for (int i = 0; i <= 1024; i++)
		sintab[i] = 32760 * (float)sin(i * 6.28f / 1023);
	makenotetab();
	for (int i = 0; i < 255; i++)
		hpfb[i] = 32760 * exp(-2.0 * 3.14 * i / 255);
}
//------------------------------
int divfreq = 440;
//-------------------------------------
short resample(int freq, char phase, char n)
{
	if (smp[n].s == 0)
		return 0;
	index++;
	long pos = index * freq / divfreq;
	pos += phase * smp[n].l / 127;
	if (pos >= smp[n].l)
		if (smp[n].loop)
		{
			index = 0;
			pos = 0;
		}
		else
			return 0;
	return smp[n].s[pos];
}
//---------------------------------
int rnd, yn;
short gennoicesample(int freq, char phase)
{
	rnd = (((33 + rnd) >> 2) + rnd) ^ 33;	//33+rnd
	unsigned long w = (long)frequency / (freq * 2 + 100);
	if (index <= 1 || index >= w)
	{
		index = w;
		yn = rnd & 1023;
	};
	index--;
	return sintab[yn];
}
//--------------------------
short genwavesample(int freq, char phase)
{
	unsigned long w = (freq << 20) / frequency;
	int y = ((index >> 10) + (phase * 256)) & 1023;
	index += w;
	return sintab[y];
}
//-------------------------------
//--------------------------
//------------------------------------------------------
short gensawsample(int freq, char phase)
{
	short nowamp;
	index += freq;
	if (index >= frequency)
		index = abs(frequency - index);
	unsigned long rise = 1 + phase * frequency / 120;
	if (index <= rise)
	{
		nowamp = 32000 - 64000 * index / rise;
	}
	else
	{
		nowamp = 32000 - 64000 * (frequency - index) / (frequency - rise);
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
	int index;
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
	b = index;
	v = b;
	if (hpfreq > 100)
		fuzz = hpfreq - 100;
	if (frequency == 44000)
		res = res >> 1;
	if (version == 4)
	{
		freq = notetab[note] + offset - 63;
		freq2 = (notetab[note2]) * (note2 != 0);
	}
	else
	{
		freq = notetab[note] + notetab[offset];
		freq2 = (notetab[offset] + notetab[note2]) * (note2 != 0);
	}
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
//while(longer){
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
			y = ((phase << 2) + fdcsaw - (f * (index++) / 4) & fsaw) * fmulsaw;	//my fast binary saw generator  /|/|/|/|
		if (osc == 5)
			y = (fdcsaw - (f * (index++) / 4) & fsaw) * (fdcsaw - ((f + phase) * index / 4) & fsaw) / fdivsaw;	//fast dissonance for drum and bass
		if (osc == 6)
		{
			index = b;
			y = gensawsample(f + phase, 63);
			b = index;
			index = v;
			y = (y * gensqrsample(f, 0)) / 32000;
			v = index;
		}
		if (osc > 9)
			y = resample(f, phase, osc - 10);
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
//      if(rtype==6)y=reverbhl(y,dl,fade,li,i,numd);
//      if(rtype==5)y=reverbhl(y,dl,fade,li,i,numd);
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
		*(buf++) = y;	//write mono chanel
#endif
		*(buf++) = y;	//write mono chanel

	}	//longer=false;li=dl*numd;osc=125;}
}
//---------------------------------------------------------
//----------------------------------------------------------------
void fillshort(short *buf)
{
	int i, x, y, u, z, s, l;
	for (i = 0; i <= 3; i++)
	{
		for (z = 0; z <= maxinstr; z++)
		{
			for (u = 0; u < 16; u++)
				if ((char)instr[z].p[u] < 0)
				{
					s = seq[playseq][abs(instr[z].p[u] + 1)] - 1;
					if (s < 0)
						editinst[u] = 0;
					else
						editinst[u] = patterns[s].p[notenum];
				}
				else
					editinst[u] = instr[z].p[u];
			bhf1 = 0;
			bhf0 = 0;
			buf0 = 0, buf1 = 0;
			index = 0;
			l = editinst[5] * (beatlen >> 4);
			if (editinst[5] == 0 && editinst[1])
			{
				l = beatlen;
				index = string[z].index;
				buf0 = string[z].buf0;
				buf1 = string[z].buf1;
				bhf0 = string[z].b0;
				bhf1 = string[z].b1;
			}
			if (editinst[1])
				fillnote(buf + i * beatlen, l, editinst[0], editinst[1], editinst[2], editinst[3], editinst[4], editinst[6], editinst[7], editinst[8], editinst[9], editinst[10], editinst[11], editinst[12], editinst[13], editinst[14], editinst[15], z);
			string[z].index = index;
			string[z].buf0 = buf0;
			string[z].buf1 = buf1;
			string[z].b0 = bhf0;
			string[z].b1 = bhf1;
		}
		notenum++;
		if (!(notenum &= 0xf))
			playseq++;
		if (playseq >= (maxseq + 1))
		{
			playseq = 0;
			index = 0;
			memset(&string, 0, sizeof(string));
		}
	}
}
//-----------------------------------------------------------
//VVVVVVVVVVVVVVVVVOOOOOOOOOOOOOOOIIIIIIIIIIIIIIIICCCCCCCCCCCEEEEEEEEEEEEEEEE--------------------------------
//-------voice generator by stan1911 ----------------------
const float PI = 3.1415926535f;
const float PI_2 = 6.28;
float Sawtooth(float x)
{
	return (0.5f - (x - floor(x / PI_2) * PI_2) / PI_2);
}
struct Phoneme {
	char p;
	unsigned char f[3];
	unsigned char w[3];
	struct {
		unsigned char len:2;
		unsigned char amp:4;
		unsigned char osc:1;
		unsigned char plosive:1;
	} Shape;
};
struct Phoneme g_phonemes[] = { {'o', 12, 15, 0, 10, 10, 0, {3, 6, 0, 0}},
{'i', 5, 56, 0, 10, 10, 0, {3, 3, 0, 0}},
{'j', 5, 56, 0, 10, 10, 0, {1, 3, 0, 0}},
{'u', 5, 14, 0, 10, 10, 0, {3, 3, 0, 0}},
{'a', 18, 30, 0, 10, 10, 0, {3, 15, 0, 0}},
{'e', 14, 50, 0, 10, 10, 0, {3, 15, 0, 0}},
{'E', 20, 40, 0, 10, 10, 0, {3, 12, 0, 0}},
{'w', 3, 14, 0, 10, 10, 0, {3, 1, 0, 0}},
{'v', 2, 20, 0, 20, 10, 0, {3, 3, 0, 0}},
{'T', 2, 20, 0, 40, 1, 0, {3, 5, 0, 0}},
{'z', 5, 28, 80, 10, 5, 10, {3, 3, 0, 0}},
{'Z', 4, 30, 60, 50, 1, 5, {3, 5, 0, 0}},
{'b', 4, 0, 0, 10, 0, 0, {1, 2, 0, 0}},
{'d', 4, 40, 80, 10, 10, 10, {1, 2, 0, 0}},
{'m', 4, 20, 0, 10, 10, 0, {3, 2, 0, 0}},
{'n', 4, 40, 0, 10, 10, 0, {3, 2, 0, 0}},
{'r', 3, 10, 20, 30, 8, 1, {3, 3, 0, 0}},
{'l', 8, 20, 0, 10, 10, 0, {3, 5, 0, 0}},
{'g', 2, 10, 26, 15, 5, 2, {2, 1, 0, 0}},
{'f', 8, 20, 34, 10, 10, 10, {3, 4, 1, 0}},
{'h', 22, 26, 32, 30, 10, 30, {1, 10, 1, 0}},
{'s', 80, 110, 0, 80, 40, 0, {3, 5, 1, 0}},
{'S', 20, 30, 0, 100, 100, 0, {3, 10, 1, 0}},
{'p', 4, 10, 20, 5, 10, 10, {1, 2, 1, 1}},
{'t', 4, 20, 40, 10, 20, 5, {1, 3, 1, 1}},
{'k', 20, 80, 0, 10, 10, 0, {1, 3, 1, 1}},
{'*', 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}}
};
//---------------------------------- Synthesizes speech synth
_Bool SpeechComplex(short *buf, char *text)
{
	int ln, f0, am;
	int maxphonemes = sizeof(g_phonemes) / sizeof(g_phonemes[0]);
	int SAMPLE_FREQUENCY = 22000;
	_Bool whisper = FALSE;
	_Bool dcshow = true;
	f0 = 120;
	ln = 32;
	am = 32;
	RECT rc = { 0 };
	rc.right = 170;
	rc.bottom = 170;
	for (unsigned char *l = text; *l; l++)
	{
		struct Phoneme *p = g_phonemes;
		float v = 0;
		if (*l == 0)
			return true;
		if (*l == '1')
		{
			l++;
			f0 = (*l) + 32;
			continue;
		}
		if (*l == '2')
		{
			l++;
			ln = *l;
			continue;
		}
		if (*l == '3')
		{
			l++;
			am = *l;
			continue;
		}
		if (*l == '4')
		{
			whisper = FALSE;
			continue;
		}
		if (*l == '5')
		{
			whisper = TRUE;
			continue;
		}
		if (*l == '6')
		{
			dcshow = false;
			continue;
		}
		if (*l != ' ')
		{
			while (p->p != *l)
			{
				p++;
				if (p->p == '*')
					return false;
			}
			v = p->Shape.amp;
		}
		rc.left += 7;
		rc.top += (rc.left & 0xf0) >> 5;
		rc.left &= 0x7f;
		rc.top &= 0x7f;
		wchar_t t[2];
		*t = (short)*l;
		if (dcshow)
			DrawText(GetDC(0), t, 1, &rc, 0);
		// Generate sound
		int preamp = 51 - (SAMPLE_FREQUENCY / 1000);
		int sl = (int)(p->Shape.len * (SAMPLE_FREQUENCY / 15) * (ln / 32.0f));
		for (int f = 0; f < 3; f++)
		{
			char ff = p->f[f];
			if (!ff)
				continue;
			float buf1Res = 0, buf2Res = 0;
			float csf = cos(PI_2 * ff * 50.0f / SAMPLE_FREQUENCY);
			float q = 1.0f - p->w[f] * (PI * 10.0f / SAMPLE_FREQUENCY);
			csf *= 2.0f * q;
			q *= q;
			short *b = buf;
			short x, xp = 0;
			index = 0;
			float a = am / 32.0f;
			float f1 = f0 * PI_2 / SAMPLE_FREQUENCY;

			for (int s = 0; s < sl; s++)
			{
				float n = (float)rand() / RAND_MAX - 0.5f;
				x = preamp * n;
				if (!p->Shape.osc && !whisper)
				{
					x = preamp * Sawtooth(s * f1);
					xp = 0;
				}
				x *= a;
				//for ( int s = 0; s < sl; s++ ) {if ( !p->Shape.osc && !whisper ) {    x =  gensawsample(f0,1);    xp = 0; }else x=gennoicesample(16000,0);    x =x / am;
				x = x + csf * buf1Res - buf2Res * q;
				buf2Res = buf1Res;
				buf1Res = x;
				x = 0.75f * xp + x * v;
				xp = x;
				*(b++) += x;
			}
		}
		// Overlap neighbour phonemes
		buf += (3 * sl / 4);
		if (p->Shape.plosive)
			buf += (sl & 0xfffffe);
	}
	return true;
}
//---------------------------------------------------------
long calclen(char *text)
{
	float ln, f0, am;
	_Bool whisper = FALSE;
	_Bool dcshow = FALSE;
	f0 = 220;
	ln = 32;
	am = 32;
	long r = 0;
	for (char *l = text; *l; l++)
	{
		if (*l == '1')
		{
			l++;
			f0 = *l;
			continue;
		}
		if (*l == '2')
		{
			l++;
			ln = *l;
			continue;
		}
		if (*l == '3')
		{
			l++;
			am = *l;
			continue;
		}
		if (*l == '4')
		{
			whisper = FALSE;
			continue;
		}
		if (*l == '5')
		{
			whisper = TRUE;
			continue;
		}
		if (*l == '6')
		{
			dcshow = TRUE;
			continue;
		}
		r += (long)((22000 / 5) * (ln / 32.0f));
	} return r;
}
// Performs no error checking and does not dispose resources.
///-----------------------------------------------------
_Bool voice(char *text, char n)
{
	long y = calclen(text);
	smp[n].s = (short *)malloc(y * 2);
	smp[n].l = y;
	smp[n].voice = true;
	memset(smp[n].s, 0, y * 2);
	return SpeechComplex(smp[n].s, text);
}
//---------------------------------------------------------
void erasemusic(void)
{
	maxsamples++;
	for (; maxsamples; maxsamples--)
		free(smp[maxsamples].s);
	maxseq = 0;
	maxinstr = 0;
	maxpatnames = 0;
	maxtrack = 0;
	maxsamples = 0;
	memset(smp, 0, sizeof(smp));
	memset(seq, 0, sizeof(seq));
	memset(patterns, 0, sizeof(patterns));
	memset(instr, 0, sizeof(instr));
}
//-------------------------------------------------------------
int searchfreechn(void)
{
	char o, c, cc[256], t;
	int i;
	memset(cc, 0, 255);
	for (i = 0; i <= maxinstr; i++)
		for (t = 0; t <= 16; t++)
		{
			c = instr[i].p[t];
			if (c < 0)
				cc[abs(c)] = 1;
		}
	for (i = 256; i--;)
		if (cc[i + 1] == 0)
			o = i;
	return o;
}
//-------------------------------------------------------------127][128
int searchmaxseq(void)
{
	char o, x, y;
	for (y = 0; y < 127; y++)
		for (x = 0; x < 127; x++)
			if (seq[y][x])
				o = y;
	return o;
}
//---------------------------------------------------------------
int searchmaxtrk(void)
{
	char o, x, y;
	for (x = 0; x < 127; x++)
		for (y = 0; y < 127; y++)
			if (seq[y][x])
				o = x;
	return o;
}
//------------------------------------------------------rndmnu[]={L"type",L"note",L"width",L"numofpat",L"pauses",L"seed",L"minnotelen",L"repeat",L"random"};// main menu
//int rnd;
int random(int rnd)
{
	return (((33 + rnd) >> 2) + rnd) ^ 33;
}

void makerndpat(char *prm, int ppat)
{
	int rnd = prm[5], i, j;
	char width = prm[2] + prm[4];
	char note;
	for (j = ppat; j <= (ppat + prm[3]); j++)
	{
		for (i = 0; i <= 16; i++)
		{	//simple random fill :-(((
			rnd = random(rnd);
			note = rnd & 127;
			if (note > width)
				note &= width;
			if (note < prm[4])
				note = prm[4];
			note = note - prm[4] + prm[1];
			patterns[j].p[i] = note;
		}
		wcscpy(patterns[j].nme, L"rnd");
	}
	maxpatnames += prm[3];
}


//--------------------------------------------------------------------------------
void prep(unsigned char *Signal, int i)
{
	out[i].lpData = (char *)Signal;
	out[i].dwBufferLength = BufferSize;
	out[i].dwFlags = 0;
	out[i].dwLoops = 0;
	waveOutPrepareHeader(hWaveOut, &out[i], sizeof(out[0]));
}
//-------------------------------------------
//------------------------------
int playengine(void)
{
	int i;
	char c;
	int ff = GetTickCount();
	for (i = numofbuf; i--;)
	{
		c = snake[i];
		if (c == 1)
		{
			snake[i]--;
			memset(bf[i].bf, 0, BufferSize);
			fillshort((short *)bf[i].bf);
			waveOutWrite(hWaveOut, &out[i], sizeof(out[0]));
			scope = out[i].lpData;
		}
	}
	return GetTickCount() - ff;
}
//-----------------------------------------------------------
//-----------------------------------------------//---------------------------------------------//-------------------------------------------------------
void CALLBACK waveOutCB(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	int i;
	char c;	//wchar_t t[3]={L" \n\r"};
	if (uMsg == WOM_DONE)
	{
		playcell++;
		if (playcell > numofbuf)
			playcell = 0;
		for (i = numofbuf + 1; i--;)
		{
			c = snake[i];
			if (c == 0)
				continue;
			if (c == echobuf)
				if (i < numofbuf)
					snake[i + 1] = echobuf;
				else
					snake[0] = echobuf;
			snake[i]--;
		}
	}
	return;
}
//----------------------------------------------
void restartbuf(void)
{
	memset(snake, 1, numofbuf);
	for (int i = echobuf + 1; i--;)
		snake[i] = i;
}
//---------------------------------------------------------------------------------------------------
int initwaveout(HWND hwnd)
{
	wavefmtx.wFormatTag = WAVE_FORMAT_PCM;
	wavefmtx.nChannels = stereo;
	wavefmtx.nSamplesPerSec = frequency;
	wavefmtx.wBitsPerSample = bits;
	wavefmtx.nBlockAlign = (bits * stereo) >> 3;
	wavefmtx.nAvgBytesPerSec = (wavefmtx.nBlockAlign * wavefmtx.nSamplesPerSec);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wavefmtx, (unsigned long)waveOutCB, 0, CALLBACK_FUNCTION);
}
//----------------------------------------------------------------------------
void unprepout(unsigned char *Signal, int i)
{
	out[i].lpData = (char *)Signal;
	waveOutUnprepareHeader(hWaveOut, &out[i], sizeof(out[0]));
}
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
	divfreq = 20 * frequency / 1000;
	if (bpm == 0)
		bpm = 150;
	int beat = 15000 / bpm;
	beatlen = beat * wavefmtx.nSamplesPerSec / 1000;
	BufferSize = beat * 4 * wavefmtx.nBlockAlign * wavefmtx.nSamplesPerSec / 1000;
	fulb = BufferSize * numofbuf;
	b1 = malloc(fulb + BufferSize * 4);
	prep(b1, 0);
	bf[0].bf = b1;
	for (i = 1; i < numofbuf; i++)
	{
		bf[i].bf = b1 + BufferSize * i;
		prep(bf[i].bf, i);
	}
	scope = b1;

	reverbuf = (short *)malloc(beatlen * 8);

}
//-----------------------------------------------------------------------------------------
void freewave()
{
	int i;
	waveOutReset(hWaveOut);
	for (i = 0; i < numofbuf; i++)
		unprepout(bf[i].bf, i);
	free(b1);
	free(reverbuf);
}
//--------------
void newbpm(void)
{
	play = false;
	freewave();
	initwavbuf();
}
//------------------------------
void closewave()
{
	freewave();
	waveOutClose(hWaveOut);
}
void newsamplerate(void)
{
	play = false;
	closewave();
	initwaveout(hwnd);
	initwavbuf();
	restartbuf();
}
//-------------------------------------------------------------------------------------------------------
struct wavehdr44 {
	CHAR wavefile[4];
	DWORD size36;
	CHAR wavef[8];
	DWORD bits16;
	WORD word1;
	WORD channels;
	DWORD freq;
	DWORD Bytespersecond;	//dd    freq*chnls*bytesperchnl
	WORD Block_alignment;	//dw    chnls*bytesperchnl
	WORD Bits_per_sample;	//dw    bits
	CHAR dta[4];	//db    'data'
	DWORD Sizeofdata;	//dd    0;datasize
} ldwhdr , wavhdr =
{
	{
	'R', 'I', 'F', 'F'}, 0,
	{
	'W', 'A', 'V', 'E', 'f', 'm', 't', ' '}, 16, 1, stereo, 0, 0, (stereo * (bits / 8)), bits,
	{
'd', 'a', 't', 'a'}, 0};
//---------------------------------------------------------------------------
void loadwavfile(wchar_t *filename, char n)
{
	free(smp[n].s);	//memset(smp[n],0,sizeof(smp[n]));
	HANDLE hFile;
	short tmp[4000];
	DWORD dwNumberOfBytesRead;
	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ReadFile(hFile, &ldwhdr, 44, &dwNumberOfBytesRead, NULL);
	if (dwNumberOfBytesRead < 44 || ldwhdr.wavefile[0] != 'R' || ldwhdr.wavef[0] != 'W' || ldwhdr.dta[0] != 'd' || ldwhdr.bits16 != 16)
	{
		SetFilePointer(hFile, 0, 0, FILE_BEGIN);
		//memset(&smp[n],0,sizeof(smp[0]));
		if (smp[n].s != 0)
			free(smp[n].s);
		ReadFile(hFile, smp[n].speak, sizeof(smp[0].speak) - 1, &dwNumberOfBytesRead, NULL);
		wcscpy(smp[n].name, filename);
		if (!voice(smp[n].speak, n))
			wcscpy(smp[n].name, L"no smp");
		CloseHandle(hFile);
		return;
	}
	int p = 0, m = ldwhdr.Sizeofdata / 2;
	smp[n].s = (short *)malloc(ldwhdr.Sizeofdata);
	ReadFile(hFile, smp[n].s, ldwhdr.Sizeofdata, &dwNumberOfBytesRead, NULL);
	smp[n].l = m;
	smp[n].voice = false;
	smp[n].loop = false;
	CloseHandle(hFile);
}
//-----------------------------------------------------------
struct header {
	char sign[4];	//db 'Zm4\0' 
	wchar_t name[20];
	char sizeofinstrument;
	char sizeofpattern;
	char numofinstruments;
	char numofpatterns;
	char numoftracks;
	char numofrows;
	char reserved0;
	char bpm;
	char numofsamples;
	ULONG instruments;
	ULONG patterns;
	ULONG tracks;
	ULONG samples;
	ULONG frequency;
} hdr;
int shdr = sizeof(hdr);
//------------------------------------------------------------------------------======================================---------
void loadfile(wchar_t *filename)
{
	HANDLE hFile;
	DWORD dwNumberOfBytesRead;
	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ReadFile(hFile, &hdr, shdr, &dwNumberOfBytesRead, NULL);
	if (dwNumberOfBytesRead < shdr)
	{
		CloseHandle(hFile);
		return;
	}
	int instrsize = sizeof(instr[0]) * hdr.numofinstruments;
	maxinstr = hdr.numofinstruments;
	frequency = hdr.frequency;
	if (frequency == 0)
		frequency = 22000;
	bpm = hdr.bpm;
	if (bpm == 0)
		bpm = 150;
	ReadFile(hFile, &instr, instrsize, &dwNumberOfBytesRead, NULL);
	if (hdr.sign[2] == '4')
		version = 4;
	else
		version = 5;	//for(int i=0;i<=hdr.numofinstruments;i++){char c=instr[i].p[3]; if(c>0)instr[i].p[3]=0;}
	instrsize = sizeof(instr[0]) * hdr.numofpatterns;
	maxpatnames = hdr.numofpatterns;
	ReadFile(hFile, &patterns, instrsize, &dwNumberOfBytesRead, NULL);
	maxseq = hdr.numofrows;
	maxtrack = hdr.numoftracks - 1;
	for (int i = 0; i <= maxseq; i++)
		ReadFile(hFile, &seq[i][0], hdr.numoftracks, &dwNumberOfBytesRead, NULL);
	maxsamples = hdr.numofsamples;
	if (maxsamples > 0)
		for (int i = 0; i <= maxsamples - 1; i++)
		{
			ReadFile(hFile, &smp[i], sizeof(smp[0]), &dwNumberOfBytesRead, NULL);
			if (smp[i].voice)
				voice(smp[i].speak, i);
			else
			{
				int m = smp[i].l;
				smp[i].s = (short *)malloc(m * 2);
				ReadFile(hFile, smp[i].s, m * 2, &dwNumberOfBytesRead, NULL);
			}	/// critical section
		}
	CloseHandle(hFile);
	newsamplerate();
}
//===============================================--------------------------------------------------------------------------
void savefile(wchar_t *filename)
{
	HANDLE hFile;
	searchmaxseq();
	searchmaxtrk();
	DWORD dwNumberOfBytesRead;
	hFile = CreateFile(filename, GENERIC_READ + GENERIC_WRITE, FILE_SHARE_READ + FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	hdr.frequency = frequency;
	hdr.numofinstruments = maxinstr;
	hdr.numofpatterns = maxpatnames;
	hdr.numofrows = maxseq;
	hdr.numoftracks = maxtrack + 1;
	hdr.numofsamples = maxsamples;
	hdr.sign[0] = 'z';
	hdr.sign[1] = 'm';
	if (version == 4)
		hdr.sign[2] = '4';
	else
		hdr.sign[2] = '5';
	hdr.bpm = bpm;
	int instrsize = sizeof(instr[0]) * hdr.numofinstruments;
	int patsize = sizeof(patterns[0]) * hdr.numofpatterns;
	hdr.instruments = shdr;	//!!!!!!!!!!!!
	hdr.patterns = shdr + instrsize;	//!!!!!!!
	WriteFile(hFile, &hdr, shdr, &dwNumberOfBytesRead, NULL);
	WriteFile(hFile, &instr, instrsize, &dwNumberOfBytesRead, NULL);
	WriteFile(hFile, &patterns, patsize, &dwNumberOfBytesRead, NULL);
	for (int i = 0; i <= maxseq; i++)
		WriteFile(hFile, &seq[i][0], hdr.numoftracks, &dwNumberOfBytesRead, NULL);
	if (maxsamples > 0)
		for (int i = 0; i <= maxsamples - 1; i++)
		{
			if (smp[i].s == 0)
				break;
			WriteFile(hFile, &smp[i], sizeof(smp[0]), &dwNumberOfBytesRead, NULL);
			if (!smp[i].voice)
				WriteFile(hFile, smp[i].s, smp[i].l * 2, &dwNumberOfBytesRead, NULL);	//!!!!
		}
	CloseHandle(hFile);
}
//-----------=====================--------------------------------
void mergefile(wchar_t *filename)
{
	HANDLE hFile;
	DWORD dwNumberOfBytesRead;
	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ReadFile(hFile, &hdr, shdr, &dwNumberOfBytesRead, NULL);
	if (dwNumberOfBytesRead < shdr)
	{
		CloseHandle(hFile);
		return;
	}
	int instrsize = sizeof(instr[0]) * hdr.numofinstruments;
	if (bpm == 0)
		bpm = 150;
	ReadFile(hFile, &instr[maxinstr], instrsize, &dwNumberOfBytesRead, NULL);
//  if(hdr.sign[2]=='4')for(int i=0;i<=hdr.numofinstruments;i++){char c=instr[i+maxinstr].p[3]; if(c>0)instr[i].p[3]=0;}
	instrsize = sizeof(instr[0]) * hdr.numofpatterns;
	ReadFile(hFile, &patterns[maxpatnames], instrsize, &dwNumberOfBytesRead, NULL);
	for (int i = 0; i <= hdr.numofrows; i++)
		ReadFile(hFile, &seq[i + maxseq + 1][maxtrack + 1], hdr.numoftracks, &dwNumberOfBytesRead, NULL);
	for (int i = 0; i <= hdr.numofinstruments; i++)
		for (int y = 0; y < 16; y++)
		{
			char c = instr[maxinstr + i].p[y];
			if (c < 0)
				instr[maxinstr + i].p[y] = c - maxtrack - 1;
		}
	for (int y = 0; y <= hdr.numofrows; y++)
		for (int x = 0; x < hdr.numoftracks; x++)
			if (seq[y + maxseq + 1][x + maxtrack + 1])
				seq[y + maxseq + 1][x + maxtrack + 1] += maxpatnames;
	maxinstr += hdr.numofinstruments;
	maxpatnames += hdr.numofpatterns;
	maxseq += hdr.numofrows + 1;
	maxtrack += hdr.numoftracks - 1;
	int t;
	if (hdr.numofsamples > 0)
		for (int i = 0; i <= hdr.numofsamples - 1; i++)
		{
			t = i + maxsamples;
			ReadFile(hFile, &smp[t], sizeof(smp[0]), &dwNumberOfBytesRead, NULL);
			if (smp[t].voice)
				voice(smp[t].speak, i);
			else
			{
				int m = smp[t].l;
				smp[t].s = (short *)malloc(m * 2);
				ReadFile(hFile, smp[t].s, m * 2, &dwNumberOfBytesRead, NULL);
			}	/// critical section
		}
	maxsamples += hdr.numofsamples;
	CloseHandle(hFile);
}
//===========================================================---------------------------------------------------
HBITMAP NewDibSection(long width, long height, int bitCount)
{
	BITMAPINFO *pbmi;
	pbmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER));
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = width;
	pbmi->bmiHeader.biHeight = height;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = bitCount;
	pbmi->bmiHeader.biCompression = BI_RGB;
	BYTE *rawBits;
	HBITMAP dibSec = CreateDIBSection(NULL, pbmi, DIB_RGB_COLORS, (void **)&rawBits, NULL, 0);
	//free(pbmi);
	return dibSec;
}
//---------------------------------------------------------------
void createvirt(HDC hdc)
{
	hdcMem = CreateCompatibleDC(hdc);
	hDIBResult = NewDibSection(sx, sy, 24);
	GetObject(hDIBResult, sizeof(BITMAP), &rDIB);
	dstDIBBits = (BYTE *) (rDIB.bmBits);
	oldBmp = SelectObject(hdcMem, hDIBResult);
	dstRowBytes = (((rDIB.bmWidth * rDIB.bmBitsPixel + 31) & (~31)) / 8);
}
//--------------------------------
//----------------------------------
void drawcolorrec(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
	HBRUSH hBrush = CreateSolidBrush(c);	//RGB(210, 55, 205));
	SelectObject(hdcMem, hBrush);
	Rectangle(hdcMem, x1, y1, x2, y2);
	DeleteObject(hBrush);
}
//----------------------------------
void drawcolorelipse(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
	HBRUSH hBrush = CreateSolidBrush(c);	//RGB(210, 55, 205));
	SelectObject(hdcMem, hBrush);
	Ellipse(hdcMem, x1, y1, x2, y2);
	DeleteObject(hBrush);
}
//---------------------------
void cls(char c)
{
	memset(dstDIBBits + 15 * dstRowBytes, c, (sy - 16) * dstRowBytes);
}
//----------------------------
void plot(short x, short y, char r, char g, char b)
{
	if (x < 0 || y < 0 || x > sx || y > sy)
		return;
	char *a = &dstDIBBits[x + x + x + y * dstRowBytes];
	*a++ = r;
	*a++ = g;
	*a = b;
}
//-----------------------------------------------
PVOID getpixel(short x, short y, char *r, char *g, char *b)
{
	if (x < 0 || y < 0 || x > sx || y > sy)
		return dstDIBBits;
	char *a = &dstDIBBits[x + x + x + y * dstRowBytes];
	*r = *a++;
	*g = *a++;
	*b = *a++;
	return a;
}
//----------------------------------------------------
void drawinvertbox(int x, int y, int xsize, int ysize, char c, char t, char n)
{
	int u, v;
	char r, g, b;
	for (v = ysize; v--;)
		for (u = xsize; u--;)
		{
			getpixel(x + u, y + v, &r, &g, &b);
			r = c - r;
			g = t - g;
			b = n - b;
			plot(x + u, y + v, r, g, b);
		}
}
//-----------------------------------------------
void horline(int x, int y, char c)
{
	if (y < 0 || y > sy)
		return;	//<hr>
	memset(dstDIBBits + y * dstRowBytes + x * 3, c, dstRowBytes - x * 3);
}
//=============
void drawscope(int x, int y, int amp)
{
	StretchBlt(hdcMem, 0, 16, sx, sy, hdcMem, 0, 16, sx - 16, sy - 26, SRCCOPY);
	short *s = (short *)scope;
	char r = *s, g = *(s + 1), b = *(s + 2);
	for (; x--;)
	{
		int u = y + *(s++) / amp;
		plot(x, u, r, g, b);
	}
	scope += sx * 2;
}
//=============
//--------------------------------------------------пианино------------------------------------------------клавиатура---------------------
void movenotes(int e, int n)
{
	int i;
	char c;
	for (i = 0; i < 16; i++)
	{
		c = patterns[e].p[i];
		if (c > 2)
			c += n;
		if (c < 0)
			c -= n;
		patterns[e].p[i] = c;
	}
}
const int maxoctaves = 140 / 12;
//----------------------------------------------------------
int drawkeybvert(HDC hdc, int len, int size, int maxs, int maxx, int offset, MSG m)
{
	cls(100);
	if (m.message == WM_KEYUP)
	{
		if (m.wParam == 'u')
			movenotes(editpattern, 1);
		if (m.wParam == 'v')
			movenotes(editpattern, -1);

		char ccc = m.wParam;
		if (ccc >= '0' && ccc <= '9')
			octave = ccc - '0';
	}
	int w, s = size / 2;
	int i = 0;
	int o = 3;
	int p = 0;
	int kx, ks, t, syy = sy - 15;
	wchar_t txt[50];

	for (w = 0; w < maxs; w += size)
	{
		keytab[i++] = w;
		horline(len + 1, w + 13, 50);
		drawcolorrec(hdc, 0, syy - w, len, syy - w - size + 1, RGB(254, 250, 200));	//white key
		if (--o != 0)
		{
			keytab[i++] = (w + s) | 0x8000;
			drawcolorrec(hdc, len / 2, syy - w - s, 0, syy - w - s - size, RGB(150, 0, 0));	//black key
		}
		else
			o = 3 + (++p & 1);
	}
	int l = len + 2;
	int ss = ((maxx - l) / (maxnotepat));
	int octaveperscr = maxoctaves - i / 12;
	if (move)
	{
		ks = syy - ty;
		selcol = (tx / ss) - 2;
		for (t = i; t--;)
		{
			kx = keytab[t];
			if (ks > kx && ks < (kx + size))
				keysel = t;
		}
	}
//select key
	if ((keysel += gy) < 0)
	{
		keysel = 0;
		if (octave > 0)
			octave--;
	}
	if (keysel > i)
	{
		keysel = i - 1;
		if (octave < octaveperscr)
			octave++;
	}
	t = keytab[keysel];
	gy = 0;
	p = 2 + (len / 2) * (t < 0x8000);
	t &= 0xfff;
	drawcolorelipse(hdc, p, syy - t, p + size - 1, syy - t - size + 1, RGB(0, 200, 0));	//draw pressed key
//select X column <- ->
	if ((selcol += gx) < (gx = 0))
		selcol = maxnotepat - 1;	//select column
	selcol *= (selcol < maxnotepat);
//if button pressed
	int yyy = keysel + offset;
	if (fire)
		if (patterns[editpattern].p[selcol] == yyy)
			patterns[editpattern].p[selcol] = 0;
		else
			patterns[editpattern].p[selcol] = yyy;
	fire = FALSE;
//draw columns & notes
//calc column
	t = -1;
	int z = 0;
	for (w = l; (w += ss) < maxx; patx[z++] = w)
	{
		if (t++ >= notincol - 1)
			drawcolorrec(hdc, w - ss, 0, w + 1 - ss, maxs, t = 0);	//vert line
		if (selcol == z)
			drawcolorrec(hdc, w - ss, 0, w, maxs, RGB(70, 20, 50));	//vert sel line
		p = patterns[editpattern].p[z];
		if (p > offset)
			p = p - offset;
		else
			p = 0;
		if ((p > 0) && (p < i) && (z < (maxnotepat)))
			drawcolorelipse(hdc, w - ss + 1, syy - (keytab[p] & 0xfff), w - 1, syy - (0xfff & keytab[p] + size - 1), RGB(70, 200, 30));	//draw note
	}
	wsprintf(txt, L"%u", octave);
	print(txt, sx - 7, 1);
	wsprintf(txt, L"  %d ", yyy);
	print(txt, sx / 2 - 8 * 4, sy - 13);
	return i;
}
//-----------------------------------------------

//---------------------------------------------------------
int drawkeybvert1(HDC hdc, int len, int size, int maxs, int maxx, int offset, MSG m)
{
	cls(100);
	if (m.message == WM_KEYUP)
	{
		char ccc = m.wParam;
		if (ccc >= '0' && ccc <= '9')
			octave = ccc - '0';
	}
	int w, s = size / 2;
	int i = 0;
	int o = 3;
	int p = 0;
	int t, syy = sy - 15;
	wchar_t txt[50];

	for (w = 0; w < maxs; w += size)
	{
		keytab[i++] = w;
		horline(len + 1, sy - w, 50);	//drawcolorrec(hdc,len+1,w,maxx,w-1,RGB(10,90,200));//horziont line
		drawcolorrec(hdc, 0, w, len, w + size - 1, RGB(254, 250, 200));	//white key
		if (--o != 0)
		{
			keytab[i++] = (w + s) | 0x8000;
			drawcolorrec(hdc, len / 2, w + s, len, w + s + size - 1, RGB(150, 0, 0));	//black key
		}
		else
			o = 3 + (++p & 1);
	}
//select key
	int octaveperscr = maxoctaves - i / 12;
	if ((keysel += -gy) < 0)
	{
		keysel = 0;
		if (octave > 0)
			octave--;
	}
	if (keysel > i)
	{
		keysel = i - 1;
		if (octave < octaveperscr)
			octave++;
	}
	t = keytab[keysel];
	gy = 0;
	p = 2 + (len / 2) * (t >= 0x8000);
	t &= 0xfff;
	drawcolorelipse(hdc, p, t, p + size - 1, t + size - 1, RGB(0, 200, 0));	//draw pressed key
//select X column <- ->
	if ((selcol += gx) < (gx = 0))
		selcol = maxnotepat - 1;	//select column
	selcol *= (selcol < maxnotepat);
//if button pressed
	int yyy = keysel + offset;
	if (fire)
		if (patterns[editpattern].p[selcol] == yyy)
			patterns[editpattern].p[selcol] = 0;
		else
			patterns[editpattern].p[selcol] = yyy;
	fire = FALSE;
//draw columns & notes
//calc column
	int l = len + 2;
	int ss = ((maxx - l) / (maxnotepat));
	t = -1;
	int z = 0;
	for (w = l; (w += ss) < maxx; patx[z++] = w)
	{
		if (t++ >= notincol - 1)
			drawcolorrec(hdc, w - ss, 0, w + 1 - ss, maxs, t = 0);	//vert line
		if (selcol == z)
			drawcolorrec(hdc, w - ss, 0, w, maxs, RGB(70, 20, 50));	//vert sel line
		p = patterns[editpattern].p[z];
		if (p > offset)
			p = p - offset;
		else
			p = 0;
		if ((p > 0) && (p < i) && (z < (maxnotepat)))
			drawcolorelipse(hdc, w - ss + 1, keytab[p] & 0xfff, w - 1, 0xfff & keytab[p] + size - 1, RGB(70, 200, 30));	//draw note
	}
	wsprintf(txt, L"%u", octave);
	print(txt, sx - 7, 1);
	wsprintf(txt, L"  %d ", yyy);
	print(txt, sx / 2 - 8 * 4, sy - 13);
	return i;
}
//-----------------------------------------------
void drawboxmenu(int width, int num)
{
	int z = num * 14;
	memset(dstDIBBits, 0, z * dstRowBytes);
}
//----------------------------
//--------------------------------------------------выбор --------------------------------
void drawtab(struct charsnames *in, int maxnames, _Bool more)
{	//listbox
	int y, o = taboffset, b = (sy - 15 * 2) / 16;
	wchar_t txt[50];
	char z;
	maxnames += more - 1;
	if (gy > 0)
		if (o > 0)
			o--;
	if ((keysel += -gy) < (gy = 0))
	{
		keysel = 0;
		if (o > 0)
			o--;
	}
	if ((keysel - o) > b)
	{
		keysel = b + o;
		o++;
	}
	if (keysel > maxnames)
	{
		keysel = maxnames;
		o--;
	}
	if (maxnames < b)
		o = 0;
	for (int i = 0; i <= maxnames; i++)
	{
		y = i * 16;
		horline(1, sy - y - 16, 0);
		if ((i + o) == keysel)
			drawcolorrec(hdcMem, 3, y, 3 + 7 * 8, y + 16, RGB(70, 20, 50));
		if ((i + o) >= maxnames)
			print(L"....", 5, 2 + y);
		else
		{
			print(in[i + o].nme, 5, 2 + y);
			for (int x = 0; x < 16; x++)
			{
				z = in[i + o].p[x];
				plot(x + 8 * 8, sy - y - (z >> 3), 132, 0, 10);
				if (z < 0)
				{
					wsprintf(txt, L".%u.", abs(z));
					print(txt, x * (sx / 24) + 8 * 8, y);
				}
			}
		}
	}
	taboffset = o;
}
//---------------------------------------
//--------------------------------------------------список инструментов --------------------------------
void drawinstrs(MSG m)
{	//listbox
	cls(253);
	wchar_t txt[100];
	if (move)
	{
		keysel = (ty / 16) - 1;
	}
//if(m.message==WM_LBUTTONDBLCLK){ xseq=LOWORD(m.lParam)/(8*4);yseq=(HIWORD(m.lParam)/16)-1;fire=true;}

	drawtab(instr, maxinstr, true);
	if (m.message == WM_KEYUP)
	{	//edit name
		if (m.wParam != VK_RETURN && m.wParam != VK_UP && m.wParam != VK_DOWN)
		{
			instr[keysel].nme[editchar++] = (char)m.wParam;
			//instr[keysel].nme[editchar]='.';
			editchar &= 7;
		}
	}
	if (fire)
	{	//select pattern
		editinstr = keysel;
		memset(txt, 0, 10);
		txt[0] = L'0' + (maxinstr & 0xf);
		if (wcslen(instr[keysel].nme) < 1)
		{
			wcscpy(instr[keysel].nme, L"ins");
			wcscat(instr[keysel].nme, txt);
		}
		if (keysel >= maxinstr)
		{
			instr[keysel].p[4] = 20;
			instr[keysel].p[1] = -1 - searchfreechn();
			maxinstr++;
		}
		mode = 10;
	}
	fire = false;
}
//---------------------------------------
//--------------------------------------------------список паттернов --------------------------------
void drawsheet(MSG m)
{	//listbox
	cls(250);
	wchar_t txt[100];
	drawtab(patterns, maxpatnames, true);
	if (m.message == WM_KEYUP)
	{	//edit name
		if (m.wParam != VK_RETURN && m.wParam != VK_UP && m.wParam != VK_DOWN && m.wParam != VK_RIGHT)
		{
			patterns[keysel].nme[editchar++] = (char)m.wParam;
			patterns[keysel].nme[editchar] = '.';
			editchar &= 7;
		}
		if (m.wParam == VK_RIGHT)
			memcpy(&patterns[maxpatnames++], &patterns[keysel], sizeof(patterns[0]));

	}
	if (fire)
	{	//select pattern
		editpattern = keysel;
		memset(txt, 0, 10);
		txt[0] = L'0' + (maxpatnames & 0xf);
		if (!onlyselectpattern)
		{
			if (wcslen(patterns[keysel].nme) < 1)
			{
				wcscpy(patterns[keysel].nme, L"pat");
				wcscat(patterns[keysel].nme, txt);
			}
			if (keysel >= maxpatnames)
				maxpatnames++;
		}
		if (editpattern >= maxpatnames)
			editpattern = -1;
		if (onlyselectpattern)
			mode = onlyselectpattern;
		else
			mode = 13;
	}
	fire = false;
}
//---------------------------------------
void movesqdn(int x, int y)
{
	int i;
	if (y > maxseq)
		return;
	if (seq[maxseq][x] != 0)
		maxseq++;
	for (i = maxseq; i > y; i--)
		seq[i][x] = seq[i - 1][x];
	seq[i][x] = 0;
}
void movesqup(int x, int y)
{
	int i;
	if (y > maxseq)
		return;
	for (i = y; i < maxseq; i++)
		seq[i][x] = seq[i + 1][x];
	seq[maxseq][x] = 0;
}
//------------------------------------основной список и расстановка паттернов--------------------------

void drawseq(MSG m)
{
	int a = 1 + (sx / (8 * 4)), b = (sy - 15 * 2) / 16, u, v;
	char c;
	wchar_t t[10];
	cls(250);
	if (move)
	{
		xseq = tx / (8 * 4);
		yseq = (ty / 16) - 1;
	}
	if (m.message == WM_KEYUP)
	{
		if (m.wParam == 'v')
			movesqdn(xseq + xofs, yseq + yofs);
		if (m.wParam == 'u')
			movesqup(xseq + xofs, yseq + yofs);
	}
	for (u = 1; u < (b + 1); u++)
	{
		wsprintf(t, L"%u", u + yofs);
		print(t, sx - 10, u * 16);
	}
	drawinvertbox(sx - 11, 16, 11, sy - 20, 255, 33, 255);
	for (u = 1; u < a; u++)
	{
		wsprintf(t, L"%u", u + xofs);
		print(t, (u - 1) * 8 * 4, 0);
	}
	drawinvertbox(0, sy - 16, sx, 16, 255, 33, 255);
	for (v = 0; v < b; v++)
		for (u = 1; u < a; u++)
		{
			c = seq[v + yofs][u - 1 + xofs];
			if (c == 0)
				print(L"----", (u - 1) * 8 * 4, 16 + v * 16);
			if (c > 0)
			{
				memset(t, 0, sizeof(t));
				c--;
				if (wcslen(patterns[c].nme) > 0)
					memcpy(t, patterns[c].nme, 8);
				print(t, (u - 1) * 8 * 4, 16 + v * 16);
			}
			horline(0, sy - (16 + v * 16), 0);
		}
	for (u = 2; u < a; u++)
		for (v = sy - 15; v--;)
			plot((u - 1) * 8 * 4 - 1, v + 15, 0, 0, 0);
	if ((yseq += -gy) < (gy = 0))
	{
		yseq = 0;
		if (yofs > 0)
			yofs--;
	}
	if (yseq > b - 1)
	{
		yseq = b - 1;
		yofs++;
	}
	if ((xseq += gx) < (gx = 0))
	{
		xseq = 0;
		if (xofs > 0)
			xofs--;
	}
	if (xseq > a - 2)
	{
		xseq = a - 2;
		xofs++;
	}
	drawinvertbox(xseq * 4 * 8, sy - yseq * 16 - 32, 8 * 4, 16, 255, 255, 255);
	c = seq[yseq][xseq];
	if (onlyselectpattern)
	{
		onlyselectpattern = 0;
		c = editpattern + 1;
		seq[yseq + yofs][xseq + xofs] = c;
		if ((yseq + yofs) > maxseq)
			maxseq = yseq + yofs;
		if ((xseq + xofs) > maxtrack)
			maxtrack = xseq + xofs;
	}	//selected pattern=editpattern here;
	if (fire)
	{
		editpattern = c - 1;
		onlyselectpattern = 6;
		mode = 5;
	}
	fire = false;
}
//--------------------------------------------------список samples --------------------------------
void drawsmp(MSG m)
{	//listbox
	cls(253);
	wchar_t txt[100];
	int y, o = taboffset, b = (sy - 15 * 2) / 16;
	int max = maxsamples;
	if ((keysel += -gy) < (gy = 0))
	{
		keysel = 0;
		if (o > 0)
			o--;
	}
	if ((keysel - o) > b)
	{
		keysel = b + o;
		o++;
	}
	if (keysel > max)
	{
		keysel = max;
		o--;
	}
	if (max < b)
		o = 0;

	for (int i = 0; i <= max; i++)
	{
		y = i * 16;
		horline(1, sy - y - 16, 0);
		wsprintf(txt, L"%u ", i + o + 10);
		print(txt, 0, 2 + y);
		int z = i + o;
		if ((z) == keysel)
			drawcolorrec(hdcMem, 13, y, 3 + 7 * 8, y + 16, RGB(70, 20, 50));
		if ((z) >= max)
			print(L"....", 15, 2 + y);
		else
		{
			print(smp[z].name, 15, 2 + y);
			if (smp[z].voice)
				print(L"V", 15 + 8 * 10, 2 + y);
			if (smp[z].loop)
				print(L"loop", 15 + 8 * 14, 2 + y);
			else
				print(L"noloop", 15 + 8 * 14, 2 + y);
		}
	}
	if (m.message == WM_KEYUP)
	{	//edit name
		if (m.wParam != VK_RIGHT && m.wParam != VK_LEFT && m.wParam != VK_RETURN && m.wParam != VK_UP && m.wParam != VK_DOWN)
		{
			smp[keysel].name[editchar++] = (char)m.wParam;
			editchar &= 7;
		}
		if (m.wParam == VK_LEFT)
		{
			print(L"    Deleted   ", 8 * 3, keysel * 16 + 2);
			free(smp[keysel].s);
			memset(&smp[keysel], 0, sizeof(smp[0]));
			if ((keysel == max - 1) || keysel == max)
				maxsamples--;
		}
		if (m.wParam == VK_RIGHT)
		{
			smp[keysel].loop = !smp[keysel].loop;
		}

	}
	if (fire)
	{
		editinstr = keysel;
		memset(txt, 0, 10);
		txt[0] = L'0' + (max & 0xf);
		if (wcslen(smp[keysel].name) < 1)
		{
			wcscpy(smp[keysel].name, L"smp");
			wcscat(smp[keysel].name, txt);
		}
		if (keysel >= max)
			maxsamples++;
		smp[maxsamples].voice = true;
		maxfilelist = 0;
		mode = 16;
	}
	fire = false;
}
//---------------------------------------
//-----------------------------------------------редактор инструмента--- изменяемые параметры---------------
//---------------
_Bool paramsign = false;
int xtmp = 0;
void drawparams(void)
{
	cls(15);
	wchar_t txt[100];
	int startx, dx, l, x, y, i, ysize = (sy - 15) / paramscount;
	char lev;
	startx = 8 * 8;
	dx = (sx - 8 - startx);
	y = ysize / 2;
	if (move)
		keysel = ty / ysize;

	if ((keysel += -gy) < (gy = 0))
		keysel = paramscount - 1;
	keysel *= (keysel < paramscount);
	i = keysel;
	if (xtmp == gx)
		gx = 0;
	if (abs(gx) > 5)
		gx = xtmp;
	xtmp = gx;	// ускорение
	paramsign = instr[editinstr].p[i] < 0;
	lev = abs(instr[editinstr].p[i]);
	if (fire)
		paramsign = !paramsign;
	fire = false;
	if ((lev += gx) < 0)
		lev = 126;	//select column
	lev *= (lev < 127);
	if (move)
		lev = abs(127 * (tx - startx) / (sx - startx)) & 127;

	if (paramsign)
		lev = -lev;
	instr[editinstr].p[i] = lev;
	for (i = 0; i < paramscount; i++)
	{
		lev = instr[editinstr].p[i];
		print(params[i].mnu, 1, i * ysize);
		if (i == keysel)
			drawcolorrec(hdcMem, startx - 3, y + i * ysize - 3, sx - 3, y + i * ysize + y + 3, RGB(10, 20, 210));
		drawcolorrec(hdcMem, startx, y + i * ysize, sx - 8, y + i * ysize + y, RGB(100, 200, 250 * (lev < 0)));
		for (l = ysize; l--;)
		{
			plot(startx + 1 + abs(dx * lev) / 127, 1 + l + (sy - 15) - (i * ysize), 214, 215, 199);
		}
	}
	print(params[keysel].mnu, 1, keysel * ysize);
	if (keysel == 11)
	{
		lev = instr[editinstr].p[keysel];
		if (lev > 15 && lev < 24)
			print(L"reverb.hpf", 1, keysel * ysize);
		if (lev > 23 && lev < 32)
			print(L"reverb.reso", 1, keysel * ysize);
	}
	if (keysel == 15)
	{
		lev = instr[editinstr].p[keysel];
		if (lev > 100)
			print(L"disthort", 1, keysel * ysize);
	}
	if (keysel == 0)
	{
		lev = instr[editinstr].p[keysel];
		if (lev == 0)
			print(L"sinus", 1, keysel * ysize);
		if (lev == 1)
			print(L"triangle", 1, keysel * ysize);
		if (lev == 2)
			print(L"square", 1, keysel * ysize);
		if (lev == 3)
			print(L"noice", 1, keysel * ysize);
		if (lev == 4)
			print(L"saw", 1, keysel * ysize);
		if (lev == 5)
			print(L"saw*saw", 1, keysel * ysize);
		if (lev == 6)
			print(L"sqr*sin", 1, keysel * ysize);
		if (lev > 9)
			print(smp[lev - 10].name, 1, keysel * ysize);
	}

	drawinvertbox(0, sy - keysel * ysize - 16, 8 * 8, 15, 55, 255, 255);
	wsprintf(txt, L" %u  _", abs(instr[editinstr].p[keysel]));
	print(txt, sx / 2 - 8 * 4, sy - 14);
}
//-------------------------------------------------------------
//-----------------------------------------------редактор rnd--- изменяемые параметры---------------
char rndgen[33];
void drawrandom(void)
{
	cls(5);
	wchar_t txt[100];
	int prcnt = sizeof(rndmnu) / sizeof(rndmnu[0]);
	int startx, dx, l, x, y, i, ysize = (sy - 15) / 14;
	char lev;
	startx = 8 * 8;
	dx = (sx - 8 - startx);
	y = ysize / 2;	//if(sy>200) y=16;
	if ((keysel += -gy) < (gy = 0))
		keysel = prcnt - 1;
	keysel *= (keysel < prcnt);
	i = keysel;
	if (xtmp == gx)
		gx = 0;
	if (abs(gx) > 5)
		gx = xtmp;
	xtmp = gx;	// ускорение
	lev = abs(rndgen[i]);
	if ((lev += gx) < 0)
		lev = 126;	//select column
	lev *= (lev < 127);
	rndgen[i] = lev;
	for (i = 0; i < prcnt; i++)
	{
		lev = rndgen[i];
		print(rndmnu[i].mnu, 1, i * ysize);
		if (i == keysel)
			drawcolorrec(hdcMem, startx - 3, y + i * ysize - 3, sx - 3, y + i * ysize + y + 3, RGB(10, 20, 210));
		drawcolorrec(hdcMem, startx, y + i * ysize, sx - 8, y + i * ysize + y, RGB(100, 200, 250 * (lev < 0)));
		for (l = ysize; l--;)
		{
			plot(startx + 1 + abs(dx * lev) / 127, 1 + l + (sy - 15) - (i * ysize), 214, 215, 199);
		}
	}
	print(rndmnu[keysel].mnu, 1, keysel * ysize);
	drawinvertbox(0, sy - keysel * ysize - 16, 8 * 8, 15, 55, 255, 255);
	wsprintf(txt, L" %u  _", abs(rndgen[keysel]));
	print(txt, sx / 2 - 8 * 4, sy - 14);
	if (fire)
		makerndpat(rndgen, maxpatnames);

}
//-------------------------------------------------------------


//-----------------------------------------------------------
int fillfiles(int first, int max, wchar_t *filt)
{
	WIN32_FIND_DATA fil;
	memset(filelist, 0, sizeof(filelist));
	int i;
	HANDLE f = FindFirstFile(filt, &fil);
	for (i = 0; i <= max + first; i++)
	{
		if (i >= first)
			wcscpy(filelist[i - first], fil.cFileName);
		if (!FindNextFile(f, &fil))
			break;
	}
	FindClose(f);
	return i;
}
//------------------
void findzm4(int s)
{
	wchar_t txt[255];
	wcscpy(txt, selfdir);
	wcscat(txt, L"\*.zm4");
	maxfilelist = fillfiles(s, (sy - 40) / 16, txt) - s;
	mode = 14;
}
//----------------------------------------------------------------------
void mergezm4(int s)
{
	wchar_t txt[255];
	wcscpy(txt, selfdir);
	wcscat(txt, L"\*.zm4");
	maxfilelist = fillfiles(s, (sy - 40) / 16, txt) - s;
	mode = 17;
}
//---------------
void findall(int s)
{
	wchar_t txt[255];
	wcscpy(txt, selfdir);
	wcscat(txt, L"\*.wav");
	maxfilelist = fillfiles(s, (sy - 40) / 16, txt) - s;
	mode = 16;
}
//---------------
void findmid(int s)
{
	wchar_t txt[255];
	wcscpy(txt, selfdir);
	wcscat(txt, L"\*.mid");
	maxfilelist = fillfiles(s, (sy - 40) / 16, txt) - s;
	mode = 19;
}
//----------------------------------------------------------------------
void drawfilelist(MSG n)
{
	cls(234);
	_Bool t;
	int i, x, y, m;
	if (move)
		keysel = ty / 16;
	if ((keysel += -gy) < (gy = 0))
	{
		keysel = 0;
		if (yofs > 0)
			yofs--;
		findzm4(yofs);
	}
	if (keysel >= maxfilelist)
	{
		keysel = 0;
		yofs += maxfilelist;
		findzm4(yofs);
	}
	m = keysel + yofs;
	for (i = 0; i <= maxfilelist; i++)
	{
		print(filelist[i], 1, i * 16);
		if (i == keysel)
			drawinvertbox(0, sy - i * 16 - 16, 8 * 10, 17, 55, 255, 255);
	}
	if (fire)
	{
		yofs = 0;
		erasemusic();
		wcscpy(editfile, selfdir);
		wcscat(editfile, filelist[keysel]);
		loadfile(editfile);
		t = play;
		newbpm();
		restartbuf();
		play = true;	//// воспроизводим как только загружаем файл
		mode = 6;
	}
	fire = false;
}
//----------------------------------------------------------------------
void drawmergefile(MSG n)
{
	cls(54);
	_Bool t;
	wchar_t editfil[255];
	int i, x, y, m;
	if (move)
		keysel = ty / 16;
	if ((keysel += -gy) < (gy = 0))
	{
		keysel = 0;
		if (yofs > 0)
			yofs--;
		mergezm4(yofs);
	}
	if (keysel >= maxfilelist)
	{
		keysel = 0;
		yofs += maxfilelist;
		mergezm4(yofs);
	}
	m = keysel + yofs;
	for (i = 0; i <= maxfilelist; i++)
	{
		print(filelist[i], 1, i * 16);
		if (i == keysel)
			drawinvertbox(0, sy - i * 16 - 16, 8 * 10, 17, 55, 255, 255);
	}
	if (fire)
	{
		yofs = 0;	//erasemusic();
		wcscpy(editfil, selfdir);
		wcscat(editfil, filelist[keysel]);
		mergefile(editfil);
		t = play;
		newbpm();
		restartbuf();
		play = t;
		mode = 6;
	}
	fire = false;
}

//-------------------------------------------------------------
void drawwavtxtlist(void)
{
	cls(234);
	_Bool t;
	wchar_t tmp[255];
	int i, x, y, m;
	if ((keysel += -gy) < (gy = 0))
	{
		keysel = 0;
		if (yofs > 0)
			yofs--;
		findall(yofs);
	}
	if (keysel >= maxfilelist)
	{
		keysel = 0;
		yofs += maxfilelist;
		findall(yofs);
	}
	m = keysel + yofs;
	for (i = 0; i <= maxfilelist; i++)
	{
		print(filelist[i], 1, i * 16);
		if (i == keysel)
			drawinvertbox(0, sy - i * 16 - 16, 8 * 10, 17, 55, 255, 255);
	}
	if (fire)
	{
		wcscpy(smp[editinstr].name, filelist[keysel]);
		yofs = 0;
		wcscpy(tmp, selfdir);
		wcscat(tmp, L"\\");
		wcscat(tmp, filelist[keysel]);
//__try{
		loadwavfile(tmp, editinstr);	//}  __finally {  }//memset(&smp[editinstr],0,sizeof(smp[0]));   }
		mode = 1;
	}
	fire = false;
}
//-----------------------------------------------------------------------------
//----------   MIDI EXPORT -------------------------------------------------------------------------------------------------
struct mthd_chunk {
	char id[4];
	unsigned char Length[4];
	unsigned char Format[2];
	unsigned char NumTrack[2];
	unsigned char Division[2];
};

struct MTRK_CHUNK {
	char ID[4];	/* This will be 'M','T','r','k' */
	unsigned char Length[4];
};
//----------------------
char *mymidi;
long midlen;
long midpos;
int midnoteoffset = 3;
//-----------------------------------
void mbf(char cmd)
{
	mymidi[midlen++] = cmd;
}
//------------------------------------
void VarLen(unsigned long value)
{
	unsigned long buffer;
	buffer = value & 0x7F;
	while ((value >>= 7))
	{
		buffer <<= 8;
		buffer |= ((value & 0x7F) | 0x80);
	}
	while (1)
	{
		mbf((char)buffer);
		if (buffer & 0x80)
			buffer >>= 8;
		else
			break;
	}
}
//--------------------------------------------------------------
char gm(void)
{
	return mymidi[midpos++];
}
//-------------------------------------------------------------
unsigned long rl(void)
{
	unsigned long l = 0;
	unsigned char i = 0, c;
	while (1)
	{
		c = gm();
		if (!(c & 0x80))
		{
			l |= c;
			break;
		}
		c &= 0x7f;
		l |= c;
		l <<= 7;
	}
	return l;
}
//-----------------------------------------------------------------
//---------------------------
void savemidparam(char u, char z, char s)
{
	long i = playseq * 16 + notenum, l = 0;
	if (i != midpos)
	{
		l = i - midpos;
		midpos = i;
	}
	l *= 2750;
	l = l / bpm;
	char c = tmpins[z].p[u];
	if (u == 0)
	{
		if (s == 3)
		{
			z = 10;
			s = 44;
		}
		VarLen(l);
		mbf(0xc0 | z);
		mbf(s);
	}	//instrument
	if (u == 1)
		if (s > 1)
		{
			if (c != s)
			{
				if (c > 0)
				{
					VarLen(l);
					mbf(0x80 | z);
					mbf(c + midnoteoffset);
					mbf(64);
				}	//note
				VarLen(l);
				mbf(0x90 | z);
				mbf(s + midnoteoffset);
				mbf(0x7f);
			}
		}
		else
		{
			VarLen(l);
			mbf(0x80 | z);
			mbf(c + midnoteoffset);
			mbf(64);
		}
	if (s < 1)
		return;
	if (u == 4)
	{
		VarLen(l);
		mbf(0xb0 | z);
		mbf(7);
		mbf(s);
	}
	if (u == 14)
	{
		VarLen(l);
		mbf(0xb0 | z);
		mbf(12);
		mbf(s);
	}
	if (u == 13)
	{
		VarLen(l);
		mbf(0xb0 | z);
		mbf(13);
		mbf(s);
	}

}
//-------------
void exportmid(void)
{
	play = false;
	waveOutReset(hWaveOut);
	midpos = 0;
	midlen = 0;
	struct mthd_chunk mthd;
	struct MTRK_CHUNK mtrk;
	memset(tmpins, 0, sizeof(tmpins));
	mymidi = malloc(73333);
	mthd.id[0] = 0x4d;
	mthd.id[1] = 0x54;
	mthd.id[2] = 0x68;
	mthd.id[3] = 0x64;
	mthd.Length[0] = 0x00;
	mthd.Length[1] = 0x00;
	mthd.Length[2] = 0x00;
	mthd.Length[3] = 0x06;
	mthd.Format[0] = 0x00;
	mthd.Format[1] = 0x00;
	mthd.NumTrack[0] = 0x00;
	mthd.NumTrack[1] = 0x01;
	mthd.Division[0] = 0x00;
	mthd.Division[1] = 0x60;
	mtrk.ID[0] = 0x4d;
	mtrk.ID[1] = 0x54;
	mtrk.ID[2] = 0x72;
	mtrk.ID[3] = 0x6b;
	playseq = 0;
	notenum = 0;
	mbf(0);
	mbf(0xff);
	mbf(0x51);
	mbf(3);
	mbf(5);
	mbf(0x9d);
	mbf(0x80);

	int i, x, y, u, z, s, l;
	for (playseq = 0; playseq <= maxseq; playseq++)
		for (notenum = 0; notenum <= 15; notenum++)
			for (z = 0; z <= maxinstr; z++)
			{
				for (u = 0; u < 16; u++)
					if ((char)instr[z].p[u] < 0)
					{
						s = seq[playseq][abs(instr[z].p[u] + 1)] - 1;

						if (s < 0)
							x = 0;
						else
							x = patterns[s].p[notenum];
						savemidparam(u, z, x);
						tmpins[z].p[u] = x;
					}
					else
					{
						if (tmpins[z].p[u] != instr[z].p[u])
							savemidparam(u, z, instr[z].p[u]);
						tmpins[z].p[u] = instr[z].p[u];
					}
			}
	wchar_t ttt[300];
	mbf(0);
	mbf(0xff);
	mbf(0x2f);
	mbf(0);
	l = midlen;
	for (i = 0; i < 4; i++)
	{
		mtrk.Length[3 - i] = l & 0xff;
		l >>= 8;
	}
	HANDLE hFile;
	DWORD dwNumberOfBytesRead;
	ULONG size = 0;
	wcscpy(ttt, editfile);
	wcscat(ttt, L".mid");
	hFile = CreateFile(ttt, GENERIC_READ + GENERIC_WRITE, FILE_SHARE_READ + FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		WriteFile(hFile, &mthd, sizeof(mthd), &dwNumberOfBytesRead, NULL);
		WriteFile(hFile, &mtrk, sizeof(mtrk), &dwNumberOfBytesRead, NULL);
		WriteFile(hFile, mymidi, midlen, &dwNumberOfBytesRead, NULL);
	}
	CloseHandle(hFile);
	free(mymidi);
}

//---------------------------------------------------------
void loadmid(wchar_t *filename)
{
	play = false;
	waveOutReset(hWaveOut);
	struct mthd_chunk mthd;
	struct MTRK_CHUNK mtrk;
	unsigned char chn, old;
	wchar_t txt[100];
	erasemusic();
	unsigned long np, x, trk, i, l, z;
	playseq = 0;
	maxseq = 0;
	notenum = 0;
	maxinstr = 0;
	maxpatnames = 0;
	maxtrack = 0;
	HANDLE hFile;
	char c;
	DWORD dwNumberOfBytesRead;
	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ReadFile(hFile, &mthd, sizeof(mthd), &dwNumberOfBytesRead, NULL);
	for (trk = 0; trk < mthd.NumTrack[1]; trk++)
	{
		ReadFile(hFile, &mtrk, sizeof(mtrk), &dwNumberOfBytesRead, NULL);
		l = 0;
		for (i = 0; i < 4; i++)
		{
			l = l << 8;
			l = l | mtrk.Length[i];
		}
		mymidi = malloc(l * 5);
		midpos = 0;
		ReadFile(hFile, mymidi, l, &dwNumberOfBytesRead, NULL);
		playseq = 0;
		notenum = 0;
		while (l--)
		{
			z = rl();
			//if(z>0)if(++notenum>15){playseq++;notenum=0;}
			c = gm();
			char r = c & 0xf0;
			if ((char)0x90 == r)
			{
				chn = c & 0x0f;
				if (z > 0)
					if (++notenum > 15)
					{
						playseq++;
						notenum = 0;
					}
				if (chn > maxtrack)
					maxtrack = chn + 1;
				np = playseq + maxpatnames;
				memset(patterns[np].nme, 0, 5);
				patterns[np].nme[1] = L'0' + (maxpatnames & 0xf);
				patterns[np].nme[0] = L'0' + (chn & 0xf);
				patterns[np].nme[2] = L'0' + (playseq & 0xf);
				seq[playseq][chn] = np + 1;
				patterns[np].p[notenum] = gm();
				gm();
			}	//note
			if ((char)0xb0 == r)
			{
				gm();
				gm();
			}	//controller
			if ((char)0x80 == r)
			{
				gm();
				gm();
			}
			if ((char)0xa0 == r)
			{
				gm();
				gm();
			}
			if ((char)0xe0 == r)
			{
				gm();
				gm();
			}
			if ((char)0xc0 == r)
			{
				gm();
			}	//instr
			if ((char)0xd0 == r)
			{
				gm();
			}
			if (c == (char)0xf0)
			{
				x = rl();
				for (; x--;)
					gm();
			}
			if (c == (char)0xf7)
			{
				x = rl();
				for (; x--;)
					gm();
			}
			if (c == (char)0xff)
			{
				char t = gm();
				if (t == (char)0x2f)
				{
					gm();
					break;
				}
				x = rl();
				for (; x--;)
					gm();
			}
		}
		if (maxseq < playseq)
			maxseq = playseq + 1;
		maxpatnames += playseq;
		if (maxpatnames > 100)
			maxpatnames = 100;
		free(mymidi);
	}
	maxpatnames++;
	CloseHandle(hFile);
}
//-----------------------------------------
//-------------------------------------------------------------
void drawmidtxtlist(void)
{
	cls(194);
	_Bool t;
	wchar_t tmp[255];
	int i, x, y, m;
	if ((keysel += -gy) < (gy = 0))
	{
		keysel = 0;
		if (yofs > 0)
			yofs--;
		findmid(yofs);
	}
	if (keysel >= maxfilelist)
	{
		keysel = 0;
		yofs += maxfilelist;
		findmid(yofs);
	}
	m = keysel + yofs;
	for (i = 0; i <= maxfilelist; i++)
	{
		print(filelist[i], 1, i * 16);
		if (i == keysel)
			drawinvertbox(0, sy - i * 16 - 16, 8 * 10, 17, 55, 255, 255);
	}
	if (fire)
	{
		yofs = 0;
		wcscpy(tmp, selfdir);
		wcscat(tmp, L"\\");
		wcscat(tmp, filelist[keysel]);
		loadmid(tmp);
		mode = 5;
	}
	fire = false;
}


//---------------------------------------------------------------------------------------------
void exporth(void)
{
	wchar_t ttt[300];
	char ccc[10], sss[10000];

	HANDLE hFile;
	DWORD dwNumberOfBytesRead;
	wavhdr.freq = frequency;
	wavhdr.Bytespersecond = (frequency * stereo * (bits / 8));
	play = false;
	waveOutReset(hWaveOut);
	playseq = 0;
	index = 0;
	memset(&string, 0, sizeof(string));
	notenum = 0;
	ULONG size = 0;
	wcscpy(ttt, editfile);
	wcscat(ttt, L".h");
	hFile = CreateFile(ttt, GENERIC_READ + GENERIC_WRITE, FILE_SHARE_READ + FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		sprintf(sss, "int bpm=%i;\r\n int frequency=%i;\r\n   #define  maxseq %i\r\n #define maxinstr %i", abs(bpm), frequency, maxseq, maxinstr);
		WriteFile(hFile, &sss, strlen(sss), &dwNumberOfBytesRead, NULL);
		strcpy(sss, "char patterns[][]={{");
		for (int t = 0; t < maxpatnames; t++)
		{
			for (int i = 0; i <= 15; i++)
			{
				sprintf(ccc, "%i,", patterns[t].p[i]);
				strcat(sss, ccc);
			} sss[strlen(sss) - 1] = 0;
			strcat(sss, "},{");
		}
		sss[strlen(sss) - 2] = 0;
		strcat(sss, "};\r\n");
		WriteFile(hFile, &sss, strlen(sss), &dwNumberOfBytesRead, NULL);
		strcpy(sss, "char seq[][]={{");
		for (int t = 0; t <= maxseq; t++)
		{
			for (int i = 0; i <= maxtrack; i++)
			{
				sprintf(ccc, "%i,", seq[t][i]);
				strcat(sss, ccc);
			} sss[strlen(sss) - 1] = 0;
			strcat(sss, "},{");
		}
		sss[strlen(sss) - 2] = 0;
		strcat(sss, "};\r\n");
		WriteFile(hFile, &sss, strlen(sss), &dwNumberOfBytesRead, NULL);
		strcpy(sss, "char instr[][]={{");
		for (int t = 0; t < maxinstr; t++)
		{
			for (int i = 0; i <= 15; i++)
			{
				sprintf(ccc, "%i,", instr[t].p[i]);
				strcat(sss, ccc);
			} sss[strlen(sss) - 1] = 0;
			strcat(sss, "},{");
		}
		sss[strlen(sss) - 2] = 0;
		strcat(sss, "};\r\n");
		WriteFile(hFile, &sss, strlen(sss), &dwNumberOfBytesRead, NULL);
	}
	CloseHandle(hFile);
}

//----------------------------------------------------------------------
void exportwav(void)
{
	wchar_t ttt[300];

	HANDLE hFile;
	DWORD dwNumberOfBytesRead;
	wavhdr.freq = frequency;
	wavhdr.Bytespersecond = (frequency * stereo * (bits / 8));
	play = false;
	waveOutReset(hWaveOut);
	playseq = 0;
	index = 0;
	memset(&string, 0, sizeof(string));
	notenum = 0;
	ULONG size = 0;
	wcscpy(ttt, editfile);
	wcscat(ttt, L".wav");
	hFile = CreateFile(ttt, GENERIC_READ + GENERIC_WRITE, FILE_SHARE_READ + FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		WriteFile(hFile, &wavhdr, 44, &dwNumberOfBytesRead, NULL);
		for (int i = 1256; i--;)
		{
			memset(b1, 0, BufferSize);
			fillshort((short *)b1);
			WriteFile(hFile, b1, BufferSize, &dwNumberOfBytesRead, NULL);
			size += dwNumberOfBytesRead;
			if (playseq == 0 && notenum == 0)
				break;
			if (dwNumberOfBytesRead < BufferSize)
				print(L"Error: 0 bytes write", 10, 10);
#ifdef _WINCE
			drawscope(sx - 1, 160, 900);
#endif
			BitBlt(GetDC(0), 0, 0, sx, sy, hdcMem, 0, 0, SRCCOPY);
		}
		wavhdr.Sizeofdata = size;
		wavhdr.size36 = size + 36;
		SetFilePointer(hFile, 0, 0, FILE_BEGIN);
		WriteFile(hFile, &wavhdr, 44, &dwNumberOfBytesRead, NULL);
	}
	CloseHandle(hFile);
}

//--------------------------------------------------------------------
wchar_t saveas[50];
void drawfilesaveas(MSG m)
{
	cls(33);
	print(L"input file name \r\n & press enter", 1, 19);
	if (m.message == WM_KEYUP)
	{	//edit name
		if (m.wParam != VK_RETURN && m.wParam != VK_UP && m.wParam != VK_DOWN)
		{
			saveas[editchar++] = (char)m.wParam;
			editchar &= 7;
		}
	}
	print(saveas, 5, sy / 3);
	if (fire)
	{
		wcscpy(editfile, selfdir);
		wcscat(editfile, L"\\");
		wcscat(editfile, saveas);
		wcscat(editfile, L".zm4");
		mode = 6;
		savefile(editfile);
	}
	fire = false;
}	//--------------------------------------------------------------
void drawfiles(MSG m)
{
	cls(215);
	wchar_t txt[255];
	_Bool t;
	int f = filemcnt, l, x, y, i, ysize = (sy - 10) / (f + 1);
	y = ysize / 2;

	if (move)
		keysel = ty / (ysize);

	if ((keysel += -gy) < (gy = 0))
		keysel = filemcnt - 1;
	keysel *= (keysel < filemcnt);
	i = keysel;
	if (keysel == (filemcnt - 1))
	{
		bpm += gx;
		if (gx != 0)
		{
			t = play;
			newbpm();
			restartbuf();
			play = t;
		}
	}
	if (keysel == (filemcnt - 2))
		if (gx != 0)
		{
			if (frequency == 22000)
				frequency = 44000;
			else
				frequency = 22000;
			play = false;
			closewave();
			initwaveout(hwnd);
			initwavbuf();
			restartbuf();
		}
	if (keysel == (filemcnt - 3))
	{
		patviewmode += gx;
		patviewmode &= 1;
	}

	gx = 0;
	for (i = 0; i < filemcnt; i++)
	{
		print(filemenu[i].mnu, 1, i * ysize);
		if (i == (filemcnt - 1))
		{
			wsprintf(txt, L"<%u>", bpm);
			print(txt, 8 * 10, i * ysize);
		}
		if (i == (filemcnt - 2))
		{
			wsprintf(txt, L"<%u>", frequency);
			print(txt, 8 * 10, i * ysize);
		}
		if (i == (filemcnt - 3))
		{
			wsprintf(txt, L"<%u>", patviewmode);
			print(txt, 8 * 10, i * ysize);
		}

		if (i == keysel)
			drawinvertbox(0, sy - i * ysize - 16, 8 * 10, 17, 55, 255, 255);
	}
	if (fire)
	{
		if (keysel == 5)
		{
			memset(editfile, 0, sizeof(editfile));
			erasemusic();
			mode = 6;
		}
		if (keysel == 0)
		{
			closewave();
			ExitProcess(0);
		}
		if (keysel == 1)
			findzm4(0);
		if (keysel == 6)
			mergezm4(0);
		if (keysel == 7)
			exporth();
		if (keysel == 8)
			mode = 18;
		if (keysel == 9)
			exportmid();
		if (keysel == 10)
			findmid(0);
		if (keysel == 2)
			if (wcslen(editfile) == 0)
				mode = 15;
			else
			{
				savefile(editfile);
				mode = 6;
			}
		if (keysel == 3)
			mode = 15;
		if (keysel == 4)
			exportwav();
	}
	fire = FALSE;
}	//-----------------------------------показывает меню---------------------------------------
void showmnu(int z)
{
	int t = mnurows[z];
	if (move)
		menuitem = (sy - ty + 15) / 15;
	drawboxmenu(6 * 9, t);
	if (menuitem >= t)
		menuitem = 1;
	if (menuitem < 1)
		menuitem = t - 1;
	for (int i = t; i--;)
	{
		print(menu[i].mnu, sx - 5 * 8, sy - i * 15);
		if (menuitem == i)
			print(L"-", sx - 6 * 8, sy - i * 15);
	}
	if (fire)
	{
		mode = menuitem;
		showmenu = !showmenu;
		drawboxmenu(5 * 9, mnurows[0]);
	}
	fire = FALSE;
}
//--------------------------------//--------------------------------
//------------------------------//----------------------------------
_Bool nomove;
int xt, yt;
void keystate(MSG m)
{
	fire = false;
	move = false;
	int d = 7;
	ty = HIWORD(m.lParam);
	tx = LOWORD(m.lParam);
	if (m.message == WM_LBUTTONUP)
	{
		move = true;
		xt = tx / d;
		yt = ty / d;
	}	//x202
	if (m.message == WM_LBUTTONDOWN)
	{
		move = true;
		if (yt == (ty / d) && xt == (tx / d))
			fire = true;
		else
			xt = tx / d;
		yt = ty / d;
	}	//x201if(yt==(ty/d)&&xt==(tx/d))fire=true; else 
	if (m.message == WM_LBUTTONDBLCLK)
	{
		move = true;
		fire = true;
	}
	if (m.message == WM_MOUSEMOVE)
	{
		move = true;
		nomove = false;
	}	//xt=tx/d;yt=ty/d;}//x200
	if (ty > (sy - 15))
		if (fire)
		{
			fire = false;
			move = false;
			if (tx > (sx - 8 * 8))
				m.wParam = VK_TSOFT2;
			if (tx < (8 * 8))
				m.wParam = VK_TSOFT1;
		}
		else
		{
			fire = false;
			move = false;
		}
#ifdef _WINCE
	if (m.wParam == VK_TSOFT2)
	{
		showmenu = !showmenu;
		drawboxmenu(5 * 9, mnurows[0]);
		editchar = 0;
		taboffset = 0;
	}
	if (m.wParam == VK_TSOFT1)
	{
		play = !play;
		playseq = 0;
		index = 0;
		memset(&string, 0, sizeof(string));
		notenum = 0;
		if (play)
			restartbuf();
		else
			waveOutReset(hWaveOut);
	}
#else
	if (m.wParam == VK_TSOFT2)
	{
		showmenu = true;
		drawboxmenu(5 * 9, mnurows[0]);
		editchar = 0;
		taboffset = 0;
	}
	if (m.wParam == VK_TSOFT1)
	{
		play = true;
		playseq = 0;
		index = 0;
		memset(&string, 0, sizeof(string));
		notenum = 0;
		if (play)
			restartbuf();
		else
			waveOutReset(hWaveOut);
	}
	if (m.wParam == VK_F3)
	{
		play = false;
		playseq = 0;
		index = 0;
		memset(&string, 0, sizeof(string));
		notenum = 0;
		if (play)
			restartbuf();
		else
			waveOutReset(hWaveOut);
	}
#endif

	//  if(m.message==WM_KEYUP)keys[m.wParam]=0;
	if (m.message == WM_KEYDOWN)
	{
		if (m.wParam == VK_RETURN)
			fire = TRUE;
		//if(keys[m.wParam]==0){                    
		if (m.wParam == VK_UP)
			gy++;
		if (m.wParam == VK_DOWN)
			gy--;
		if (m.wParam == VK_RIGHT)
			gx++;
		if (m.wParam == VK_LEFT)
			gx--;
		//}     keys[m.wParam]=0;   
	}
}
#ifdef _WINCE
void WinMainCRTStartup()
#else
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpszCmdLine, int nCmdShow)
#endif
//void WinMainCRTStartup()
{
	HDC hdc;
	RawFrameBufferInfo screen_rfbi;
	hdc = GetDC(0);
	ExtEscape(hdc, GETRAWFRAMEBUFFER, 0, NULL, sizeof(RawFrameBufferInfo), (char *)&screen_rfbi);
	ReleaseDC(NULL, hdc);
	LPWORD fb = (LPWORD)screen_rfbi.pFramePointer;
	wchar_t temp[500];
	GetModuleFileName(NULL, selfdir, 255);
	int i = wcslen(selfdir);
	for (; i > 1; i--)
		if (selfdir[i] == '\\')
			break;
	selfdir[i + 1] = 0;
	fillcell = 2;
	playcell = 1;
	RECT rc;
	wchar_t txt[300], *cl;
	mode = 14;
	menuitem = mode;
#ifdef _WINCE
	sx = GetSystemMetrics(SM_CXSCREEN);
	sy = GetSystemMetrics(SM_CYSCREEN);

//sx=screen_rfbi.cxPixels;
//sy=screen_rfbi.cyPixels;

	int strty = 0;
	if (sy > 190)
		strty = 23;
	hwnd = CreateWindow(L"DIALOG", 0, WS_POPUP | WS_VISIBLE, 0, strty, sx, sy - strty, 0, 0, 0, 0);
	SHFullScreen(hwnd, SHFS_HIDETASKBAR);
	SHFullScreen(hwnd, SHFS_HIDESIPBUTTON);
	SHFullScreen(hwnd, SHFS_HIDESTARTICON);

#else
	sx = 320;
	sy = 400;
	hwnd = CreateWindow(L"BUTTON", 0, WS_POPUP | WS_VISIBLE, 0, 0, sx, sy, 0, 0, 0, 0);
//txt={L"EDIT"};
#endif
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, sx, sy, 0);
	unsigned int a, d = sx * 3;
	MSG m;
	int ty = 0;
	mnurows[0] = sizeof(menu) / sizeof(menu[0]);
	GetClientRect(hwnd, &rc);
	sx = rc.right;
	sy = rc.bottom;
	hdc = GetDC(hwnd);
	createvirt(hdc);
	initsintab();
	initwaveout(hwnd);
	erasemusic();
	cl = GetCommandLine();
#ifdef _WINCE
	if (cl[0] != 0)
	{
		loadfile(cl);
		wcscpy(editfile, cl);
		mode = 6;
		playseq = 0;
		index = 0;
		play = true;
		restartbuf();
	}
	else
		mode = 14;
#endif
	initwavbuf();
	octave = 2;
// MAIN LOOP   MAIN LOOP   MAIN LOOP   MAIN LOOP   MAIN LOOP   MAIN LOOP 
	_Bool yestime = true;
	do
	{
		if (play)
		{
			i = playengine();
			yestime = i < 110;
			if (yestime && mode == 2)
				drawscope(sx - 1, 160, 900);
		}
#ifdef _WINCE
		if (!showmenu)
			print(L"   Menu ", sx - 5 * 8, sy - 15);
		else
			showmnu(0);
		if (!play)
			print(L"play", 1, sy - 15);
		else if (yestime)
			print(L"stop", 1, sy - 15);	//wince
#else
		if (!showmenu)
			print(L"ESC= EXIT F2= Menu ", sx - 25 * 8, sy - 15);
		else
			showmnu(0);
		if (!play)
			print(L" F1= play ", 1, sy - 15);
		else
			print(L"  F3= stop  ", 1, sy - 15);	//pc version;
#endif

		if (yestime && !showmenu && change)
		{
			if (mode == 13)
				drawkeybvert(hdcMem, 20, 7, sy - 17, sx, octave * 12 - 3, m);
			if (mode == 10)
				drawparams();
			if (mode == 4)
				drawinstrs(m);
			if (mode == 5)
				drawsheet(m);
			if (mode == 6)
				drawseq(m);
			if (mode == 3)
				drawfiles(m);
			if (mode == 14)
				drawfilelist(m);
			if (mode == 15)
				drawfilesaveas(m);
			if (mode == 1)
				drawsmp(m);
			if (mode == 16)
				drawwavtxtlist();
			if (mode == 17)
				drawmergefile(m);
			if (mode == 18)
				drawrandom();
			if (mode == 19)
				drawmidtxtlist();
		}
		if (mode == 2 || change)
			BitBlt(hdc, 0, 0, sx, sy, hdcMem, 0, 0, SRCCOPY);
		int mmm = m.message;
		PeekMessage(&m, 0, 0, 0, PM_REMOVE);
		change = mmm != m.message;
		yestime = true;
		int mit = gy;
		keystate(m);
		if (showmenu)
			menuitem += gy - mit;
	} while (m.wParam != VK_ESCAPE && m.message != WM_CLOSE && m.message != WM_QUIT && m.message != WM_KILLFOCUS);
// END OF MAIN LOOP    END OF MAIN LOOP    END OF MAIN LOOP    END OF MAIN LOOP   
	closewave();
	ExitProcess(0);
}
