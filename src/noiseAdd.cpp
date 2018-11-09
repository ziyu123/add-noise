#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "readWave.h"

#define MAX_LINE_NUM 10000

typedef struct recodList{
    int index;
	char name[1024];
}* RecodList;


int main(int argc, char* argv[]){
    if(argc < 3 || argc > 5){
        printf("Usage: %s noiselib.list trans.scp [snr_low] [snr_high]\n", argv[0]);
        printf("      noiselib.list : noise file list");
        printf("      trans.scp     : wave file from source to destion, form:\n");
        printf("                      source.wav destination.wav\n");
	printf("      [snr_low]     : snr low threshold, int, default 5\n");
	printf("      [snr_high]    : snr high threshold, int, default 10 \n");
	return -1;
    }
    
    const char * noiselib = argv[1];  
    const char * translist = argv[2];
    int snr_low = SNR_LOW_DEFAULT_VALUE;
    int snr_high = SNR_HIGH_DEFAULT_VALUE;
    if(argc == 4){
        snr_low = atof(argv[3]);
    }else if(argc == 5){
        snr_low = atof(argv[3]);
        snr_high = atof(argv[4]);
    }
    srand(time(NULL));

    int maxRecod = MAX_LINE_NUM;
    RecodList p_recodList = (RecodList)malloc(MAX_LINE_NUM * sizeof(recodList));
    if(NULL == p_recodList){
        printf("malloc recodlist fail.\n");
	return -1;
    }
    memset(p_recodList, 0x00, MAX_LINE_NUM * sizeof(recodList));
    FILE * fp_noise = fopen(noiselib, "r");
    if(NULL == fp_noise){
        printf("open file %s fail.\n", noiselib);
	return -1;
    }
    char line[1024] = {0};
    int line_count = 0;
    while(NULL != fgets(line, 1024, fp_noise)){
        if(line[strlen(line) -1] == '\n'){
	    line[strlen(line) -1] = '\0';
	}
	line_count += 1;
	if(line_count >= maxRecod){
	    maxRecod += 2000;
            p_recodList = (RecodList)realloc(p_recodList, maxRecod * sizeof(recodList));
	    if(NULL == p_recodList){
	        printf("realloc recodlist fail.\n");
		return -1;
	    }
	}
	p_recodList[line_count].index = line_count;
	strcpy(p_recodList[line_count].name, line);
        memset(line, 0x00, 1024);
    }
    printf("noise file numbers : %d \n", line_count);
    if(NULL != fp_noise){
        fclose(fp_noise);
    }
    
    FILE * fp_trans = fopen(translist, "r");
    if(NULL == fp_trans){
        printf("open translist %s fail.\n", translist);
	return -1;
    }
    int process_num = 0;
    int err_num = 0;
    char source[1024] = {0}, destination[1024] = {0};
    while(NULL != fgets(line, 1024, fp_trans)){
        if(line[strlen(line) -1] == '\n'){
            line[strlen(line) -1] = '\0';
	}
	int noiseid = random() % line_count;
	if(0 == noiseid )  noiseid += 1;
        int snr = random() % (snr_high - snr_low) + snr_low; 
	char *res = NULL;
	res = strtok(line, " ");
	while(NULL != res){
	    strcpy(source, res);
            res = strtok(NULL, " ");
	    strcpy(destination, res);
	    break;
	}
	char * noisefile = p_recodList[noiseid].name;
	printf("source file %s, noise file %s, snr %d ", source, noisefile, snr);
	int ret = addRandomNoiseBaseSnr(noisefile, source, destination, snr);
	if(-1 == ret){
            printf("add noise for %s fail.\n", source);
            err_num += 1;
            continue;
	}
	process_num += 1;
    }
    printf("Have process audio file %d, %d error.\n", process_num, err_num);
    if(NULL != fp_trans){
        fclose(fp_trans);
    }
    if(NULL != p_recodList){
	free(p_recodList);
    }
    return 0;
}
