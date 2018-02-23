#include "ofxThreadedVideoPlayerManager.h"

ThreadedVideoPlayerManager::ThreadedVideoPlayerManager(deque<string> *pq, ofMutex *pm)
{
    this->playing_queue= pq;
    this->playing_mutex = pm;
    string samplePath = "/media/rice1902/OuterSpace1/dataStore/audio_samples.h5";
    videoPath = "/media/rice1902/OuterSpace1/dataStore/VIDEO/mjpeg/";
    samplePlayer.loadHDF5Data(samplePath);
    playingVideoIndex = 0;
    lastVideoIndex =0;
    for (int i=0; i<MAX_VIDEOS; i++){
        players[i].maxVol = 1.0f;
        players[i].status= empty;
        players[i].videoID = "/EMPTY";
    }
    switch_timer = ofGetElapsedTimeMillis();

}

// Destructor
ThreadedVideoPlayerManager::~ThreadedVideoPlayerManager() {
//todo : stop + close all videos
}

void ThreadedVideoPlayerManager::setVolume(float _volume) {
    for (int i=0; i<MAX_VIDEOS; i++){
        players[i].maxVol = _volume;
    }
}


void ThreadedVideoPlayerManager::setSpeed(int speed) {
    int last_ms = switch_ms;
    switch_ms = speed;
    if (last_ms <500 &&switch_ms >500){
        sampler_active =False;
        setAllVolumes(1.0);
    }
    else if (last_ms >500 &&switch_ms <500){
        sampler_active =true;
        setAllVolumes(0.0);
    }
}

void ThreadedVideoPlayerManager::receiveVideo(string path){
    ofLogVerbose() << "Received : " << path;
    ThreadedVideoPlayerManager::command c;
    c.type = "load";
    c.path = path;
    if (lock()){
        queue.push_front(c);
        unlock();
    }
    else {
        ofLogError() << "x loadVideo: couldn't acquire lock";
    }
}

string ThreadedVideoPlayerManager::getFileName(string s){

    std::size_t botDirPos = s.find_last_of("/");
    std::string file = s.substr(botDirPos+1, s.length());
    return file;
}

bool ThreadedVideoPlayerManager::alreadyLoaded(string _path){
      for (int i =0; i<MAX_VIDEOS; i++){
        if (_path.compare( players[i].videoID)==0) return true;
      }
      return false;
}

bool ThreadedVideoPlayerManager::loadVideo(string _path){
    ofLogVerbose() << "Started loading " << _path ;
    string fullpath =videoPath +_path+".mov";
    ofLogError(ofToString(ofGetElapsedTimef(),3)) << "[Loading] " << fullpath << " appended";

    if (alreadyLoaded(_path)){
       ofLogError(ofToString(ofGetElapsedTimef(),3)) << _path << " already loaded";
       return True;
    }

    int freePlayer= getFreePlayerFromIndex(playingVideoIndex);

    if (freePlayer == -1){
        ofLogError() << "Couldn't find a free video player";
        return False;
    }

    ofLogVerbose(ofToString(ofGetElapsedTimef(),3)) << "Load call received";

    players[freePlayer].loadTime = ofGetElapsedTimeMillis();
    players[freePlayer].video.setLoopState(OF_LOOP_NORMAL);
    players[freePlayer].status  = loading;
    players[freePlayer].videoID  = _path;
    players[freePlayer].filePath  = fullpath;
    players[freePlayer].video.loadAsync(fullpath);
    return True;
}

int ThreadedVideoPlayerManager::getNextPlayerFromIndex(int playerIndex){
    int nextIndex = -1;
    for (int i =playerIndex; i<MAX_VIDEOS; i++){
        if (i== playingVideoIndex && players[i].status ==playing) continue;
        if (players[i].status == loading) continue;
        if (players[i].status == ready){
            nextIndex = i;
            break;
        }
        if (players[i].status == played and players[i].video.isPaused()){
            nextIndex = i;
            break;
        }
    }
    if (nextIndex == -1 && playerIndex!=0){
        return getNextPlayerFromIndex(0);
    }
    return nextIndex;
}

