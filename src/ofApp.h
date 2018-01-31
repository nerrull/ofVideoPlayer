#pragma once

#include "ofMain.h"
#include "ofxGaplessVideoplayer.h"
#include "ofxOsc.h"

// listen on port 12345
#define PORT 33333
#define NUM_MSG_STRINGS 20

class ofApp : public ofBaseApp{

public:
    ofxGaplessVideoPlayer   MO;
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
    int				current_msg_string;
    std::string		msg_strings[NUM_MSG_STRINGS];
    float	timers[NUM_MSG_STRINGS];
    string		next_video;
    string		movieFile;
    void cycleVideos();
    void getMessages();
    void playOscVideos();
    void seekInVideo();
    int SEEK_FRAME = 0;
    bool SEEK = false;
    float SPEED = 1.0;



};
