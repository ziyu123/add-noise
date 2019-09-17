#include "readWave.h"

int readData(const char * file, char** dataBuff, int& len, int &samples){
    if(NULL == file || NULL == dataBuff){
	   fprintf(stderr, "readData point is NULL.\n");
	   return -1;
	}
	FILE * fp = fopen(file, "rb");
	if(NULL == fp){
	    printf("audio file %s open fail.\n", file);
		return -1;
	}
	RIFF_HEADER riff_h;
	fread(&riff_h, sizeof(riff_h), 1, fp);
	if(0 != strncmp((const char*)riff_h.szRiffID, "RIFF", 4)){
	    printf("bad audio head, expect RIFF, but get %s\n",riff_h.szRiffID);
		return -1;
	}
	if(0 != strncmp((const char*)riff_h.szRiffFormat, "WAVE", 4)){
	    printf("bad audio format, expect 'WAVE', but get %s\n",riff_h.szRiffFormat);
		return -1;
	}

    FMT_BLOCK fmt_h;
	fread(&fmt_h, sizeof(fmt_h), 1, fp);
	if(0 != strncmp((const char*)fmt_h.szFmtID, "fmt ", 4)){
	    printf("bad audio format, expect 'fmt ',bug get %s\n",fmt_h.szFmtID);
		return -1;
	}
    if(fmt_h.wChannels > 2){
	    printf("bad audio channels, support 1 or 2 channel only.\n");
		return -1;
	}
	if(18 == fmt_h.dwFmtSize){
        unsigned short ultraMessage;
        fread(&ultraMessage, 1, 2, fp);
	}

	int samplesPerSec = fmt_h.dwSamplesPerSec;
    int bytesPerSec = fmt_h.dwAvgBytesPerSec; 
    int blockAlign = fmt_h.wBlockAlign;
    int bitsPerSample = fmt_h.wBitsPerSample;

	if(8000 != samplesPerSec && 16000 != samplesPerSec){
	    printf("bad audio format, only support 8k or 16k sample per sec. but get %d\n", samplesPerSec);
		return -1;
	}
	if(16 != bitsPerSample){
	    printf("bad audio format, only support 16 bits per sample, but get %d\n", bitsPerSample);
		return -1;
	}
    if(bytesPerSec != (fmt_h.wChannels * samplesPerSec * bitsPerSample / 8)){
	    fprintf(stderr, "bad audio format\n");
		return -1;
	}
	samples = samplesPerSec;

    DATA_BLOCK data_h;
    fread(&data_h, 1, sizeof(data_h), fp);
    int dataLen = 0;
	char tmpbuff[10240]  = {0};
	while(0 != strncmp((const char*)data_h.szDataID, "data", 4)){
	    fread(tmpbuff, 1, data_h.dwDataSize, fp);
		fread(&data_h, 1, sizeof(data_h), fp);
	}
	if(0 == strncmp((const char*)data_h.szDataID, "data", 4)){
        dataLen = data_h.dwDataSize;  	    
	}else if(0 == strncmp((const char*)data_h.szDataID, "fact", 4)){
	    fread(&data_h, 1, sizeof(data_h), fp);
		dataLen = data_h.dwDataSize;
	}
	char * databuff = (char*)malloc(sizeof(char) * dataLen);
	fread(databuff, 1, dataLen, fp);
	char * singleData = NULL;
    if(2 == fmt_h.wChannels){
	    singleData = (char*)malloc(sizeof(char) * dataLen / 2 );
		char * pData = singleData;
		for(int i = 0; i < dataLen; i += blockAlign){
		    memcpy(pData,  databuff + i, blockAlign/fmt_h.wChannels);
			pData += blockAlign/fmt_h.wChannels;
		}
	    *dataBuff = singleData;
		if(NULL != databuff){
		    free(databuff);
			databuff = NULL;
		}
		len = dataLen / 2;
	}else{
	    *dataBuff = databuff;
	    len = dataLen;
	}
	if(NULL != fp){
	    fclose(fp);
	}
    return 0;
}

int writeData2Wave(const char * file, char * dataBuff, int len, int samples){
    if(NULL == file || NULL == dataBuff){
	    fprintf(stderr, "poist is NULL.\n");
		return -1;
	}
    FILE *fp = fopen(file, "w");
	if(NULL == fp){
	    fprintf(stderr, "open %s fail.\n", file);
		return -1;
	}
	RIFF_HEADER riff_h;
	strncpy((char*)riff_h.szRiffID, "RIFF", 4);
    strncpy((char*)riff_h.szRiffFormat, "WAVE", 4);
    riff_h.dwRiffSize = 32 + len ;
    fwrite(&riff_h, 1, sizeof(riff_h), fp);

	FMT_BLOCK fmt_h;
	strncpy((char*)fmt_h.szFmtID, "fmt ", 4);
    fmt_h.dwFmtSize = 16;
    fmt_h.wFormatTag = 1;
    fmt_h.wChannels = 1;
	if(samples != 8000 && samples != 16000){
	    fprintf(stderr, "write fail, support 8k or 16k only.\n");
		return -1;
	}
	fmt_h.dwSamplesPerSec = samples;
	fmt_h.wBitsPerSample = 16;
    fmt_h.dwAvgBytesPerSec = fmt_h.dwSamplesPerSec * (fmt_h.wBitsPerSample / 8);
    fmt_h.wBlockAlign = 2;
	fwrite(&fmt_h, 1, sizeof(fmt_h), fp);

    DATA_BLOCK data_h;
    strncpy((char*)data_h.szDataID, "data", 4);
	data_h.dwDataSize = len;
	fwrite(&data_h, 1, sizeof(data_h), fp);
	fwrite(dataBuff, 1, len, fp);

	if(NULL != fp){
	    fclose(fp);
	}
	return 0;
}

