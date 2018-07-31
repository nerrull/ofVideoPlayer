#include "ofMain.h"
#include "ofApp.h"
#include "ofxJsonSettings.h"

//========================================================================
int main( ){
    ofGLFWWindowSettings settings;
    Settings::get().load("settings.json");

    settings.setSize(1920,1080);
    settings.monitor =Settings::getInt("monitor_index");
    settings.windowMode = Settings::getBool("windowed") ? OF_WINDOW :OF_FULLSCREEN;

    ofCreateWindow(settings);
//    ofSetupOpenGL(1920,1080,OF_FULLSCREEN);			// <-------- setup the GL context
    // this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
