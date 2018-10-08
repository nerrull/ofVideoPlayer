#ifndef DATABASELOADER_H
#define DATABASELOADER_H

#include "ofMain.h"
#include "ofxHDF5.h"

#define STRING_LENGTH 40

class DatabaseLoader
{
public:
    DatabaseLoader();
    void loadVideoPaths(string, string, string);
    vector<string>  getVideoPaths(){return video_names;}
    vector<string>  getAudioPaths(){return audio_names;}

private:
    int num_videos;
    vector<string> video_names;
    vector<string> audio_names;
};

#endif // DATABASELOADER_H
