#include "readWave.h"
#include <stdio.h>

int main(int argc, char * argv[]){
    const char* file = argv[1];
	char * dataBuff = NULL;
	int len = 0;
	int samples = 0;
	readData(file, &dataBuff, len, samples);
    return 0;
}
