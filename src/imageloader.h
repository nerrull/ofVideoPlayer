#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include "ofMain.h"

class ImageLoader
{
public:
    ImageLoader();
    vector<ofPixels> imagePixels;
    ofDirectory dir;
    void loadAllImages();
    ofPixels getRandomImage();

};

#endif // IMAGELOADER_H
