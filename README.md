# add-noise
add noise of a certain SNR to audio files
build:
*  cd src
*  gcc noiseAdd.cpp  readWave.cpp  -lm -o ../bin/add_noise

usage:
   ./bin/add_noise
