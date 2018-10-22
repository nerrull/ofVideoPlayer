#include "ofMain.h"
#include "ofApp.h"
#include "ofxJsonSettings.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
//========================================================================

int main( ){
    ofGLFWWindowSettings settings;
    Settings::get().load("settings.json");

    settings.setSize(Settings::getInt("monitor_width"), Settings::getInt("monitor_height"));
    settings.monitor =Settings::getInt("monitor_index");    settings.monitor =Settings::getInt("monitor_index");
    settings.windowMode = Settings::getBool("windowed") ? OF_WINDOW :OF_FULLSCREEN;

    ofCreateWindow(settings);
    auto app = new ofApp();

    glfwSetWindowFocusCallback(((ofAppGLFWWindow*) ofGetWindowPtr())->getGLFWWindow(), app->window_focus_callback );
    ofRunApp(app);
}
