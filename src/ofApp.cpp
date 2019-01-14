#include "ofApp.h"

const size_t kOutputChannels = 2;
const size_t kSampleRate = 44100;
const size_t kBufferSize = 512;

//--------------------------------------------------------------
void ofApp::setup()
{
	//ofSetFullscreen(true);

	mOutput.setNumChannels(kOutputChannels);
	mOutput.setSampleRate(kSampleRate);
	mOutput.resize(ofGetWidth());
	mOutputBegin = 0;
	
	mSoundSettings.numOutputChannels = kOutputChannels;
	mSoundSettings.sampleRate = kSampleRate;
	mSoundSettings.bufferSize = kBufferSize;
	mSoundSettings.setApi(ofSoundDevice::MS_WASAPI);
	mSoundSettings.setOutListener(this);
	ofSoundStreamSetup(mSoundSettings);

	Program::CompileError error;
	int errorPosition;
	mProgram = Program::Compile("[*] = t*(42&t>>10);", 512, error, errorPosition);
	mTick = 0;
	mTempo = 120;
	mBitDepth = 8;
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackground(0);
	
	ofPolyline left;
	ofPolyline right;
	const float leftCenter = ofGetHeight() * 0.75f;
	const float rightCenter = ofGetHeight() * 0.25f;
	const float waveHeight = ofGetHeight() * 0.25f;

	mMutex.lock();
	int w = ofGetWidth();
	int sz = mOutput.size();
	for (int i = 0; i < w; i += 2)
	{
		int f = (mOutputBegin + i) % sz;
		left.addVertex(i, leftCenter + waveHeight * mOutput[f]);
		right.addVertex(i, rightCenter + waveHeight * mOutput[f + 1]);
	}
	mMutex.unlock();

	ofSetColor(200);
	left.draw();
	right.draw();
}

void ofApp::exit()
{
	mMutex.lock();
	ofSoundStreamStop();
	delete mProgram;
	mProgram = nullptr;
	mMutex.unlock();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

void ofApp::audioOut(ofSoundBuffer& output)
{
	mMutex.lock();
	if (mProgram != nullptr)
	{		
		const Program::Value range = (Program::Value)1 << mBitDepth;
		const float mdenom = mSoundSettings.sampleRate / 1000.0f;
		const float qdenom = (mSoundSettings.sampleRate / (mTempo / 60.0f)) / 128.0f;

		mProgram->Set('w', range);
		mProgram->Set('~', (Program::Value)mSoundSettings.sampleRate);

		Program::Value results[kOutputChannels];
		Program::RuntimeError error;
		const size_t bufferSize = output.size();
		const size_t nChannels = output.getNumChannels();
		for (size_t f = 0; f < bufferSize; f += nChannels)
		{
			mProgram->Set('t', mTick);
			mProgram->Set('m', (Program::Value)round(mTick / mdenom));
			mProgram->Set('q', (Program::Value)round(mTick / qdenom));
			results[0] = 0;
			results[1] = 0;
			error = mProgram->Run(results, 2);
			output[f] = -1.0f + 2.0f*((float)(results[0] % range) / (range - 1));
			output[f + 1] = -1.0f + 2.0f*((float)(results[1] % range) / (range - 1));
			++mTick;
		}
		
		const size_t startAt = (mTick*nChannels - bufferSize) % mOutput.size();
		const size_t outSize = mOutput.size();
		for (size_t f = 0; f < bufferSize; f += nChannels)
		{
			size_t i = (startAt + f) % outSize;
			mOutput[i] = output[f];
			mOutput[i + 1] = output[f+1];
		}
		mOutputBegin = (mOutputBegin + bufferSize) % outSize;		
	}
	mMutex.unlock();
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