void  ThreadedVideoPlayerManager::setAllVolumes(float value){
    for (int i=0; i<MAX_VIDEOS; i++){
        if (players[i].status == empty) continue;
        if (players[i].status == priming) continue;
        players[i].video.setVolume(value);
    }

}

int ThreadedVideoPlayerManager::getFreePlayerFromIndex(int playerIndex){
    int nextIndex = -1;
    for (int i=playerIndex; i<MAX_VIDEOS; i++){
        if (i == playingVideoIndex) continue;
        if (players[i].status == loading) continue;
        if (players[i].status == empty) {
            nextIndex = i;
            break;
        }
        if (players[i].status == played and players[i].video.isPaused()) {
            nextIndex = i;
            break;
        }
    }
    if (nextIndex == -1 && playerIndex!=0){
        return getFreePlayerFromIndex(0);
    }
    return nextIndex;
}


void ThreadedVideoPlayerManager::emptyOldVideos(vector<string> toPlay){
    int nextIndex = -1;
    for (int i=0; i<MAX_VIDEOS; i++){
        if (i == playingVideoIndex) continue;
        else if (players[i].status == loading) continue;
        else if (players[i].status == empty) continue;
        else if (players[i].status == playing) continue;
        //if it's in the list conitnue
        else if (std::find(toPlay.begin(), toPlay.end(), players[i].videoID) != toPlay.end())
        {
            continue;
        }
        //if not set it to empty
        else {
           players[i].status = empty;
        }
    }

}


void ThreadedVideoPlayerManager::_playNextVideo(){
    int playerIndex =playingVideoIndex;
    int nextIndex = getNextPlayerFromIndex(playerIndex);
    if (nextIndex == -1){
        ofLogError() << "Couldn't find a ready player";
        players[playingVideoIndex].video.setFrame(1);
        samplePlayer.playFile(players[playingVideoIndex].videoID, 1);
        return;
    }

    if(players[lastVideoIndex].video.isPlaying()){
       players[lastVideoIndex].status=played;
       players[lastVideoIndex].video.setPaused(true);
       players[lastVideoIndex].video.setFrame(1);

//       if (players[lastVideoIndex].video.getCurrentFrame()>players[lastVideoIndex].video.getTotalNumFrames()-30){
//       }
       //players[lastVideoIndex].status =empty;
    }

    lastVideoIndex = playingVideoIndex;
    playingVideoIndex=nextIndex;

    int frame = players[playingVideoIndex].video.getCurrentFrame();
    if (sampler_active){
        samplePlayer.playFile(players[playingVideoIndex].videoID, frame);
    }
    players[playingVideoIndex].video.setVolume(0.0f);
    players[playingVideoIndex].video.setPaused(false);
    players[playingVideoIndex].status = playing;

    std::unique_lock<std::mutex> lock(*playing_mutex, std::try_to_lock);
    if(!lock.owns_lock()){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Couldn't update playing video";
        return;
    }
    playing_queue->push_front( players[playingVideoIndex].videoID);

}


void ThreadedVideoPlayerManager::setToPlay(vector<string> toPlay){
    this->toPlay =toPlay;
    ofLogError(ofToString(ofGetElapsedTimef(),3)) << "New toPlay vector received";

    if (lock()){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Clearing queue";
        queue.clear();
        unlock();
    }

    for (size_t i = 0; i<toPlay.size(); i++){
        receiveVideo(toPlay[i]);
    }
}


