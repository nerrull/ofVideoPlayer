#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){

    //ofSetFullscreen(true);
    //ofGetWindowPtr()->setVerticalSync(true);
    ofSetVerticalSync(true);
    ofSetFrameRate(60);

    //htopofSetFrameRate(30);
    ofSetBackgroundAuto(false);
    std::cout << "listening for osc messages on port " << TO_PLAY_PORT << "\n";
    std::cout << "sending osc messages on port " << TO_PLAY_PORT << "\n";

    receiver.setup( TO_PLAY_PORT );
    sender.setup("localhost",PLAYING_FILE_NAME_PORT);

    string video_path = "/media/rice1902/OuterSpace2/dataStore/VIDEO/hap/";
    string audio_path = "/media/rice1902/OuterSpace2/dataStore/AUDIO/full_audio/";

    //    string path = "/media/rice1902/Seagate4T/hap_test/";
    ofSetDataPathRoot(video_path);
    dir= ofDirectory(video_path);
    dir.allowExt("mov");
    dir.listDir();

    videoManager = new HapPlayerManager(&playing_queue, &playing_mutex);
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    videoManager->loadAllVideos(dir);
}

//--------------------------------------------------------------
void ofApp::update(){
    uint64_t mainUpdateTime = ofGetElapsedTimeMillis();
    getMessages();
    if (NEW_VIDEOS){
        if (PLAY_IMMEDIATELY){
            videoManager->playNow(toPlay[0]);
            PLAY_IMMEDIATELY = False;
        }
        else{
            videoManager->readToPlay(toPlay);
        }
        NEW_VIDEOS = False;
    }
    videoManager->update();

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
    //ofLogError()<<"Main update time: "<< mainUpdateTime;
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
    w = ofGetWidth();
    ofDrawBitmapString(strm.str(), w-w/4+2, 20);
    FBO_DIRTY = true;
    drawUpdateTime = ofGetElapsedTimeMillis() -drawUpdateTime;
    // ofLogError()<<"Draw time: "<< drawUpdateTime;
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
        SPEED =-1;
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
        SPEED =66;
        break;
    case 8:
        SPEED =33;
        break;
    }
    videoManager->setSpeed(SPEED);


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

        if ( m.getAddress().compare( string("/PLAY_NOW") ) == 0 )
        {
            // the single argument is a string
            //strcpy( next_video, m.getArgAsString( 0 ) );
            lastToPlay = toPlay;
            toPlay.clear();
            string name = m.getArgAsString( 0);
            toPlay.push_back(name);
            NEW_VIDEOS = True;
            PLAY_IMMEDIATELY = True;

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

    case 'h':
        videoManager->toggleOverlay();
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
