#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>

#define MIN(x,y) ((x) > (y) ? (y) : (x))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define Log10(x)     log10(double(x))
#define Exp10(x)     exp10(double(x))
#define SNR_DEFAULT_VALUE     5
#define SNR_LOW_DEFAULT_VALUE     5
#define SNR_HIGH_DEFAULT_VALUE    10


struct RIFF_HEADER{
    //RIFF_HEADER 
    unsigned   char     szRiffID[4];           //   'R','I','F','F' 
    unsigned   int     dwRiffSize; 
    unsigned   char     szRiffFormat[4];     //   'W','A','V','E'  
};

struct FMT_BLOCK{
    //   FMT_BLOCK 
    unsigned   char     szFmtID[4];           //   'f','m','t','   ' 
    unsigned   int     dwFmtSize;  
    // dwFmtSize为16则其后16字节，否则为18，末尾有2个字节的附加信息
    unsigned   short   wFormatTag; 
    unsigned   short   wChannels; 
    unsigned   int     dwSamplesPerSec; 
    unsigned   int     dwAvgBytesPerSec; 
    unsigned   short   wBlockAlign; 
    unsigned   short   wBitsPerSample;     
//  unsigned   short   ultraMessage;   
};

struct FACT_BLOCK{
    unsigned   char     szFactID[4];   //   'f','a','c','t' 
    unsigned   int     dwFactSize; 
};

struct DATA_BLOCK{
    //   DATA_BLOCK 
    unsigned   char   szDataID[4];   //   'd','a','t','a' 
    unsigned   int   dwDataSize;    
};

/*
  return :
       dataBuff : data buff 
       len : buff length
*/
int readData(const char * file, char** dataBuff, int &len, int &samples);
int writeData2Wave(const char * file, char * dataBuff, int len, int samples);
int addRandomNoiseBaseSnr(char *noisefile, char *source, char *destination, int snr);
