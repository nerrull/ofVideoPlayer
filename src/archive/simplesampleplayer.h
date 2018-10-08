#ifndef SIMPLESAMPLEPLAYER_H
#define SIMPLESAMPLEPLAYER_H

#include "ofMain.h"
#include "ofxTwoChannelFader.h"
#include "ofxSoundPlayerObject.h"
#include "ofxSoundMixer.h"


class SimpleSamplePlayer
{
public:
    SimpleSamplePlayer();
    ptrdiff_t  getSampleIndexFromName(string name);
    void playFromIndex(int i);

    void playFile(string sample_name, int start_frame);
    void init (string path, int num_files);

    //ofxSoundMixer mixer;
    ofxTwoChannelFader fader;

    vector<ofxSoundPlayerObject> players;
    vector<string> playerFileIndexes;

    ofxSoundOutput output;
    ofSoundStream stream;
    int current_index;
    int last_index;


};

#endif // SIMPLESAMPLEPLAYER_H
