
#ifndef HAPPLAYERMANAGER_H
#define HAPPLAYERMANAGER_H

#include "ofMain.h"
#include "audiosampleplayer.h"
#include "ofxHapPlayer.h"
#define MAX_VIDEOS 8

class HapPlayerManager:public ofThread{

public:

    int playingVideoIndex;
    int lastVideoIndex;


    HapPlayerManager(deque<string>*, ofMutex*);
    ~HapPlayerManager();
    void setVolume(float _volume);
    void receiveVideo(string path);
    void setToPlay(vector<string> toPlay);
    void setSpeed(int speed);
    void loadAllVideos(ofDirectory dir);
    void update();
    bool draw(int x, int y);


    void audioOut(ofSoundBuffer& buffer);
    float getWidth() {return players[playingVideoIndex]->video.isLoaded() ? players[playingVideoIndex]->video.getWidth() : 0;}
    float getHeight() {return players[playingVideoIndex]->video.isLoaded() ? players[playingVideoIndex]->video.getHeight() : 0;}



private:
    /* Fades */

    const string state_string[7] = {"empty", "loading", "priming", "ready", "playing", "played", };

    enum PStatus { empty, loading,priming, ready, playing, played};


    struct player {
        ofxHapPlayer video;
        bool          fade;
        int           loadTime;
        float         maxVol;
        PStatus       status;
        string        videoID;
        string        filePath;


    };

    struct command {
        string type;
        string path;
        bool fade;
    };
    deque<string>* playing_queue;
    ofMutex* playing_mutex;

    deque<command> queue;
    vector<string> toPlay;

    vector<player*> players;
    AudioSamplePlayer samplePlayer;
    string videoPath;
    uint64_t switch_timer;
    int switch_ms=330;

    bool alreadyLoaded(string _path);
    void _playNextVideo();
    bool LOADING;
    bool PLAYING;
    bool OVERLAY;
    int loadIndex;

    string getFileName(string );
    bool loadVideo(string _path);
    int getNextPlayerFromIndex(int playerIndex);
    int getFreePlayerFromIndex(int playerIndex);
    void emptyOldVideos(vector<string> toPlay);
    void setAllVolumes(float);
    virtual void threadedFunction();

    uint64_t call_time;
    int internal_counter = 0;




};

#endif // OFXTHREADEDVIDEOPLAYERMANAGER_H
