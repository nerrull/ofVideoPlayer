#include "simplesampleplayer.h"

SimpleSamplePlayer::SimpleSamplePlayer()
{
    stream.setDeviceID(6); //Is computer-specific
    int num_channels = 1;
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
//        players[i].setMultiPlay(true);
        players[i].setPosition(0.);
        players[i].setPaused(True);
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


void SimpleSamplePlayer::playFile(string sample_name, int start_frame){
    ptrdiff_t index= getSampleIndexFromName(sample_name);
//    if (!players[index].isPlaying()){
//        players[index].play();
//    }
    players[index].connectTo(fader);
}

void SimpleSamplePlayer::update(){

//    if (players[last_index].isPlaying()){
//        float v = mixer.getChannelVolume(last_index);
//        if( v <0.2){
//            players[last_index].setPaused(True);
//            players[last_index].setPosition(0.);
//            mixer.setChannelVolume(last_index, 0.);
//        }

//        mixer.setChannelVolume(last_index,v*0.95);
//    }

//    if (players[current_index].isPlaying()){
//        float v = mixer.getChannelVolume(current_index);
//        if( v==1.0){
//           return;
//        }
//        else if( v==0.0){
//            mixer.setChannelVolume(current_index,0.1);
//        }

//        else if( v>0.9){
//            mixer.setChannelVolume(current_index,1.);
//        }
//        else {
//            mixer.setChannelVolume(current_index,v*1.1);
//        }
//    }
}
