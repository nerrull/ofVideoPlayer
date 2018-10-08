#include "imageloader.h"

ImageLoader::ImageLoader()
{


    string path = "/media/rice1902/OuterSpace1/dataStore/IMAGE/frames";
    ofSetDataPathRoot(path);
    dir= ofDirectory(path);
    dir.allowExt("png");
    dir.listDir();

}

void ImageLoader::loadAllImages(){
    vector<ofFile> files =  dir.getFiles();
    for (int i =0 ; i<files.size(); i++){
        ofPixels p;
        ofLoadImage(p,dir.getPath(i));
        imagePixels.push_back(p);
    }
}

ofPixels ImageLoader::getRandomImage(){
    int size = imagePixels.size();
    int imageIndex = ofRandom(0,size);
    return imagePixels[imageIndex];
}
