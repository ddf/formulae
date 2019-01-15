#include "VarViz.h"

#include "ofGraphics.h"
#include "ofPolyline.h"

VarViz::VarViz(const char varName, const size_t bufferSize, const size_t sampleRate, float x, float y, float w, float h)
	: Var(varName)
	, mX(x), mY(y)
	, mW(w), mH(h)
	, mHead(0)
	, mSize(bufferSize)
	, mSampleRate(sampleRate)
	, mCount(1)
{
	mBuffer = new Program::Value[bufferSize];
}


VarViz::~VarViz()
{
	delete[] mBuffer;
}

void VarViz::push(Program::Value value)
{
	--mCount;
	if (mCount == 0)
	{
		mBuffer[mHead] = value;
		mHead = (mHead + 1) % mSize;
		mCount = mSampleRate;
	}
}

void VarViz::draw()
{
	const float x = mX * ofGetWidth();
	const float y = mY * ofGetHeight();
	const float w = mW * ofGetWidth();
	const float h = mH * ofGetHeight();

	ofSetColor(200);
	ofSetRectMode(OF_RECTMODE_CORNER);
	ofNoFill();
	ofDrawRectangle(x, y, w, h);
	ofPolyline line;
	Program::Value max = 0;
	for (int i = 0; i < mSize; ++i)
	{
		max = std::max(max, mBuffer[i]);
	}
	for (int i = 0; i < mSize; ++i)
	{
		int s = (mHead + i) % mSize;
		float vx = x + ofMap(i, 0, mSize, 0, w);
		float vy = y + h - h * ((float)mBuffer[s] / max);
		line.addVertex(vx, vy);
	}
	line.draw();
}