#ifndef AUDIOSAMPLEPLAYER_H
#define AUDIOSAMPLEPLAYER_H

#include "ofMain.h"
#include "ofxHDF5.h"

#define AUDIO_BUFFER_LENGTH 1024
#define SAMPLE_RATE 48000

class AudioSamplePlayer
{
public:
    AudioSamplePlayer();

    void loadHDF5Data(string path);
    int getFrameIndexFromName(string name);
    void setPlayingIndex(int index, int frame);
    void playFile(string file, int frame);

    void audioOut(ofSoundBuffer &buffer);


    ofMutex audioMutex;

    ofxHDF5DataSetPtr audio_data_ptr;
    ofxHDF5DataSetPtr name_index_ptr;
    vector<string> name_index;
    uint64_t sample_pointer;
    uint64_t last_pointer;


    int num_videos;
    int num_samples;
    int frameByteLength;
    int16_t* audioBuffer;
    int16_t* audioBlock;
    int16_t* mixBuffer;

    ofxHDF5File hdf5File;

    bool BUFFER_READY=false;

};

#endif // AUDIOSAMPLEPLAYER_H
