#pragma once

#include "ofMain.h"
#include "ofxThreadedVideoPlayerManager.h"
#include "ofxOsc.h"
#include "imageloader.h"
#include "happlayermanager.h"


// listen on port 33333
#define TO_PLAY_PORT 33333
//Publish on port 44445
#define PLAYING_FILE_NAME_PORT 44445
#define NUM_MSG_STRINGS 20

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    ofTrueTypeFont		font;


private:

    ofxOscReceiver receiver;
    ofxOscSender sender;
    HapPlayerManager* videoManager;
    //ThreadedVideoPlayerManager* videoManager;


    ofTexture t;
    ofFbo fbo;

    std::string		msg_strings[NUM_MSG_STRINGS];
    string		next_video;
    string		movieFile;

    int SEEK_FRAME = 0;
    int SPEED = 33*10;
    int count =0;
    bool SEEK = false;
    bool USE_FRAMES = false;
    bool ADD = false;
    bool FBO_DIRTY = true;
    bool NEW_VIDEOS =false;
    bool FIRST_UPDATE = true;
    bool PLAY_IMMEDIATELY = false;
    bool DEV_MODE;


    vector<string> toPlay;
    vector<string> lastToPlay;
    deque<HapPlayerManager::PlayingInfo> playingQueue;
    ofMutex playing_mutex;
    ofDirectory dir;



    //void audioOut(ofSoundBuffer & buffer);
    void cycleVideos();
    void getMessages();
    void seekInVideo();
    void addVideo();
    bool getPlayingFileInfo(string& ,int &);
    void sendPlayingFile();
    void setSpeed(int speedIndex);

    int SPEEDS [19]= {-1, 4000, 3000, 2000, 1500, 1000,900,800,700,600, 500,400, 300, 250, 200, 150, 100, 66, 33};




};
