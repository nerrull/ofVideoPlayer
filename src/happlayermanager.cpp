#include "happlayermanager.h"

HapPlayerManager::HapPlayerManager(deque<string> *pq, ofMutex *pm)
{
    this->playing_queue= pq;
    this->playing_mutex = pm;
    string samplePath = "/media/rice1902/OuterSpace2/dataStore/audio_samples.h5";
    videoPath = "/media/rice1902/OuterSpace2/dataStore/VIDEO/hap/";
    string audiopath = "/media/rice1902/OuterSpace2/dataStore/AUDIO/full_audio/";

    //videoPath = "/media/rice1902/Seagate4T/hap_test/";

    //samplePlayer.loadHDF5Data(samplePath);
    samplePlayer.init(audiopath ,-1);

    playingVideoIndex = 0;
    lastVideoIndex =0;
    loadIndex =0;
    LOADING =false;
    PLAYING = false;
    OVERLAY= true;
//    for (int i=0; i<MAX_VIDEOS; i++){
//        players.push_back(new player());
//        players[i]->video.setLoopState(OF_LOOP_NORMAL);
//        players[i]->maxVol = 1.0f;
//        players[i]->status= empty;
//        players[i]->videoID = "/EMPTY";
//    }
    switch_timer = ofGetElapsedTimeMillis();
    call_time = ofGetElapsedTimeMillis();
}

// Destructor
HapPlayerManager::~HapPlayerManager() {
//todo : stop + close all videos
    for (auto p: players){
        p->video.stop();
        p->video.close();
    }
}

void HapPlayerManager::loadAllVideos(ofDirectory dir){
   int num_videos = dir.size();
   //DEBUG MODE

   for (int i = 0; i<num_videos;i++){
       string movieFile = dir.getName(i);
       size_t lastindex = movieFile.find_last_of(".");
       string rawname = movieFile.substr(0, lastindex);
       addVideoPlayer(rawname, False);
   }
}

void HapPlayerManager::setVolume(float _volume) {
    for (int i=0; i<MAX_VIDEOS; i++){
        players[i]->maxVol = _volume;
    }
}


void HapPlayerManager::setSpeed(int speed) {
    int last_ms = switch_ms;
    switch_ms = speed;
}

