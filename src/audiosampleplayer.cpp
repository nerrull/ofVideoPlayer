#include "audiosampleplayer.h"

AudioSamplePlayer::AudioSamplePlayer()
{
       mixBuffer = new int16_t[AUDIO_BUFFER_LENGTH];
       BUFFER_READY= False;
       sample_pointer=0;
       last_pointer =-1;
}



void AudioSamplePlayer::loadHDF5Data(string path){
    ofSetLogLevel(OF_LOG_VERBOSE);
    hdf5File.open(path, true);
    cout << "File '" << path << "' has " << hdf5File.getNumGroups() << " groups and " << hdf5File.getNumDataSets() << " datasets" << endl;
    for (int i = 0; i < hdf5File.getNumGroups(); ++i) {
        cout << "  Group " << i << ": " << hdf5File.getGroupName(i) << endl;
    }
    for (int i = 0; i < hdf5File.getNumDataSets(); ++i) {
        cout << "  DataSet " << i << ": " << hdf5File.getDataSetName(i) << endl;
    }

    audio_data_ptr= hdf5File.loadDataSet("audio");
    size_t  data_size = audio_data_ptr->getDataSize();

    int offset = 0;
    num_videos = audio_data_ptr->getDimensionSize(0);
    num_samples = audio_data_ptr->getDimensionSize(1);
    frameByteLength = num_samples*2;


    int stride = 1;
    audioBuffer = new int16_t[num_samples];
    audioBlock = new int16_t[num_samples*num_videos];

    audio_data_ptr->read(audioBlock);

    name_index_ptr= hdf5File.loadDataSet("sample_names");
    size_t  name_size = name_index_ptr->getDataSize();
    int num_names = name_index_ptr->getDimensionSize(0);
    int num_chars = 30;

    uint8_t *characters = new uint8_t[num_names*30];
    name_index_ptr->read(characters);
    char * name = new char[30];

    for( int row =0; row<num_names; row++){
        std::copy(characters +row*30,characters +(row+1)*30,name );
        name_index.push_back(name);
    }
    hdf5File.close();

}

int AudioSamplePlayer::getFrameIndexFromName(string name){
    auto it = std::find(name_index.begin(), name_index.end(), name);
    if (it == name_index.end())
    {
        ofLogError() <<"Received faulty video name: "<< name<<endl;
        return 0;
    } else
    {
      auto index = std::distance(name_index.begin(), it);
      return index;
    }
}

void AudioSamplePlayer::playFile(string id, int frame){
    int index =getFrameIndexFromName(id);
    setPlayingIndex(index, frame);
}


void AudioSamplePlayer::setPlayingIndex(int index, int frame){
    std::unique_lock<ofMutex> lock(audioMutex);
    last_pointer = sample_pointer;
    sample_pointer =index *frameByteLength/2;
    int frame_offset = SAMPLE_RATE/30*frame;
    sample_pointer+=frame_offset;
}


void AudioSamplePlayer::audioOut(ofSoundBuffer & buffer){
    std::unique_lock<ofMutex> lock(audioMutex);
    if (last_pointer !=-1){
//        //fade in fade out
//        float fade_in, fade_out, mult;
//        for(int i = 0; i< AUDIO_BUFFER_LENGTH; i++){
//            mult = ((float)i)/((float)AUDIO_BUFFER_LENGTH/4);
//            fade_in =CLAMP(mult,0.0,1.0);
//            fade_out =CLAMP(1-mult, 0.0,1.0);

//            mixBuffer[i] =(short) audioBlock[sample_pointer+i]*fade_in +audioBlock[last_pointer+i]*fade_out;
//        }

//        buffer.copyFrom(mixBuffer,AUDIO_BUFFER_LENGTH,1, SAMPLE_RATE);
//        sample_pointer +=AUDIO_BUFFER_LENGTH;
//        last_pointer = -1;
//        return;


        short last_value = audioBlock[last_pointer];
        short next_value =audioBlock[sample_pointer+256];
        short diff = next_value - last_value;
        short step = diff/256;

        for(int i = 0; i< AUDIO_BUFFER_LENGTH; i++){
            if (i<256) mixBuffer[i] =(short) last_value +i*step;

            else mixBuffer[i] =(short) audioBlock[sample_pointer+i];
        }

        buffer.copyFrom(mixBuffer,AUDIO_BUFFER_LENGTH,1, SAMPLE_RATE);
        sample_pointer +=AUDIO_BUFFER_LENGTH;
        last_pointer = -1;
        return;

    }

    //memcpy(outBuffer, audioBlock+offset, frameByteLength );
    buffer.copyFrom(audioBlock+sample_pointer,AUDIO_BUFFER_LENGTH,1, SAMPLE_RATE);
    sample_pointer +=AUDIO_BUFFER_LENGTH;

}

