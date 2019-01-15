#include "VarViz.h"

#include "ofGraphics.h"
#include "ofPolyline.h"
#include "ofMesh.h"

VarViz::VarViz(const char varName, float x, float y, float w, float h, const VarVizType vizType, const size_t bufferSize, const size_t sampleRate, const Program::Value range )
	: Var(varName)
	, mVizType(vizType)
	, mX(x), mY(y)
	, mW(w), mH(h)
	, mHead(0)
	, mSize(bufferSize)
	, mSampleRate(sampleRate)
	, mRange(range)
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
		mBuffer[mHead] = mRange ? value % mRange : value;
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
	
	Program::Value max = 0;
	for (int i = 0; i < mSize; ++i)
	{
		max = std::max(max, mBuffer[i]);
	}

	switch (mVizType)
	{
		case kVizTypeWave:
		{
			ofPolyline line;
			for (int i = 0; i < mSize; ++i)
			{
				int s = (mHead + i) % mSize;
				float vx = x + ofMap(i, 0, mSize - 1, w, 0);
				float vy = y + h - h * ((float)mBuffer[s] / max);
				line.addVertex(vx, vy);
			}
			line.draw();
		}
		break;

		case kVizTypeBars:
		{
			const float cy = y + h * 0.5f;
			for (int i = 0; i < mSize; ++i)
			{
				int s = (mHead + i) % mSize;
				float bx = x + ofMap(i, 0, mSize - 1, w, 0);
				float bh = h * ((float)mBuffer[s] / max) * 0.5f;
				ofDrawLine(bx, cy + bh, bx, cy - bh);
			}
		}
		break;

		case kVizTypeScatter:
		{
			ofSetColor(255);
			ofMesh mesh;
			mesh.setMode(OF_PRIMITIVE_POINTS);
			for (int i = 0; i < mSize; ++i)
			{
				int sz = w * h;
				int s = ((float)mBuffer[(mHead + i) % mSize] / max) * sz;
				float sx = x + s % (int)w;
				float sy = y + h - s / (int)w;
				mesh.addVertex(ofVec3f(sx, sy, 0));
			}
			mesh.drawVertices();
		}
		break;
	}

}