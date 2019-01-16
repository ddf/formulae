#include "ofApp.h"

const size_t kOutputChannels = 2;
const size_t kSampleRate = 44100;
const size_t kBufferSize = 512;

//--------------------------------------------------------------
void ofApp::setup()
{
	//ofSetFullscreen(true);
	ofSetFrameRate(60);

	mOutput.setNumChannels(kOutputChannels);
	mOutput.setSampleRate(kSampleRate);
	mOutput.resize(ofGetWidth());
	mOutputBegin = 0;
	
	mSoundSettings.numOutputChannels = kOutputChannels;
	mSoundSettings.sampleRate = kSampleRate;
	mSoundSettings.bufferSize = kBufferSize;
	mSoundSettings.setApi(ofSoundDevice::MS_DS);
	mSoundSettings.setOutListener(this);
	ofSoundStreamSetup(mSoundSettings);

	Program::CompileError error;
	int errorPosition;
	mProgram = Program::Compile(
	   "c = 16 * (1 + (q/1024)%8) + (q/(256 - 16*V0))%16; \
		r = q / c; \
		a = t / (32 - V4); \
		s = $(q / 128) * (a << (r % 3 + 1) & a << (r % 5 + 1)); \
		z = (s << 3) ^ (s << 5) ^ (s << 7) ^ (s << 11) ^ (s << 13) ^ (s << 17) ^ (s << 19) ^ (s << 23); \
		z = q < 128 * 4 | V4 < 1 ? 0 : z; \
		j = j + (64 - 4 * (q % 16)); \
		k = $(j) << 4 >> (q*V3); k = k * (V3>0); \
		s = ((q / 64) % 2) * ($(t * 320) | R(w / 2)) << 6 >> (q*V2); \
		s = s * (V2 > 0); \
		b = V1; \
		h = ((w / 4) + R(w / 2)) >> (q*(c < 112 ? b : b-1)); \
		h = h * (V1 > 0); \
		d = (k | s | h) * (q / 1024 + 1); \
		z = z >> d * d; \
		d = d >> c / 32; \
		[0] = d | (w / 2 + z); \
		[1] = d | (w / 2 - z); ", 
		512, error, errorPosition);
	mTick = 0;
	mTempo = 60;
	mBitDepth = 15;

	// c
	V0.setMin(0);
	V0.setMax(15);
	V0 = 8;

	// h
	V1.setMin(0);
	V1.setMax(32);
	V1 = 4;

	// s
	V2.setMin(0);
	V2.setMax(32);
	V2 = 1;

	// k
	V3.setMin(0);
	V3.setMax(8);
	V3 = 1;

	// z
	V4.setMin(0);
	V4.setMax(31);
	V4 = 31;

	mGUI.setup("prime_glitch");
	mGUI.add(V0);
	mGUI.add(V1);
	mGUI.add(V2);
	mGUI.add(V3);
	mGUI.add(V4);
	mGUI.add(V5);

	mVars.push_back(new VarViz('c', 0.1f, 0.1f, 0.1, 0.1f, kVizTypeBlocks, 1, 1, 16 * 8));	
	mVars.push_back(new VarViz('h', 0.1f, 0.25f, 0.1f, 0.1f, kVizTypeScatter, kSampleRate / mTempo));
	mVars.push_back(new VarViz('s', 0.1f, 0.4, 0.1f, 0.1f, kVizTypeBars, 1024 * 8));
	mVars.push_back(new VarViz('k', 0.1f, 0.55f, 0.1f, 0.1f, kVizTypeBars, 1024 * 8));

	mVars.push_back(new VarViz('z', 0.25f, 0.1f, 0.55f, 0.55f, kVizTypeBars, 1024 / 8, 1, 1<<mBitDepth));
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
	//int w = ofGetWidth();
	//int sz = mOutput.size();
	//for (int i = 0; i < w; i += 2)
	//{
	//	int f = (mOutputBegin + i) % sz;
	//	left.addVertex(i, leftCenter + waveHeight * mOutput[f]);
	//	right.addVertex(i, rightCenter + waveHeight * mOutput[f + 1]);
	//}
	for (size_t i = 0; i < mVars.size(); ++i)
	{
		mVars[i]->draw();
	}
	mMutex.unlock();

	ofSetColor(200);
	left.draw();
	right.draw();

	mGUI.draw();
}

void ofApp::exit()
{
	mMutex.lock();
	ofSoundStreamStop();
	delete mProgram;
	mProgram = nullptr;
	for (int i = 0; i < mVars.size(); ++i)
	{
		delete mVars[i];
	}
	mVars.clear();
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
			mProgram->SetVC(0, V0);
			mProgram->SetVC(1, V1);
			mProgram->SetVC(2, V2);
			mProgram->SetVC(3, V3);
			mProgram->SetVC(4, V4);
			mProgram->SetVC(5, V5);
			mProgram->SetVC(6, V6);
			mProgram->SetVC(7, V7);
			results[0] = 0;
			results[1] = 0;
			error = mProgram->Run(results, 2);
			output[f] = -1.0f + 2.0f*((float)(results[0] % range) / (range - 1));
			output[f + 1] = -1.0f + 2.0f*((float)(results[1] % range) / (range - 1));
			for (size_t i = 0; i < mVars.size(); ++i)
			{
				auto  var = mVars[i];
				var->push(mProgram->Get(var->Var));

			}
			++mTick;
		}
		
		mOutputBegin = (mTick*nChannels - bufferSize) % mOutput.size();
		const size_t outSize = mOutput.size();
		for (size_t f = 0; f < bufferSize; f += nChannels)
		{
			size_t i = (mOutputBegin + f) % outSize;
			mOutput[i] = output[f];
			mOutput[i + 1] = output[f+1];
		}	
	}
	mMutex.unlock();
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
