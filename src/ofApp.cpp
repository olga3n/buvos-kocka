#include "ofApp.h"

#define STRINGIFY(A) #A

//--------------------------------------------------------------
void ofApp::setup(){

	w = 640;
	h = 480;

	vidGrabber.setDeviceID(1);
	vidGrabber.setVerbose(true);
	vidGrabber.initGrabber(w, h);

	newTmpl.allocate(w, h);

	newTmplRect.allocate(w, h);

	newTmplRect.begin();
	ofClear(0, 0, 0, 0);
	newTmplRect.end();

	flagNewTmpl = false;

	newTmplX = -1;
	newTmplY = -1;
	newTmplXX = -1;
	newTmplYY = -1;

	n = 0;

	coeff = 0.5;

	coeffs = new float[num];

	for (int i = 0; i < num; ++i)
	{
		coeffs[i] = coeff * (0.6 + i * 0.4);
	}
	
	ww = w * coeff;
	hh = h * coeff;

	fbo.allocate(ww, hh);

	vidRect.allocate(w, h);

	vidRect.begin();
	ofClear(0, 0, 0, 0);
	vidRect.end();

	//////////////////////////////////////////////////////////////////////

	string shaderProgram = STRINGIFY(

		uniform sampler2DRect map;
		uniform sampler2DRect tmpl;

		uniform float mat_y0;
		uniform float mat_y1;
		uniform float mat_y2;
									 
		uniform float w;
		uniform float h;

		uniform float ii;
		uniform float ij;

		void main (void){
			
            vec2 pos = gl_TexCoord[0].st;

			float mat_x0 = 0.0;
			float mat_x1 = 0.0;
			float mat_x2 = 0.0;
			
			int count = 0;
			
            for (int i = 0; i < int(w); i += int(ii) )
			{
				for (int j = 0; j < int(h); j+= int(ij) )
				{
					vec2 pos_rel = vec2(float(i), float(j));

					vec3 src_x = texture2DRect(map, vec2(pos + pos_rel)).rgb;
					vec4 src_y = texture2DRect(tmpl, pos_rel).rgba;

					mat_x0 += src_x[0];
					mat_x1 += src_x[1];
					mat_x2 += src_x[2];
							
					count ++;
				}
			}

			mat_x0 = mat_x0 / float(count);
			mat_x1 = mat_x1 / float(count);
			mat_x2 = mat_x2 / float(count);

			float c0 = 0.0;
			float c1 = 0.0;
			float c2 = 0.0;

			for (int i = 0; i < int(w); i += int(ii) )
			{
				for (int j = 0; j < int(h); j += int(ij) )
				{
					vec2 pos_rel = vec2(float(i), float(j));

					vec3 src_x = texture2DRect(map, vec2(pos + pos_rel)).rgb;
					vec4 src_y = texture2DRect(tmpl, pos_rel).rgba;
					
					float tmp_x0 = src_x[0] - mat_x0;
					float tmp_y0 = src_y[0] - mat_y0 / 255.0;
							
					float tmp_x1 = src_x[1] - mat_x1;
					float tmp_y1 = src_y[1] - mat_y1 / 255.0;
							
					float tmp_x2 = src_x[2] - mat_x2;
					float tmp_y2 = src_y[2] - mat_y2 / 255.0;
							
					c0 += tmp_x0 * tmp_y0 + tmp_x1 * tmp_y1 + tmp_x2 * tmp_y2;
					c1 += tmp_x0 * tmp_x0 + tmp_x1 * tmp_x1 + tmp_x2 * tmp_x2;
					c2 += tmp_y0 * tmp_y0 + tmp_y1 * tmp_y1 + tmp_y2 * tmp_y2;
				}

			}

			float corr = abs(c0) / sqrt(c1 * c2);

			gl_FragColor = vec4(corr, corr, corr, 1);
		}
	);

	shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgram);
	shader.linkProgram();

	//////////////////////////////////////////////////////////////////////

	//  load templates

	wList = new int[N1];
	hList = new int[N1];

	imageList = new ofImage[N];
	fboList = new ofFbo[N1];
	corrList = new ofFbo[N1];
	rectList = new ofFbo[N1];
	corrRectList = new ofFbo[N1];

	average = new float*[N1];
	maxValue = new float[N1];

	keyFlag = false;
	stopflag = false;
}

