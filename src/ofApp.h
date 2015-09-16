#pragma once

#include "ofMain.h"
#include "stdlib.h"
#include "string.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void newTemplate();
		void clearTemplates();

		float calcAverage(int k, int z);
		float calcMax(int k, int *x, int *y);

        ofxCvColorImage doHarm(int k);

		ofVideoGrabber vidGrabber;

		float w;
		float h;

		ofFbo newTmpl;
		ofFbo newTmplCircle;
		ofFbo newTmplRect;

		bool flagNewTmpl;

		int newTmplX;
		int newTmplY;

		int newTmplXX;
		int newTmplYY;

        const static int N = 10;
        const static int num = 2;
        const static int N1 = N * num;
		int n;

    	ofShader shader;

        float ww;
		float hh;

		bool stopflag;

		ofFbo fbo;

		float coeff;
		float *coeffs;

		ofFbo *fboList;
		ofFbo *corrList;
		ofFbo *rectList;
		ofFbo *corrRectList;
    	ofImage *imageList;

    	int *wList;
    	int *hList;

    	float **average;
    	float *maxValue;

    	bool keyFlag;

    	ofFbo vidRect;

    	ofFbo tmplRect;
};
