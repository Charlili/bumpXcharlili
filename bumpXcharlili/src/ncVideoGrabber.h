#pragma once

#include "ofMain.h"

class ncVideoGrabber : public ofNode{
public:

    void setup(int width, int height);
    void update();
    void draw(int x, int y);
    
    //-------------------------------------------

    ofVideoGrabber grabber1;
    ofVideoGrabber grabber2;
    
    ofFbo texture;
    ofFbo &getTexture();

};