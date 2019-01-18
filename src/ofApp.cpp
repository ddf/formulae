#include "ofApp.h"

const size_t kOutputChannels = 2;
const size_t kSampleRate = 44100;
const size_t kBufferSize = 512;

std::map<std::string, VarVizType> kVizType = {
	{ "wave", kVizTypeWave },
	{ "bars", kVizTypeBars },
	{ "blocks", kVizTypeBlocks },
	{ "scatter", kVizTypeScatter }
};

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

	mAppSettings.load("settings.xml");

	ofXml programSettings = mAppSettings.getChild("program");

	mCode = programSettings.getChild("code").getValue();
	mTick = 0;
	mTempo = programSettings.getAttribute("tempo").getUintValue();
	mBitDepth = programSettings.getAttribute("bits").getUintValue();

	Program::CompileError error;
	int errorPosition;
	mProgram = Program::Compile(mCode.c_str(), programSettings.getAttribute("memory").getUintValue(), error, errorPosition);

	ofXml interfaceSettings = programSettings.getChild("interface");
	const float ws = ofGetWidth() / interfaceSettings.getAttribute("w").getFloatValue();
	const float hs = ofGetHeight() / interfaceSettings.getAttribute("h").getFloatValue();

	ofxBaseGui::setDefaultBorderColor(ofColor(250));

	mGUI.setup(programSettings.getAttribute("name").getValue());

	// setup all the controls
	for (auto child : interfaceSettings.getChildren())
	{
		auto element = child.getName();
		auto name = child.getAttribute("name").getValue();		
		if (element == "control")
		{
			auto VC = mVC[child.getAttribute("target").getValue()];
			VC->setMin(child.getAttribute("min").getUintValue());
			VC->setMax(child.getAttribute("max").getUintValue());
			VC->set(name, child.getAttribute("value").getUintValue());

			mGUI.add(*VC);
		}
		else if (element == "viz")
		{
			auto source = child.getAttribute("source").getValue()[0];
			auto vizType = kVizType[child.getAttribute("type").getValue()];
			auto buffer = child.getAttribute("buffer").getUintValue();
			auto rate = std::max(child.getAttribute("rate").getUintValue(), 1U);
			auto range = child.getAttribute("range").getUintValue();
			auto viz = new VarViz(name, source, vizType, buffer, rate, range);
			mGUI.add(viz);
			mVars.push_back(viz);
		}

		ofxBaseGui* ui = mGUI.getControl(name);
		auto shape = child.getChild("shape");
		if (ui != nullptr && shape)
		{		
			float x = shape.getAttribute("x").getFloatValue();
			float y = shape.getAttribute("y").getFloatValue();
			float w = shape.getAttribute("w").getFloatValue();
			float h = shape.getAttribute("h").getFloatValue();
		
			ui->setShape(x*ws, y*hs, w*ws, h*hs);
		}
	}

	mGUI.setShape(0, 0, ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackground(0);

	mMutex.lock();
	mGUI.draw();
	mMutex.unlock();
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
				var->push(mProgram->Get(var->getVar()));

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
