# add-noise
add noise of a certain SNR to audio files
## build:
- cd src
- gcc noiseAdd.cpp  readWave.cpp  -lm -o ../bin/add_noise
- gcc rm44head.cpp  readWave.cpp  -lm -o ../bin/rm44head
- gcc wav2pcm.cpp  readWave.cpp -lm -o ../bin/wav2pcm 
## usage:
- ./bin/add_noise
- ./bin/r44head
- ./bin/wav2pcm
