/*
 * 
 * TODO - readme : 
 * 
 * https://github.com/jeelabs/esp-link/blob/master/README.md
 * https://github.com/Spritetm/libesphttpd
 * api & tech reference
 * 
 * */


#include <esp8266.h>
#include "cgiwifi.h"
#include "cgi.h"
#include "config.h"
#include "sntp.h"
#include "cgimqtt.h"
#ifdef SYSLOG
#include "syslog.h"
#endif
#define DBG(format, ...) do { os_printf(format, ## __VA_ARGS__); } while(0)

int ICACHE_FLASH_ATTR cgiMetaWav(HttpdConnData *connData) {
/*
from https://codereview.stackexchange.com/questions/105272/writing-computer-generated-music-to-a-wav-file-in-c
* 
 bool WriteWavePCM(short* sound, size_t pairAmount, char* fileName){

 * */
/******************************
*  Magic file format strings. *
******************************/
const char fChunkID[]     = {'R', 'I', 'F', 'F'};
const char fFormat[]      = {'W', 'A', 'V', 'E'};
const char fSubchunk1ID[] = {'f', 'm', 't', ' '};
const char fSubchunk2ID[] = {'d', 'a', 't', 'a'};

/********************************
* WriteWavePCM() configuration: *
* - 2 channels,                 *
* - frequency 44100 Hz.         *
********************************/
const unsigned short N_CHANNELS = 1;
const unsigned int SAMPLE_RATE = 44100;
const unsigned short BITS_PER_BYTE = 8;
const unsigned int N_SAMPLE_PAIRS = 1024; /*1048576;*/

    const static unsigned int fSubchunk1Size = 16;
    const static unsigned short fAudioFormat = 1;
    const static unsigned short fBitsPerSample = 16;

    unsigned int fByteRate = SAMPLE_RATE * N_CHANNELS *
                             fBitsPerSample / BITS_PER_BYTE;

    unsigned short fBlockAlign = N_CHANNELS * fBitsPerSample / BITS_PER_BYTE;
    unsigned int fSubchunk2Size;
    unsigned int fChunkSize;

    char header[46+N_SAMPLE_PAIRS * N_CHANNELS*2*sizeof(short)];
	memset(header,0,46+N_SAMPLE_PAIRS * N_CHANNELS*2*sizeof(short));
	short *sound;
    int i;
    int j;

    DBG("META : generating");
	sound=(short*)(header+46);
    for (i = 0, j = 0; i < N_SAMPLE_PAIRS * N_CHANNELS; i += 2, j++ )
    {
        short datum1 = 450 * ((j >> 9 | j >> 7 | j >> 2) % 128);
        short datum2 = 450 * ((j >> 11 | j >> 8 | j >> 3) % 128);

        sound[i]     = datum1; // One channel.
        sound[i + 1] = datum2; // Another channel.
    }

    fSubchunk2Size =  N_SAMPLE_PAIRS * N_CHANNELS *2*sizeof(short);
    fChunkSize = 46 + fSubchunk2Size;
    DBG("META : generating header 1\n");
	memcpy(header,&fChunkID,4);
	memcpy(header+4,&fChunkSize,4);
	memcpy(header+8,&fFormat,4);
    DBG("META : generating header 2\n");

	memcpy(header+12,&fSubchunk1ID,4);
	memcpy(header+16,&fSubchunk1Size,4);
	memcpy(header+20,&fAudioFormat,2);
	memcpy(header+22,&N_CHANNELS,2);
	memcpy(header+24,&SAMPLE_RATE,4);
	memcpy(header+28,&fByteRate,4);
	memcpy(header+32,&fBlockAlign,2);
	memcpy(header+34,&fBitsPerSample,2);

    DBG("META : generating header 3\n");
	memcpy(header+36,&fSubchunk2ID,4);
	memcpy(header+40,&fSubchunk2Size,4);
	DBG("META : sending header\n");
   noCacheHeaders(connData, 200);
  httpdHeader(connData, "Content-Type", "audio/x-wav");
//  httpdHeader(connData, "Content-Length", fChunkSize);
  httpdEndHeaders(connData);
DBG("META : sending data\n");
   
    httpdSend(connData, header, fChunkSize);
    httpdSend(connData, header+44, -1);
    DBG("META : done");

/*
    // Writing the RIFF header:
    fwrite(&fChunkID, 1, sizeof(fChunkID),      fout);
    fwrite(&fChunkSize,  sizeof(fChunkSize), 1, fout);
    fwrite(&fFormat, 1,  sizeof(fFormat),       fout);

    // "fmt" chunk:
    fwrite(&fSubchunk1ID, 1, sizeof(fSubchunk1ID),      fout);
    fwrite(&fSubchunk1Size,  sizeof(fSubchunk1Size), 1, fout);
    fwrite(&fAudioFormat,    sizeof(fAudioFormat),   1, fout);
    fwrite(&N_CHANNELS,      sizeof(N_CHANNELS),     1, fout);
    fwrite(&SAMPLE_RATE,     sizeof(SAMPLE_RATE),    1, fout);
    fwrite(&fByteRate,       sizeof(fByteRate),      1, fout);
    fwrite(&fBlockAlign,     sizeof(fBlockAlign),    1, fout);
    fwrite(&fBitsPerSample,  sizeof(fBitsPerSample), 1, fout);

    // "data" chunk: 
    fwrite(&fSubchunk2ID, 1, sizeof(fSubchunk2ID),      fout);
    fwrite(&fSubchunk2Size,  sizeof(fSubchunk2Size), 1, fout);

    // sound data: 
    fwrite(sound, sizeof(short),  N_SAMPLE_PAIRS * N_CHANNELS, fout);
*/
/*
  else {
   noCacheHeaders(connData, 200);
    httpdEndHeaders(connData);
  httpdHeader(connData, "Content-Type", "application/json");
    httpdSend(connData, "Failed to get sound", -1);
  }*/
  return HTTPD_CGI_DONE;
//    return true;
}

