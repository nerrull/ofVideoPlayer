#include "databaseloader.h"

DatabaseLoader::DatabaseLoader()
{

}

void DatabaseLoader::loadVideoPaths(string database_path, string video_dir, string audio_dir){
//    ofSetLogLevel(OF_LOG_VERBOSE);
    ofxHDF5File hdf5File;
    hdf5File.open(database_path, true);
    cout << "File '" << database_path << "' has " << hdf5File.getNumGroups() << " groups and " << hdf5File.getNumDataSets() << " datasets" << endl;
    for (int i = 0; i < hdf5File.getNumGroups(); ++i) {
        cout << "  Group " << i << ": " << hdf5File.getGroupName(i) << endl;
    }
    for (int i = 0; i < hdf5File.getNumDataSets(); ++i) {
        cout << "  DataSet " << i << ": " << hdf5File.getDataSetName(i) << endl;
    }

    ofxHDF5DataSetPtr video_names_ptr= hdf5File.loadDataSet("video_names");
    num_videos = video_names_ptr->getDimensionSize(0);

    char * name = new char[STRING_LENGTH];
    uint8_t * characters = new uint8_t[num_videos*STRING_LENGTH];
    video_names_ptr->read(characters);
    for( int row =0; row<num_videos; row++){
        std::copy(characters +row*STRING_LENGTH,characters +(row+1)*STRING_LENGTH,name );
        ostringstream oss, oss_audio;
        oss<< name<<".mov";
        oss_audio<< audio_dir<< name<<".wav";
        video_names.push_back(oss.str());
        audio_names.push_back(oss_audio.str());
    }
    hdf5File.close();
}

