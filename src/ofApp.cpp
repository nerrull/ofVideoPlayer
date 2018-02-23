#include "ofApp.h"

ofDirectory dir;

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFullscreen(true);
    //ofGetWindowPtr()->setVerticalSync(true);
    ofSetVerticalSync(true);
    ofSetFrameRate(30);
    ofSetBackgroundAuto(false);
    std::cout << "listening for osc messages on port " << TO_PLAY_PORT << "\n";
    std::cout << "sending osc messages on port " << TO_PLAY_PORT << "\n";

    receiver.setup( TO_PLAY_PORT );
    sender.setup("localhost",PLAYING_FILE_NAME_PORT);

    string path = "/media/rice1902/OuterSpace1/dataStore/VIDEO/mjpeg";
    ofSetDataPathRoot(path);
    dir= ofDirectory(path);
    dir.allowExt("mov");
    dir.listDir();

//    movieFile = dir.getName(1);
//    size_t lastindex = movieFile.find_last_of(".");
//    string rawname = movieFile.substr(0, lastindex);

    //ofSetFrameRate(30);
    videoManager = new HapPlayerManager(&playing_queue, &playing_mutex);
    videoManager->loadAllVideos(dir);
   // videoManager->receiveVideo(rawname);
    videoManager->update();
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

    soundStream.printDeviceList();
    soundStream.setDeviceID(6); //Is computer-specific
    soundStream.setup(this, 1, 0, SAMPLE_RATE, AUDIO_BUFFER_LENGTH, 4);

}

//--------------------------------------------------------------
void ofApp::update(){
    uint64_t mainUpdateTime = ofGetElapsedTimeMillis();
    getMessages();
    if (NEW_VIDEOS){
        videoManager->setToPlay(toPlay);
        NEW_VIDEOS = False;
    }
    videoManager->update();

    //seekInVideo();
    //if (ADD) addVideo();

    if (FBO_DIRTY){
        fbo.begin();
        if (USE_FRAMES){
            t.draw(0,0);
        }
        else{
            videoManager->draw(0,0);
        }
        fbo.end();
        FBO_DIRTY = false;
    }

    if (playing_queue.size()>0){
        sendPlayingFile();

    }
    mainUpdateTime = ofGetElapsedTimeMillis() -mainUpdateTime;
    ofLogError()<<"Main update time: "<< mainUpdateTime;
}


void ofApp::addVideo() {
    static int lastMod = ofGetElapsedTimeMillis();
    static int moviecount = 0;
    if (ofGetElapsedTimeMillis() - lastMod > 50) {
        //moviecount=rand()%dir.size();
        moviecount = (moviecount+1)%dir.size();
        movieFile = dir.getName(moviecount);
        size_t lastindex = movieFile.find_last_of(".");
        string rawname = movieFile.substr(0, lastindex);
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Append called from main loop " <<moviecount << " -" << rawname ;
        videoManager->receiveVideo(rawname);
        lastMod = ofGetElapsedTimeMillis();
    }
}


//--------------------------------------------------------------
void ofApp::draw(){
    uint64_t drawUpdateTime = ofGetElapsedTimeMillis();
    float w = videoManager->getWidth();
    float h = videoManager->getHeight();
//    if (USE_FRAMES){

//        t.draw(0,0);
//    }
//    else{
//        videoManager.draw(0,0 , w, h);
//    }
    fbo.draw(0,0);

    ofSetColor(255);
    std::stringstream strm;
    strm << "FPS: " << ofGetFrameRate()<<endl;
    strm << "SPEED: " << SPEED<<endl;

    ofDrawBitmapString(strm.str(),20, 20);
    FBO_DIRTY = true;
    drawUpdateTime = ofGetElapsedTimeMillis() -drawUpdateTime;
    ofLogError()<<"Draw time: "<< drawUpdateTime;
}

bool ofApp::getPlayingFile(string& filename){
    std::unique_lock<std::mutex> lock(playing_mutex, std::try_to_lock);
    if(!lock.owns_lock()){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Couldn't update playing video";
        return false;
    }
    filename = playing_queue.front();
    playing_queue.clear();
    return true;

}

void ofApp::sendPlayingFile(){
    string name;
    if (!getPlayingFile(name)){
        return;
    }
    ofxOscMessage m;
    m.setAddress("/PLAYING_VIDEO");
    m.addStringArg(name);
    sender.sendMessage(m);
}
void ofApp::setSpeed(int speedIndex){
    switch (speedIndex){
    //full length
    case 0:
        SPEED =10000;
    break;
    case 1:
        SPEED =4000;
    break;
    case 2:
        SPEED =2000;
    break;
    case 3:
        SPEED =1000;
    break;
    case 4:
        SPEED =500;
    break;
    case 5:
        SPEED =250;
    break;
    case 6:
        SPEED =100;
    break;
    case 7:
        SPEED =33;
    break;

    }

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
            NEW_VIDEOS = True;
        }

        if ( m.getAddress().compare( string("/SET_SPEED") ) == 0 )
        {
           setSpeed(m.getArgAsInt32(0));

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
            printf( "WOops" );

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

void ofApp::audioOut(ofSoundBuffer & buffer){

  videoManager->audioOut(buffer);
}