void HapPlayerManager::receiveVideo(string path){
    ofLogVerbose() << "Received : " << path;
    HapPlayerManager::command c;
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

string HapPlayerManager::getFileName(string s){

    std::size_t botDirPos = s.find_last_of("/");
    std::string file = s.substr(botDirPos+1, s.length());
    return file;
}

bool HapPlayerManager::alreadyLoaded(string _path){
      for (int i =0; i<MAX_VIDEOS; i++){
        if (_path.compare( players[i]->videoID)==0) return true;
      }
      return false;
}

void HapPlayerManager::threadedFunction(){
    LOADING = true;
    players[loadIndex]->video.load(players[loadIndex]->filePath);
    players[loadIndex]->loadTime = ofGetElapsedTimeMillis()-players[loadIndex]->loadTime;
    LOADING =false;
}

bool HapPlayerManager::loadVideo(string _path){
    ofLogError() << "Started loading " << _path ;
    string fullpath =videoPath +_path+".mov";
    ofLogError(ofToString(ofGetElapsedTimef(),3)) << "[Loading] " << fullpath;

    if (alreadyLoaded(_path)){
       ofLogError(ofToString(ofGetElapsedTimef(),3)) << _path << " already loaded";
       return True;
    }

    int freePlayer= getFreePlayerFromIndex(playingVideoIndex);

    if (freePlayer == -1){
        ofLogError() << "Couldn't find a free video player";
        return False;
    }

    if (LOADING){
        ofLogError() << "Already loading a video";
        return False;
    }

    ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Load call received";
    players[freePlayer]->status  = loading;

    players[freePlayer]->loadTime = ofGetElapsedTimeMillis();
    players[freePlayer]->video.setLoopState(OF_LOOP_NORMAL);
    players[freePlayer]->videoID  = _path;
    players[freePlayer]->filePath  = fullpath;
    loadIndex = freePlayer;

    this->threadedFunction();


    return True;
}

void HapPlayerManager::addVideoPlayer(string _path, bool async = True){
    string fullpath =videoPath +_path+".mov";
    ofLogError(ofToString(ofGetElapsedTimef(),3)) << "[Loading] " << fullpath;
    player * p = new player;

    p->status  = loading;

    p->loadTime = ofGetElapsedTimeMillis();
    p->video.setLoopState(OF_LOOP_NORMAL);
    p->videoID  = _path;
    p->filePath  = fullpath;

    p->video.load(fullpath);
    p->loadTime = ofGetElapsedTimeMillis()-p->loadTime;
    players.push_back(p);
    toPlayVideoIndexes.push_back(players.size()-1);
}


void HapPlayerManager::readToPlay(vector<string>toPlay){\
    string toPlayId;
    toPlayVideoIndexes.clear();
    for (int vIndex = 0; vIndex<players.size();vIndex++){
        for (int i = 0; i< toPlay.size(); i++){
            if (strcmp(toPlay[i].c_str(), players[vIndex]->videoID.c_str()) ==0){
                toPlayVideoIndexes.push_back(vIndex);
            }
        }
    }
}


int HapPlayerManager::getNextPlayerIndex(){
    if (toPlayVideoIndex < (toPlayVideoIndexes.size() -1)){
        toPlayVideoIndex++;
        return toPlayVideoIndexes[toPlayVideoIndex];

    }
    else{
        toPlayVideoIndex =0;
        //ofRandomize(toPlayVideoIndexes);
        return toPlayVideoIndexes[0];
    }
}

int HapPlayerManager::getNextPlayerFromIndex(int playerIndex){
    int nextIndex = -1;
    for (int i =playerIndex; i<MAX_VIDEOS; i++){
        if (i== playingVideoIndex && players[i]->status ==playing) continue;
        if (players[i]->status == loading) continue;
        if (players[i]->status == ready){
            nextIndex = i;
            break;
        }
        if (players[i]->status == played and players[i]->video.isPaused()){
            nextIndex = i;
            break;
        }
    }
    if (nextIndex == -1 && playerIndex!=0){
        return getNextPlayerFromIndex(0);
    }
    return nextIndex;
}

void  HapPlayerManager::setAllVolumes(float value){
    for (int i=0; i<MAX_VIDEOS; i++){
        if (players[i]->status == empty) continue;
        if (players[i]->status == priming) continue;
        players[i]->video.setVolume(value);
    }

}

int HapPlayerManager::getFreePlayerFromIndex(int playerIndex){
    int nextIndex = -1;
    for (int i=playerIndex; i<MAX_VIDEOS; i++){
        if (i == playingVideoIndex) continue;
        if (players[i]->status == loading) continue;
        if (players[i]->status == empty) {
            nextIndex = i;
            break;
        }
        if (players[i]->status == played and players[i]->video.isPaused()) {
            nextIndex = i;
            break;
        }
    }
    if (nextIndex == -1 && playerIndex!=0){
        return getFreePlayerFromIndex(0);
    }
    return nextIndex;
}


void HapPlayerManager::emptyOldVideos(vector<string> toPlay){
    for (int i=0; i<MAX_VIDEOS; i++){
        if (i == playingVideoIndex) continue;
        else if (players[i]->status == loading) continue;
        else if (players[i]->status == empty) continue;
        else if (players[i]->status == playing) continue;
        else if (players[i]->status == priming) continue;

        //if it's in the list conitnue
        else if (std::find(toPlay.begin(), toPlay.end(), players[i]->videoID) != toPlay.end())
        {
            continue;
        }
        //if not set it to empty
        else {
           players[i]->status = empty    ;
           players[i]->video.setPaused(true);
           break;
        }
    }

}



void HapPlayerManager::_playNextVideoLoaded(){
    int playerIndex =playingVideoIndex;
    int nextIndex = getNextPlayerIndex();

    PLAYING = true;
    lastVideoIndex = playingVideoIndex;

    if(!players[lastVideoIndex]->video.isPaused()){
       players[lastVideoIndex]->status=played;
       players[lastVideoIndex]->video.setPaused(true);
       players[lastVideoIndex]->video.setPosition(0);
    }
    playingVideoIndex=nextIndex;

    int frame = players[playingVideoIndex]->video.getPosition()*players[playingVideoIndex]->video.getDuration()/0.033 ;
    samplePlayer.playFile(players[playingVideoIndex]->videoID, frame);

    players[playingVideoIndex]->video.setVolume(0.0f);
    players[playingVideoIndex]->video.setPaused(false);
    players[playingVideoIndex]->status = playing;

    std::unique_lock<std::mutex> lock(*playing_mutex, std::try_to_lock);
    if(!lock.owns_lock()){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Couldn't update playing video";
        return;
    }
    playing_queue->push_front( players[playingVideoIndex]->videoID);

}

void HapPlayerManager::_playNextVideo(){
    int playerIndex =playingVideoIndex;
    int nextIndex = getNextPlayerFromIndex(playerIndex);
    if (nextIndex == -1){
        ofLogError() << "Couldn't find a ready player";
        players[playingVideoIndex]->video.setPosition(0);//setFrame(1);
        samplePlayer.playFile(players[playingVideoIndex]->videoID, 1);
        return;
    }
    PLAYING = true;

    lastVideoIndex = playingVideoIndex;

    if(!players[lastVideoIndex]->video.isPaused()){
       players[lastVideoIndex]->status=played;
       players[lastVideoIndex]->video.setPaused(true);
       players[lastVideoIndex]->video.setPosition(0);
    }
    playingVideoIndex=nextIndex;

    int frame = 1;
    samplePlayer.playFile(players[playingVideoIndex]->videoID, frame);

    players[playingVideoIndex]->video.setVolume(1.0f);
    players[playingVideoIndex]->video.setPaused(false);
    players[playingVideoIndex]->status = playing;

    std::unique_lock<std::mutex> lock(*playing_mutex, std::try_to_lock);
    if(!lock.owns_lock()){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Couldn't update playing video";
        return;
    }
    playing_queue->push_front( players[playingVideoIndex]->videoID);

}


void HapPlayerManager::setToPlay(vector<string> toPlay){
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


void HapPlayerManager::update(){
    //ofLogError()<<"Call time : " <<ofGetElapsedTimeMillis() - call_time;
    //uint64_t update_time = ofGetElapsedTimeMillis();

    samplePlayer.update();
    if (queue.size()>0 && (internal_counter++ %10)==0) {
        HapPlayerManager::command next_command;
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
        if (players[i]->status==loading){
            if(players[i]->video.isLoaded()) {
                players[i]->video.setPaused(false);
                players[i]->status =priming;
            }
        }
        //Check if priming videos have loaded their first frame
        else if (players[i]->status==priming){
            players[i]->video.setPaused(false);
            players[i]->video.update();
            //todo check
            if(players[i]->video.isPlaying() && players[i]->video.getPosition() >= 0.001){
                players[i]->video.setPaused(true);
                players[i]->status =ready;
            }
        }
    }

    if (ofGetElapsedTimeMillis() - switch_timer > switch_ms) {
        _playNextVideoLoaded();
        switch_timer = ofGetElapsedTimeMillis();
    }

//    if (players[playingVideoIndex]->status==playing){
//        players[playingVideoIndex]->video.update();
//    }
//    if (players[lastVideoIndex]->status==playing){
//        players[lastVideoIndex]->video.update();
//    }

    //update_time = ofGetElapsedTimeMillis() - update_time ;
    //ofLogError()<<"Update time : " <<update_time;

    //call_time = ofGetElapsedTimeMillis();


}
bool HapPlayerManager::draw(int x, int y){

    static bool isDrawing = false;
    int volumeSteps =1;
    ofPushStyle();

    players[playingVideoIndex]->video.draw(x, y);
    ofDisableBlendMode();
    ofPopStyle();
    isDrawing = true;

    if (!OVERLAY) return isDrawing;
    ofPushStyle();
    ofSetColor(0, 0, 0, 150);
    int w = ofGetWidth();
    int h = ofGetHeight();
    ofDrawRectangle(w-w/4-2, 0, w/4+2, h);
    ofNoFill();
    ofSetColor(255, 0, 0);


    float current_pos =  players[playingVideoIndex]->video.getPosition();
    float total_pos =  players[playingVideoIndex]->video.getDuration();

    ostringstream os;
    os << "Queue size"  << queue.size()<< endl;

    os << "Current video"  << endl;
    os << "Index    : " << playingVideoIndex << endl;
    os << "Load    : " << players[playingVideoIndex]->loadTime << endl;
    os << "Frame   : " << current_pos << "/" << total_pos << endl;
    os << "ID      : " <<  getFileName(players[playingVideoIndex]->videoID) <<endl;
    os << "State   : " << state_string[players[playingVideoIndex]->status ]<< endl << "------------------" << endl;


    os << "Last video"  << endl;
    os << "Index    : " << lastVideoIndex << endl;
    os << "ID      : " <<  getFileName(players[lastVideoIndex]->videoID)<< endl;
    os << "Load    : " << players[lastVideoIndex]->loadTime << endl;
    os << "Position   : " << players[lastVideoIndex]->video.getPosition() << "/" << players[lastVideoIndex]->video.getDuration() << endl;
    os << "Playing : " << players[lastVideoIndex]->video.isPlaying() << endl;
    os << "State   : " << state_string[ players[lastVideoIndex]->status] << endl << "------------------" << endl;

//    for (int i=0; i<MAX_VIDEOS; i++){
//        os << "Player  :"  << i<<endl;
//        os << "State   : " << state_string[ players[i]->status] << endl;
//        os << "Path    : " << players[i]->filePath << endl;
//        os << "ID      : " <<  getFileName(players[i]->videoID) <<endl;
//        os << "Position   : " << players[i]->video.getPosition() << "/" << players[i]->video.getDuration() << endl << "------------------" << endl;

//    }


    ofDrawBitmapString(os.str(), w-w/4+2, 17);
    ofPopStyle();

    return isDrawing;

}

void HapPlayerManager::audioOut(ofSoundBuffer& buffer){
    //if (PLAYING) samplePlayer.audioOut(buffer);


}

