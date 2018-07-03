#ifndef OFXTHREADEDVIDEOPLAYERMANAGER_H
#define OFXTHREADEDVIDEOPLAYERMANAGER_H
#include "ofMain.h"
#include "audiosampleplayer.h"

#define MAX_VIDEOS 30


class ThreadedVideoPlayerManager:public ofThread{

public:
    ThreadedVideoPlayerManager(deque<string>*, ofMutex*);
    ~ThreadedVideoPlayerManager();
    void setVolume(float _volume);
    void receiveVideo(string path);
    void setToPlay(vector<string> toPlay);
    void setSpeed(int speed);


    int playingVideoIndex;
    int lastVideoIndex;
    int loadingPlayerIndex;
    void update();
    bool draw(int x, int y);

    void audioOut(ofSoundBuffer& buffer);

    float getWidth() {return players[playingVideoIndex].video.isLoaded() ? players[playingVideoIndex].video.getWidth() : 0;}
    float getHeight() {return players[playingVideoIndex].video.isLoaded() ? players[playingVideoIndex].video.getHeight() : 0;}



private:
    /* Fades */

    const string state_string[7] = {"empty", "loading", "priming", "ready", "playing", "played", };

    enum PStatus { empty, loading,priming, ready, playing, played};


    struct player {
        ofVideoPlayer video;
        bool          fade;
        int           loadTime;
        int           primeTime;

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

    vector<player> players;
    AudioSamplePlayer samplePlayer;
    string videoPath;
    uint64_t switch_timer;
    uint64_t LOADING_TIMER;

    int switch_ms=330;
    bool sampler_active= true;
    bool LOADING;
    bool PLAYING;

    player* playingVideoPointer;
    player* lastVideoPointer;
    player* loadingPlayerPointer;


    string getFileName(string );
    bool loadVideo(string _path);
    int getNextPlayerFromIndex(int playerIndex);
    //player* getNextPlayerFromIndex(int playerIndex);

    int getFreePlayerFromIndex(int playerIndex);
    //player* getFreePlayerFromIndex(int playerIndex);

    void emptyOldVideos(vector<string> toPlay);
    void setAllVolumes(float);

    bool alreadyLoaded(string _path);
    bool idIsInToPlay(string);

    void _playNextVideo();





};

#endif // OFXTHREADEDVIDEOPLAYERMANAGER_H