void ThreadedVideoPlayerManager::update(){
    //emptyOldVideos(toPlay);

    if (ofGetElapsedTimeMillis() - switch_timer > switch_ms) {
        _playNextVideo();
        switch_timer = ofGetElapsedTimeMillis();
    }

    if (queue.size()>0) {
        ThreadedVideoPlayerManager::command next_command;
        int t = ofGetElapsedTimeMillis();
        if (lock()) {
            next_command = queue.back();
            queue.pop_back();
            unlock();
        }
        else {
            ofLogError() << "x update(): couldn't acquire lock";
        }

        if (next_command.type == "load") {
            if (!loadVideo(next_command.path)){
                if (lock()) {
                    queue.push_back(next_command);
                    unlock();
                }
            }
        }
    }

    for (int i=0; i<MAX_VIDEOS; i++){
        //Check if loading videos are ready to be started
        if (players[i].status==loading){
            if(players[i].video.isLoaded()) {
                players[i].video.setVolume(0.0f);
                players[i].video.setPaused(false);
                players[i].status =priming;
            }
        }
        //Check if priming videos have loaded their first frame
        else if (players[i].status==priming){
            players[i].video.setPaused(false);
            players[i].video.update();
            if(players[i].video.isPlaying() && players[i].video.getCurrentFrame() >= 1){
                if (!sampler_active){
                    players[i].video.setVolume(1.0);
                }
                players[i].video.setPaused(true);
                players[i].status =ready;
                players[i].loadTime = ofGetElapsedTimeMillis() - players[i].loadTime;
            }
        }
    }

    if (players[playingVideoIndex].status==playing){
        players[playingVideoIndex].video.update();
    }
    if (players[lastVideoIndex].status==playing){
        players[lastVideoIndex].video.update();
    }
}
bool ThreadedVideoPlayerManager::draw(int x, int y){

    static bool isDrawing = false;
    int current_pos = players[playingVideoIndex].video.getCurrentFrame();
    int total_pos = players[playingVideoIndex].video.getTotalNumFrames();
    int volumeSteps =1;
    float volume = players[playingVideoIndex].maxVol *current_pos/float(volumeSteps);
    ofPushStyle();


    players[playingVideoIndex].video.draw(x, y);
    ofDisableBlendMode();
    ofPopStyle();
    isDrawing = true;

    ofPushStyle();
    ofSetColor(0, 0, 0, 150);
    int w = ofGetWidth();
    int h = ofGetHeight();
    ofDrawRectangle(w-w/4-2, 0, w/4+2, h);
    ofNoFill();
    ofSetColor(255, 0, 0);



    ostringstream os;
    os << "Queue size"  << queue.size()<< endl;

    os << "Current video"  << endl;
    os << "Index    : " << playingVideoIndex << endl;
    os << "Load    : " << players[playingVideoIndex].loadTime << endl;
    os << "Frame   : " << current_pos << "/" << total_pos << endl;
    os << "ID      : " <<  getFileName(players[playingVideoIndex].videoID) <<endl;
    os << "State   : " << state_string[players[playingVideoIndex].status ]<< endl << "------------------" << endl;


    os << "Last video"  << endl;
    os << "Index    : " << lastVideoIndex << endl;
    os << "ID      : " <<  getFileName(players[lastVideoIndex].videoID)<< endl;
    os << "Load    : " << players[lastVideoIndex].loadTime << endl;
    os << "Frame   : " << players[lastVideoIndex].video.getCurrentFrame() << "/" << players[lastVideoIndex].video.getTotalNumFrames() << endl;
    os << "Playing : " << players[lastVideoIndex].video.isPlaying() << endl;
    os << "State   : " << state_string[ players[lastVideoIndex].status] << endl << "------------------" << endl;

    for (int i=0; i<MAX_VIDEOS; i++){
        os << "Player  :"  << i<<endl;
        os << "State   : " << state_string[ players[i].status] << endl;
        os << "Path    : " << players[i].filePath << endl;
        os << "ID      : " <<  getFileName(players[i].videoID) << endl << "------------------" << endl;

    }


    ofDrawBitmapString(os.str(), w-w/4+2, 17);
    ofPopStyle();

    return isDrawing;

}

void ThreadedVideoPlayerManager::audioOut(ofSoundBuffer& buffer){
    if (sampler_active){
        samplePlayer.audioOut(buffer);
    }

}

