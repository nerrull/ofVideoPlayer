#include "ofxThreadedVideoPlayerManager.h"

ThreadedVideoPlayerManager::ThreadedVideoPlayerManager(deque<string> *pq, ofMutex *pm)
{
    this->playing_queue= pq;
    this->playing_mutex = pm;
    string samplePath = "/media/rice1902/OuterSpace1/dataStore/audio_samples.h5";
    videoPath = "/media/rice1902/OuterSpace1/dataStore/VIDEO/mjpeg/";
    samplePlayer.loadHDF5Data(samplePath);

    LOADING=false;
    PLAYING = false;

    for (int i=0; i<8; i++){
        player p;
        p.maxVol = 1.0f;
        p.status= empty;
        p.videoID = "/EMPTY";
        players.push_back(p);
    }
    playingVideoPointer = &players[0];
    lastVideoPointer = &players[0];
    playingVideoIndex =0;
    switch_timer = ofGetElapsedTimeMillis();

}

// Destructor
ThreadedVideoPlayerManager::~ThreadedVideoPlayerManager() {
//todo : stop + close all videos
}

void ThreadedVideoPlayerManager::setVolume(float _volume) {
    for (int i=0; i<players.size(); i++){
        players[i].maxVol = _volume;
    }
}


void ThreadedVideoPlayerManager::setSpeed(int speed) {
    int last_ms = switch_ms;
    switch_ms = speed;
    if (last_ms <500 &&switch_ms >500){
        sampler_active =false;
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
      for (int i =0; i<players.size(); i++){
        if (_path.compare( players[i].videoID)==0) return true;
      }
      return false;
}

bool ThreadedVideoPlayerManager::loadVideo(string _path){
    string fullpath =videoPath +_path+".mov";
    ofLogError(ofToString(ofGetElapsedTimef(),3)) << fullpath << " appended";

    if (alreadyLoaded(_path)){
       ofLogError(ofToString(ofGetElapsedTimef(),3)) << _path << " already loaded";
       return true;
    }

    if (LOADING){
        ofLogError() << "Already loading, waiting for last load to finish";
        return false;
    }
    int nextIndex = getFreePlayerFromIndex(playingVideoIndex);

    if (nextIndex == -1){
        ofLogError() << "Couldn't find a free video player";
        return false;
    }
    loadingPlayerPointer =&players[nextIndex];

    LOADING =true;

    ofLogError() << "Started loading " << _path ;

    loadingPlayerPointer->loadTime = ofGetElapsedTimeMillis();
    loadingPlayerPointer->video.close();
    loadingPlayerPointer->video.setLoopState(OF_LOOP_NORMAL);
    loadingPlayerPointer->status  = loading;
    loadingPlayerPointer->videoID  = _path;
    loadingPlayerPointer->filePath  = fullpath;
    loadingPlayerPointer->video.loadAsync(fullpath);
    LOADING_TIMER=0;
    return false;
}

int ThreadedVideoPlayerManager::getNextPlayerFromIndex(int playerIndex){
    int nextIndex = -1;
    for (int i =playerIndex; i<players.size(); i++){
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
    for (int i=0; i<players.size(); i++){
        if (players[i].status == empty) continue;
        if (players[i].status == priming) continue;
        players[i].video.setVolume(value);
    }

}

int ThreadedVideoPlayerManager::getFreePlayerFromIndex(int playerIndex){
    int nextIndex = -1;
    for (int i=playerIndex; i<players.size(); i++){
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



bool ThreadedVideoPlayerManager::idIsInToPlay(string videoID){
    return std::find(toPlay.begin(), toPlay.end(), videoID) != toPlay.end();
}

void ThreadedVideoPlayerManager::emptyOldVideos(vector<string> toPlay){
    int index= 0;
    for (vector<player>::iterator it = players.begin(); it!=players.end();){
        index++;
        //If it's playing or loading let it continue
        if (it->status == playing) {
            ++it;
            continue;
        }
        else if (it->status == loading) {
            ++it;
            continue;
        }
        else if (it->status == priming) {
            ++it;
            continue;
        }

        //if not in the list set it to empty
        else if (!idIsInToPlay(it->videoID)){
           it->video.close();
           it->status = empty;
           //Remove from vector if we have too many players
           if (players.size() >toPlay.size()) {
               it = players.erase(it);
               continue;
           }
        }
        ++it;
    }

}

void ThreadedVideoPlayerManager::_playNextVideo(){
    int nextIndex = getNextPlayerFromIndex(playingVideoIndex);
    if (nextIndex == -1){
        ofLogError() << "Couldn't find a ready player";
        if (playingVideoPointer->status==playing)
        {
            playingVideoPointer->video.setFrame(1);
            samplePlayer.playFile(playingVideoPointer->videoID, 1);
        }
        return;
    }
    playingVideoIndex =nextIndex;
    PLAYING=true;
    if(playingVideoPointer->status ==playing){
       playingVideoPointer->status=played;
       playingVideoPointer->video.setFrame(1);
       playingVideoPointer->video.setPaused(true);

    }

    lastVideoPointer = playingVideoPointer;

    playingVideoPointer= &players[playingVideoIndex];

    int frame = playingVideoPointer->video.getCurrentFrame();

    samplePlayer.playFile(playingVideoPointer->videoID, frame);

    playingVideoPointer->video.setVolume(0.0f);
    playingVideoPointer->video.setPaused(false);
    playingVideoPointer->status = playing;

    std::unique_lock<std::mutex> lock(*playing_mutex, std::try_to_lock);
    if(!lock.owns_lock()){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Couldn't update playing video";
        return;
    }
    playing_queue->push_front( playingVideoPointer->videoID);
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
        if (i>= players.size()){
            player p;
            p.status = empty;
            players.push_back(p);
        }
        receiveVideo(toPlay[i]);
    }
}


void ThreadedVideoPlayerManager::update(){
    emptyOldVideos(toPlay);

    if (ofGetElapsedTimeMillis() - switch_timer > switch_ms) {
        _playNextVideo();
        switch_timer = ofGetElapsedTimeMillis();
    }
//    if (LOADING){
//        if (players[loadingPlayerIndex].video.isLoaded()){
//            //LOADING =false;
//        }
//    }

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
            if (!loadVideo(next_command.path) ){
                if (lock()) {
                    queue.push_back(next_command);
                    unlock();

                }
            }
        }
    }

    for (int i=0; i<players.size(); i++){
        //Check if loading videos are ready to be started
        if (players[i].status==loading){
            players[i].video.update();
            LOADING_TIMER +=  ofGetElapsedTimeMillis()- players[i].loadTime;
            if (LOADING_TIMER >100){
                ofLogError()<<"Fatal error loading video";
                players[i].video.close();
                players[i].status = empty;
                LOADING =false;
            }
            if(players[i].video.isLoaded()) {
                players[i].video.setVolume(0.0f);
                players[i].video.setPaused(false);
                players[i].video.update();

                players[i].status =priming;
                players[i].loadTime = ofGetElapsedTimeMillis() - players[i].loadTime;
                players[i].primeTime = ofGetElapsedTimeMillis();

            }
        }
        //Check if priming videos have loaded their first frame
        else if (players[i].status==priming){
            if (!players[i].video.isPlaying()){
                players[i].video.setPaused(false);
                ofLogError() <<"Shouldn't be here";
            }

            players[i].video.update();
            if(players[i].video.isPlaying() && players[i].video.getCurrentFrame() >= 1){
                players[i].video.setPaused(true);
                players[i].status =ready;
                //players[i].loadTime = ofGetElapsedTimeMillis() - players[i].loadTime;
                players[i].primeTime = ofGetElapsedTimeMillis()- players[i].primeTime;
                LOADING =false;
            }
        }
    }

    if (players[playingVideoIndex].status==playing){
        players[playingVideoIndex].video.update();
    }
}
bool ThreadedVideoPlayerManager::draw(int x, int y){
    if (players.size()==0){
        return false;
    }
    if (!(playingVideoPointer->status ==playing)){
        return false;
    }
    static bool isDrawing = false;
    int current_pos = playingVideoPointer->video.getCurrentFrame();
    int total_pos = playingVideoPointer->video.getTotalNumFrames();
    int volumeSteps =1;
    float volume = playingVideoPointer->maxVol *current_pos/float(volumeSteps);
    ofPushStyle();


    playingVideoPointer->video.draw(x, y);
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
    os << "Number of players"  << players.size()<< endl;
    os << "ToPlay size"  << toPlay.size()<< endl;

    os << "Queue size"  << queue.size()<< endl;

    os << "Current video"  << endl;
    //os << "Index    : " << playingVideoIndex << endl;
    os << "Load    : " << playingVideoPointer->loadTime << endl;
    os << "Prime    : " << playingVideoPointer->primeTime << endl;

    os << "Frame   : " << current_pos << "/" << total_pos << endl;
    os << "ID      : " <<  playingVideoPointer->videoID <<endl;
    os << "State   : " << state_string[playingVideoPointer->status ]<< endl << "------------------" << endl;


//    os << "Last video"  << endl;
//    //os << "Index    : " << lastVideoIndex << endl;
//    os << "ID      : " <<  getFileName(lastVideoPointer->videoID)<< endl;
//    os << "Load    : " << lastVideoPointer->loadTime << endl;
//    os << "Prime    : " << lastVideoPointer->primeTime << endl;

//    os << "Frame   : " << lastVideoPointer->video.getCurrentFrame() << "/" << lastVideoPointer->video.getTotalNumFrames() << endl;
//    os << "Playing : " <<lastVideoPointer->video.isPlaying() << endl;
//    os << "State   : " << state_string[ lastVideoPointer->status] << endl << "------------------" << endl;

    for (int i=0; i<players.size(); i++){
        os << "Player  :"  << i<<endl;
        os << "ID      : " <<  getFileName(players[i].videoID) << endl ;
        os << "State   : " << state_string[ players[i].status] << endl;
        os << "Load    : " << players[i].loadTime << endl;
        os << "Prime    : " << players[i].primeTime << endl<< "------------------" << endl;
    }


    ofDrawBitmapString(os.str(), w-w/4+2, 17);
    ofPopStyle();

    return isDrawing;

}

void ThreadedVideoPlayerManager::audioOut(ofSoundBuffer& buffer){
    if (PLAYING){
        samplePlayer.audioOut(buffer);
    }

}

























/************************* DUMP ********************/


//vector<player>::iterator ThreadedVideoPlayerManager::getNextPlayerFromIterator(vector<player>::iterator it, bool second_pass){
//    vector<player>::iterator next= NULL;
//    for (; it!=players.end(); it++){
//        if (it== playingVideoPointer && it->status ==playing) continue;
//        if (it->status == loading) continue;
//        if (it->status == ready){
//            next = it;
//            break;
//        }
//        if (it->status == played and it->video.isPaused()){
//            next = it;
//            break;
//        }
//    }
//    if (next == NULL && second_pass ==False){
//        return getNextPlayerFromIndex(players.begin(), True);
//    }
//    return next;
//}



//vector<player>::iterator ThreadedVideoPlayerManager::getFreePlayerFromIterator(vector<player>::iterator it, bool second_pass){
//    vector<player>::iterator next = NULL;
//    for (; it!=players.end(); it++){
//        if (it == playingVideoPointer) continue;
//        if (it->status == loading) continue;
//        if (it->status == empty) {
//            next = i;
//            break;
//        }
//        if (it->status == played and it->video.isPaused()) {
//            next = i;
//            break;
//        }
//    }

//    if (next == NULL && !second_pass){
//        return getFreePlayerFromIterator(players.begin(), True);
//    }
//    return next;
//}