//--------------------------------------------------------------
void ofApp::update(){
	if( ! stopflag )
	{
		vidGrabber.update();

		fbo.begin();

		ofPushMatrix();
		ofScale(coeff, coeff);
		vidGrabber.draw(0, 0);
		ofPopMatrix();

		fbo.end();

		float maxV = 0;
		int maxI = -1;
		int mX = -1;
		int mY = -1;

		for(int i = 0; i < n; ++i)
		{
			for (int j = 0; j < num; j++)
			{
				int k = i + N * j;
				
				corrList[k].begin();

				ofClear(0, 0, 0, 255);

				shader.begin();

				shader.setUniformTexture("tmpl", 
                    fboList[k].getTextureReference(), 1 );

				shader.setUniform1f("mat_y0", average[k][0]);
				shader.setUniform1f("mat_y1", average[k][1]);
				shader.setUniform1f("mat_y2", average[k][2]);
				shader.setUniform1f("w", float(wList[k]));
				shader.setUniform1f("h", float(hList[k]));

				shader.setUniform1f("ii", 2.0);
				shader.setUniform1f("ij", 2.0);

				fbo.draw(0, 0);

				shader.end();

				corrList[k].end();

				int x = -1; int y = -1;

				maxValue[k] = calcMax(k, &x, &y);

				if(maxValue[k] >= maxV)
				{
					maxI = k;
					maxV = maxValue[k];
					mX = x;
					mY = y;
				}

				rectList[k].begin();
				ofClear(0, 0, 0, 0);
				rectList[k].end();

				corrRectList[k].begin();
				ofClear(0, 0, 0, 0);
				corrRectList[k].end();
			}
		}

		if (maxI > -1)
		{
			vidRect.begin();
			ofClear(0, 0, 0, 0);
			ofSetColor(0, 255, 0);
			ofNoFill();
			ofSetLineWidth(5.0);
			ofRect(mX / coeff, mY / coeff, 
                wList[maxI] / coeff, hList[maxI] / coeff);
			vidRect.end();

			rectList[maxI].begin();
			
            ofClear(0, 0, 0, 0);
			ofSetColor(0, 255, 0);
			ofNoFill();
			ofSetLineWidth(5.0);
			ofRect(10, 10, wList[maxI] / coeffs[maxI/N] - 20, hList[maxI] / coeffs[maxI/N] - 20 );
			
            rectList[maxI].end();

			corrRectList[maxI].begin();
			ofClear(0, 0, 0, 0);
			ofSetColor(0, 255, 0);
			ofNoFill();
			ofRect(mX, mY, wList[maxI], hList[maxI]);
			corrRectList[maxI].end();

			ofSetColor(255, 255, 255);
		}

	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	vidGrabber.draw(10, 40);
	newTmplRect.draw(10, 40);

	vidRect.draw(10, 40);

	int ilw = 660, ilwm = 0;
	int ilh = 20, ilh0 = 20;

	for (int i = 0; i < n; ++i)
	{
		if (ilh > 660)
		{
			ilh = ilh0;
			ilw += ilwm;
			ilwm = 0;
		}
		
		imageList[i].draw(ilw, ilh);
		
		for (int j = 0; j < num; j++)
		{
			int k = i + N * j;
			rectList[k].draw(ilw, ilh);
		}
		
		ilh += imageList[i].getHeight();
		
		if (imageList[i].getWidth() > ilwm)
		{
			ilwm = imageList[i].getWidth();
		}
	}
	

	ofDrawBitmapStringHighlight("'n' - select new template; 'c' - clear templates; "
		+ ofToString(ofGetFrameRate()) + "fps", 10, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	if (key == 'n')
	{
		flagNewTmpl = true;
		newTmplX = -1;
		newTmplY = -1;
		newTmplXX = -1;
		newTmplYY = -1;

		newTmplRect.begin();
		ofClear(0, 0, 0, 0);
		newTmplRect.end();
	}
	else if (key == 'c')
	{
		clearTemplates();
	}

	else
	{
		if(keyFlag)
		{
			keyFlag = false;
			stopflag = false;
		}
		else
		{
			keyFlag = true;
			stopflag = true;
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	
	if (flagNewTmpl && newTmplX >= 0 && newTmplY >= 0 &&
		x > 10 && x < w + 10 && y > 40 && y < h + 40 &&
		newTmplXX == -1 )
	{
		newTmplRect.begin();
		ofNoFill();
		ofClear(0, 0, 0, 0);
		ofSetColor(0, 255, 0);
		ofSetLineWidth(5.0);
		ofRect(newTmplX, newTmplY, (x - 10) - newTmplX, (y - 40) - newTmplY);
		newTmplRect.end();

		ofSetColor(255, 255, 255);
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	
	if (flagNewTmpl && x > 10 && x < 10 + w && y > 40 && y <  40 + h)
	{
		ofSetColor(255, 255, 255);

		if (newTmplX == -1 || newTmplY == -1)
		{
			newTmplX = x - 10;
			newTmplY = y - 40;
		}
		else
		{
			newTmplXX = x - 10;
			newTmplYY = y - 40;

			newTemplate();
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

//--------------------------------------------------------------
void ofApp::newTemplate(){
	
    int neww = newTmplX > newTmplXX ? 
        newTmplX - newTmplXX : newTmplXX - newTmplX;
	int newh = newTmplY > newTmplYY ? 
        newTmplY - newTmplYY : newTmplYY - newTmplY;
	int minx = newTmplX < newTmplXX ? newTmplX : newTmplXX;
	int miny = newTmplY < newTmplYY ? newTmplY : newTmplYY;

	newTmpl.allocate(neww, newh);
	
    newTmpl.begin();
	ofPushMatrix();
	ofTranslate(-minx, -miny, 0);
	vidGrabber.draw(0, 0);
	ofPopMatrix();

	newTmpl.end();

	ofPixels pix;
	newTmpl.readToPixels(pix);

	ofImage newTmplImage;
	newTmplImage.setFromPixels(pix);

	char name[50] = "";
	sprintf(name, "tmp%d.png", n);
	newTmplImage.saveImage(name);

	////////////////////////////////////////

	imageList[n].loadImage( name );

	for (int j = 0; j < num; j++)
	{
		int k = n + N * j;
		
		wList[k] = imageList[n].getWidth() * coeffs[j];
		hList[k] = imageList[n].getHeight() * coeffs[j];

		printf("num %i w %i h %i\n", k, wList[k], hList[k]);
		
		fboList[k].allocate(wList[k], hList[k]);

		fboList[k].begin();

		ofPushMatrix();
		ofScale(coeffs[j], coeffs[j]);
		imageList[n].draw(0, 0);
		ofPopMatrix();
		
		fboList[k].end();

		corrList[k].allocate(ww, hh);
		corrList[k].begin();
		ofClear(0, 0, 0, 255);
		corrList[k].end();

		average[k] = new float[3];
		for (int z = 0; z < 3; z++)
        {
			average[k][z] = calcAverage(k,z);
		}
		
		maxValue[k] = 0;

		rectList[k].allocate(imageList[n].getWidth(), 
            imageList[n].getHeight());
		rectList[k].begin();
		ofClear(0, 0, 0, 0);
		rectList[k].end();

		corrRectList[k].allocate(ww, hh);
		corrRectList[k].begin();
		ofClear(0, 0, 0, 0);
		corrRectList[k].end();
	}

	////////////////////////////////////////

	n++;

	flagNewTmpl = false;

	newTmplX = -1;
	newTmplY = -1;
	newTmplXX = -1;
	newTmplYY = -1;

	newTmplRect.begin();
	ofClear(0, 0, 0, 0);
	newTmplRect.end();
}

//--------------------------------------------------------------
void ofApp::clearTemplates(){
	for(int i=0; i<n; i++)
	{
		char name[50] = "";
		sprintf(name, "data/tmp%d.png", i);
		if (remove(name) != 0 )
		{
			printf("Oops: %s;\n", name);
		}
	}
	n = 0;
}

//--------------------------------------------------------------
float ofApp::calcAverage(int k, int z){
	float mat = 0.0;

	ofxCvColorImage cvimg = doHarm(k);

	unsigned char *pixels0 = cvimg.getPixels();
	int count = 0;

	for (int i = 0; i < wList[k]; i++)
		for (int j = 0; j < hList[k]; j++)
		{
			int ind = j * wList[k] * 4 + i * 4;
			mat += float(pixels0[ind + z]);
			count++;
		}

	mat /= float(count);
	return mat;
}

//--------------------------------------------------------------
float ofApp::calcMax(int k, int *x, int *y){
	float max = 0;

	ofxCvColorImage cvimg = doHarm(k);

	unsigned char *pixels0 = cvimg.getPixels();

	for (int i = 0; i < ww - wList[k]; i++)
		for (int j = 0; j < hh - hList[k]; j++)
		{
			int ind = j * ww * 4 + i * 4;
			if( float(pixels0[ind]) >= max)
			{
				max = float(pixels0[ind]);

				*x = i;
				*y = j;
			}
		}

	return max;
}

//--------------------------------------------------------------
ofxCvColorImage ofApp::doHarm(int k){
	ofPixels pix;
	
	corrList[k].readToPixels(pix);
	pix.setNumChannels(4);
	ofxCvColorImage cvimg;
	cvimg.allocate(ww, hh);
	cvimg.setFromPixels(pix);
	
	return cvimg;
}
