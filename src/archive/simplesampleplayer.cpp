#include "simplesampleplayer.h"

SimpleSamplePlayer::SimpleSamplePlayer()
{
    stream.listDevices();
    stream.setDeviceID(7); //Is computer-specific
    int num_channels = 2;
    int bufferlength = 1024;
    stream.setup(num_channels  , 0, 48000, bufferlength, 1);
    stream.setOutput(fader);
    fader.create_fades(bufferlength,num_channels);

    current_index =0;
}

void SimpleSamplePlayer::init(string audio_file_path, int num_files = -1){
    ofSetDataPathRoot(audio_file_path);

    ofDirectory dir(audio_file_path);
    dir.allowExt("wav");
    dir.listDir();
    if (num_files == -1){
        num_files = dir.size();
    }
    players.resize(num_files);
    playerFileIndexes.resize(num_files);

    for (int i = 0; i<num_files;i++){
        string path = dir.getName(i);
        players[i].load(path);
//      players[i].setMultiPlay(true);
        players[i].setPosition(0.);
        players[i].setPaused(true);
        players[i].setVolume(1.);
        playerFileIndexes[i] = ofFile(path).getBaseName(); // filename without extension
    }
}

ptrdiff_t  SimpleSamplePlayer::getSampleIndexFromName(string name){
    ptrdiff_t pos = find(playerFileIndexes.begin(), playerFileIndexes.end(), name) - playerFileIndexes.begin();
    if(pos >= playerFileIndexes.size()) {
      printf("Not found");
      return 0;
    }
    return pos;
}

void SimpleSamplePlayer::playFromIndex(int i){
       players[i].connectTo(fader);
}

//The fader controls the playback
void SimpleSamplePlayer::playFile(string sample_name, int start_frame){
    ptrdiff_t index= getSampleIndexFromName(sample_name);
    players[index].connectTo(fader);
}