float getAddRatio(char* signal, char* noise, int numbers, int snr){
    short *psignal = (short*)signal;
    short *pnoise = (short*)noise;
    float signalPow = 0.0f, noisePow = 0.0f;
    for(int i = 0; i < numbers; i++){
        int data = *(psignal + i);
        int noisedata = *(pnoise + i);
        signalPow += (data * data);
	noisePow += (noisedata * noisedata);
    }
    signalPow /= numbers;
    noisePow /= numbers; 
    float ratio = sqrt(Exp10(-0.1 * snr) * (signalPow / noisePow));
    return ratio;  
}

int addRandomNoiseBaseSnr(char *noisefile, char *source, char *destination, int snr){
    if(NULL == noisefile || NULL == source || NULL == destination){
        fprintf(stderr, "point is NULL.\n");
        return -1;
    }
    char *noiseData = NULL, *sourceData = NULL;
    int noiseLen = 0, sourceLen = 0;
    int samplesNoise = 0, samplesSignal = 0;
    int ret = readData(noisefile, &noiseData, noiseLen, samplesNoise);
    if(-1 == ret){
        fprintf(stderr, "read data for %s fail.\n", noisefile);
        return -1;
    }
    ret = readData(source, &sourceData, sourceLen, samplesSignal);
    if(-1 == ret){
        fprintf(stderr, "read data for %s fail.\n", source);
	return -1;
    }
    if(samplesNoise != samplesSignal){
        fprintf(stderr, "noise and signal audio sample are not same.\n");
	return -1;
    }
    if(0 == noiseLen || 0 == sourceLen){
        fprintf(stderr, "noise or signal audio file is not data.\n");
        return -1;
    }
    short * psource = (short*)sourceData;
    short * pnoise = (short*)noiseData;
    int min = MIN(noiseLen, sourceLen) / sizeof(short); 
    int overlap = random() % min;
    int startpos = random() % 3200; // start from < 0.1s, for 16k sample
    psource += startpos;
    overlap = min;
    if(startpos + overlap > min){
        overlap = min - startpos - 1;
    }
    printf("add noise start pos: %d, point numbers: %d \n", startpos, overlap);
     if(snr == SNR_DEFAULT_VALUE){
		 for(int i = 0; i < overlap; i++){
			 if(NULL == psource || NULL == pnoise)
			     break;
		     *psource += *pnoise;
			 psource++;
			 pnoise++;
		 }
	 }else{
	     float ratio = getAddRatio((char*)psource, (char*)pnoise, overlap, snr);
		 for(int i = 0; i < overlap; i++){
			 if(NULL == psource || NULL == pnoise)
			     break;
		     *psource += (short(ratio * (*pnoise)));
			 psource++;
			 pnoise++;
		 }
	 }
     ret = writeData2Wave(destination, sourceData, sourceLen, samplesSignal);
	 if(-1 == ret){
	     fprintf(stderr, "add noise fail for %s. \n", source);
		 return -1;
	 }
	 if(NULL != sourceData){
	     free(sourceData);
	 }
	 if(NULL != noiseData){
	     free(noiseData);
	 }
	 return 0; 
}

