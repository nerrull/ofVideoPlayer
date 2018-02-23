#include "ofApp.h"

ofDirectory dir;

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    std::cout << "listening for osc messages on port " << PORT << "\n";
    receiver.setup( PORT );
    current_msg_string = 0;

    string path = "/media/rice1902/OuterSpace1/dataStore/VIDEO/mjpeg";
    ofSetDataPathRoot(path);
    dir= ofDirectory(path);
    dir.allowExt("mov");
    dir.listDir();
    movieFile = dir.getPath(1);

    MO.setPreview(true);
    MO.start();
    MO.update();

    MO.appendMovie(movieFile, false, false);

}

//--------------------------------------------------------------
void ofApp::update(){
    MO.update();

    //seekInVideo();
    //playOscVideos();
    cycleVideos();


}


void ofApp::playOscVideos() {
    getMessages();
}



void ofApp::seekInVideo() {
    if (SEEK){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Seeking to : " << SEEK_FRAME ;

        MO.seek(SEEK_FRAME);
        SEEK = false;
    }
}


void ofApp::cycleVideos() {
    static int lastMod = ofGetElapsedTimeMillis();

    static int action = 0;
    static int moviecount = 1;
    if (ofGetElapsedTimeMillis() - lastMod > 60) {
        if (action == 0) {

            moviecount=rand()%dir.size();
            movieFile = dir.getPath(moviecount);
            ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Append called from main loop " << movieFile ;

            MO.appendMovie(movieFile, false, false);

        }
        lastMod = ofGetElapsedTimeMillis();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    float w = MO.getWidth();
    float h = MO.getHeight();
    MO.draw(0,0 , w, h);


    ofSetColor(255);
    std::stringstream strm;
    strm << "fps: " << ofGetFrameRate();
    ofDrawBitmapString(strm.str(),20, 20);
}

void ofApp::getMessages() {
    // hide old messages
    for ( int i=0; i<NUM_MSG_STRINGS; i++ )
    {
        if ( timers[i] < ofGetElapsedTimef() )
            msg_strings[i] = "";
    }

    // check for waiting messages
    while( receiver.hasWaitingMessages() )
    {
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage( &m );


        // check for mouse button message
        if ( m.getAddress().compare( string("/FILE") ) == 0 )
        {
            // the single argument is a string
            //strcpy( next_video, m.getArgAsString( 0 ) );
            next_video = string(m.getArgAsString( 0 ));

            if (next_video != movieFile) {
                movieFile = next_video;
                MO.appendMovie(movieFile, false, false);
            }

            ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Video received : " << next_video ;
            ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Current video : " << movieFile ;


        }
        else
        {
            printf( "WOops" );

        }

    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key){
        case ' ':
            SEEK= true;
            break;
        case 'w':
            SEEK_FRAME +=1;
            if (SEEK_FRAME > 100)
                SEEK_FRAME =100;
            SEEK= true;
            break;
        case 's':
            SEEK_FRAME -=1;
            if (SEEK_FRAME < 0)
                SEEK_FRAME =0;
            SEEK= true;
            break;
        case 'd':
            SPEED +=0.1;
            MO.setSpeed(SPEED);
            break;
        case 'a':
            SPEED -=0.1;
            MO.setSpeed(SPEED);
            break;

    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
