#include "ofApp.h"
#include "ofxJsonSettings.h"


//--------------------------------------------------------------
void ofApp::setup(){

    //ofSetFullscreen(true);
    //ofGetWindowPtr()->setVerticalSync(true);
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    ofSetBackgroundAuto(false);
    std::cout << "listening for osc messages on port " << TO_PLAY_PORT << "\n";
    std::cout << "sending osc messages on port " << TO_PLAY_PORT << "\n";

    receiver.setup( TO_PLAY_PORT );
    sender.setup("localhost",PLAYING_FILE_NAME_PORT);

    Settings::get().load("settings.json");
    string video_path = Settings::getString("video_path");
    string db_path = Settings::getString("db_path");
    string audio_path =  Settings::getString("audio_path");
    DEBUG_MODE=  Settings::getBool("debug_mode");
    OVERLAY = Settings::getBool("overlay");

    debugTimer = 1.;
    randomTime = 0.;
    dbl.loadVideoPaths(db_path,video_path, audio_path);

    videoManager = new HapPlayerManager(&playingQueue, &playing_mutex, &dbl);
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
}

//--------------------------------------------------------------
void ofApp::update(){
    uint64_t mainUpdateTime = ofGetElapsedTimeMillis();
    currentTime = ofGetElapsedTimef();

    if (DEBUG_MODE && (currentTime - randomTime) >debugTimer ){
        videoManager->playRandom();
        randomTime =currentTime;
        debugTimer = ofRandom(0., 5.);
    }
    getMessages();
    if (NEW_VIDEOS){
        if (PLAY_IMMEDIATELY){
            videoManager->playNow(toPlay[0]);
            PLAY_IMMEDIATELY = false;
        }
        else{
            videoManager->readToPlay(toPlay);
        }
        NEW_VIDEOS = false;
    }

    videoManager->update();

    if (FBO_DIRTY){
        fbo.begin();
        videoManager->draw(0,0);
        fbo.end();
        FBO_DIRTY = false;
    }

    if (playingQueue.size() >0){
        sendPlayingFile();
    }

    mainUpdateTime = ofGetElapsedTimeMillis() -mainUpdateTime;
    //ofLogError()<<"Main update time: "<< mainUpdateTime;
}


//--------------------------------------------------------------
void ofApp::draw(){
    uint64_t drawUpdateTime = ofGetElapsedTimeMillis();
    float w = videoManager->getWidth();
    float h = videoManager->getHeight();
    fbo.draw(0,0);
    FBO_DIRTY = true;

    ofSetColor(255);
    if (OVERLAY){
        std::stringstream strm;
        strm << "FPS: " << ofGetFrameRate()<<endl;
        strm << "SPEED: " << SPEED<<endl;
        w = ofGetWidth();
        ofDrawBitmapString(strm.str(), w-w/4+2, 20);
    }

    drawUpdateTime = ofGetElapsedTimeMillis() -drawUpdateTime;
    // ofLogError()<<"Draw time: "<< drawUpdateTime;
}

bool ofApp::getPlayingFileInfo(string& filename, float& length, bool& isLoop){
    std::unique_lock<std::mutex> lock(playing_mutex, std::try_to_lock);
    if(!lock.owns_lock()){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Couldn't update playing video";
        return false;
    }
    HapPlayerManager::PlayingInfo pi = playingQueue.front();
    filename = pi.fileName;
    length = pi.durationS;
    isLoop = pi.isLoop;
    playingQueue.clear();
    return true;

}

void ofApp::sendPlayingFile(){
    string name;
    float duration;
    bool isLoop;
    if (!getPlayingFileInfo(name, duration, isLoop)){
        return;
    }
    ofxOscMessage m;
    m.setAddress("/PLAYING_VIDEO");
    m.addStringArg(name);
    m.addInt64Arg(duration);
    m.addBoolArg(isLoop);
    sender.sendMessage(m);
}

void ofApp::getMessages() {

    //Not the bottleneck
    // check for waiting messages
    while( receiver.hasWaitingMessages() )
    {
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage( &m );

        if ( m.getAddress().compare( string("/VIDEO_NAMES") ) == 0 )
        {
            // the single argument is a string
            //strcpy( next_video, m.getArgAsString( 0 ) );
            int num_videos = m.getArgAsInt32( 0 );
            lastToPlay = toPlay;
            toPlay.clear();
            for (int i = 1; i<= num_videos;i++){
                string name = m.getArgAsString( i );
                toPlay.push_back(name);
                ofLogError()<<"received " <<name;
            }
            NEW_VIDEOS = true;
        }

        if ( m.getAddress().compare( string("/PLAY_NOW") ) == 0 )
        {
            // the single argument is a string
            //strcpy( next_video, m.getArgAsString( 0 ) );
            lastToPlay = toPlay;
            toPlay.clear();
            string name = m.getArgAsString( 0);
            toPlay.push_back(name);
            NEW_VIDEOS = true;
            PLAY_IMMEDIATELY = true;
        }

        // check for mouse button message
        else if ( m.getAddress().compare( string("/FILE") ) == 0 )
        {
            // the single argument is a string
            //strcpy( next_video, m.getArgAsString( 0 ) );
            next_video = string(m.getArgAsString( 0 ));

            if (next_video != movieFile) {
                movieFile = next_video;
                videoManager->receiveVideo(movieFile);
            }

            ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Video received : " << next_video ;
            ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Current video : " << movieFile ;
        }
        else
        {
            printf( "Received unknown message" );

        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key){
    case ' ':
        ADD= !ADD;
        break;
    case 'w':
        break;
    case 's':

        break;
    case 'd':
        SPEED +=33;
        videoManager->setSpeed(SPEED);
        break;
    case 'a':
        SPEED -=33;
        SPEED =max(SPEED,33);

        videoManager->setSpeed(SPEED);
        break;

    case 'h':
        OVERLAY = !OVERLAY;
        videoManager->toggleOverlay();
        break;
    case '0':
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

//void ofApp::audioOut(ofSoundBuffer & buffer){

//  videoManager->audioOut(buffer);
//}