int mvWaveHead(const char * wavfile, const char* pcmfile){
    if(NULL == wavfile || NULL == pcmfile){
        fprintf(stderr, "file  point is NULL.\n");
	return -1;
    }
    FILE * fp = fopen(wavfile, "rb");
    if(NULL == fp){
        printf("audio file %s open fail.\n", wavfile);
	return -1;
    }
    RIFF_HEADER riff_h;
    fread(&riff_h, sizeof(riff_h), 1, fp);
    if(0 != strncmp((const char*)riff_h.szRiffID, "RIFF", 4)){
        fprintf(stderr, "bad audio head, expect RIFF, but get %s\n",riff_h.szRiffID);
	return -1;
    }
    if(0 != strncmp((const char*)riff_h.szRiffFormat, "WAVE", 4)){
        fprintf(stderr, "bad audio format, expect 'WAVE', but get %s\n",riff_h.szRiffFormat);
	return -1;
    }

    FMT_BLOCK fmt_h;
    fread(&fmt_h, sizeof(fmt_h), 1, fp);
    if(0 != strncmp((const char*)fmt_h.szFmtID, "fmt ", 4)){
        fprintf(stderr, "bad audio format, expect 'fmt ',bug get %s\n",fmt_h.szFmtID);
	return -1;
    }
    if(18 == fmt_h.dwFmtSize){
        unsigned short ultraMessage;
        fread(&ultraMessage, 1, 2, fp);
    }

    int samplesPerSec = fmt_h.dwSamplesPerSec;
    int bytesPerSec = fmt_h.dwAvgBytesPerSec; 
    int blockAlign = fmt_h.wBlockAlign;
    int bitsPerSample = fmt_h.wBitsPerSample;

    if(bytesPerSec != (fmt_h.wChannels * samplesPerSec * bitsPerSample / 8)){
        fprintf(stderr, "bad audio format\n");
	return -1;
    }
    DATA_BLOCK data_h;
    fread(&data_h, 1, sizeof(data_h), fp);
    int dataLen = 0;
    char tmpbuff[10240]  = {0};
    while(0 != strncmp((const char*)data_h.szDataID, "data", 4)){
        fread(tmpbuff, 1, data_h.dwDataSize, fp);
	fread(&data_h, 1, sizeof(data_h), fp);
    }
    if(0 == strncmp((const char*)data_h.szDataID, "data", 4)){
        dataLen = data_h.dwDataSize;  	    
    }else if(0 == strncmp((const char*)data_h.szDataID, "fact", 4)){
	fread(&data_h, 1, sizeof(data_h), fp);
	dataLen = data_h.dwDataSize;
    }
    char * databuff = (char*)malloc(sizeof(char) * dataLen);
    fread(databuff, 1, dataLen, fp);
    
  
    FILE *pcm_fp = fopen(pcmfile, "w");
    if(NULL == pcm_fp){
        fprintf(stderr, "open file %s fail.\n", pcmfile);
        return -1;
    }
    fwrite(databuff, 1, dataLen, pcm_fp);
    if(NULL != fp)  fclose(fp);
    if(NULL != databuff) free(databuff);
    if(NULL != pcm_fp) fclose(pcm_fp);
    return 0;
}


int rmWaveHead44Bytes(const char * wavfile, const char* pcmfile){
    if(NULL == wavfile || NULL == pcmfile){
        fprintf(stderr, "file  point is NULL.\n");
	return -1;
    }
    FILE * fp = fopen(wavfile, "rb");
    if(NULL == fp){
        printf("audio file %s open fail.\n", wavfile);
	return -1;
    }
    fseek(fp, 0, SEEK_END);
    int fileLen = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    RIFF_HEADER riff_h;
    fread(&riff_h, sizeof(riff_h), 1, fp);
    if(0 != strncmp((const char*)riff_h.szRiffID, "RIFF", 4)){
        fprintf(stderr, "bad audio head, expect RIFF, but get %s\n",riff_h.szRiffID);
	return -1;
    }
    if(0 != strncmp((const char*)riff_h.szRiffFormat, "WAVE", 4)){
        fprintf(stderr, "bad audio format, expect 'WAVE', but get %s\n",riff_h.szRiffFormat);
	return -1;
    }

    FMT_BLOCK fmt_h;
    fread(&fmt_h, sizeof(fmt_h), 1, fp);
    if(0 != strncmp((const char*)fmt_h.szFmtID, "fmt ", 4)){
        fprintf(stderr, "bad audio format, expect 'fmt ',bug get %s\n",fmt_h.szFmtID);
	return -1;
    }
    if(18 == fmt_h.dwFmtSize){
        unsigned short ultraMessage;
        fread(&ultraMessage, 1, 2, fp);
    }

    int samplesPerSec = fmt_h.dwSamplesPerSec;
    int bytesPerSec = fmt_h.dwAvgBytesPerSec; 
    int blockAlign = fmt_h.wBlockAlign;
    int bitsPerSample = fmt_h.wBitsPerSample;

    if(bytesPerSec != (fmt_h.wChannels * samplesPerSec * bitsPerSample / 8)){
        fprintf(stderr, "bad audio format\n");
	return -1;
    }

    
    fseek(fp, 0, SEEK_SET);
    fseek(fp, 44, SEEK_SET);
    
    char * databuff = (char*)malloc(sizeof(char) * (fileLen - 44));
    fread(databuff, 1, fileLen - 44, fp);
   
    FILE *pcm_fp = fopen(pcmfile, "w");
    if(NULL == pcm_fp){
        fprintf(stderr, "open file %s fail.\n", pcmfile);
        return -1;
    }
    fwrite(databuff, 1, fileLen - 44, pcm_fp);
    if(NULL != fp)  fclose(fp);
    if(NULL != databuff) free(databuff);
    if(NULL != pcm_fp) fclose(pcm_fp);
    return 0;
}
