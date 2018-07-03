#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    ofGLFWWindowSettings settings;

    settings.setSize(1920,1080);
    settings.monitor =1;
    settings.windowMode = OF_FULLSCREEN;
    ofCreateWindow(settings);
//    ofSetupOpenGL(1920,1080,OF_FULLSCREEN);			// <-------- setup the GL context
    // this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
