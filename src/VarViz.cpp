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
	, mMax(1)
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
		if (mMax > 1)
		{
			--mMax;
		}

		mBuffer[mHead] = mRange ? value % mRange : value;
		mMax = std::max(mBuffer[mHead], mMax);
		mHead = (mHead + 1) % mSize;
		mCount = mSampleRate;
	}
}

void VarViz::draw()
{
	const int x = mX * ofGetWidth();
	const int y = mY * ofGetHeight();
	const int w = mW * ofGetWidth();
	const int h = mH * ofGetHeight();

	ofSetColor(200);

	ofDrawBitmapString(Var, x, y-5);

	ofSetRectMode(OF_RECTMODE_CORNER);
	ofNoFill();
	ofDrawRectangle(x, y, w, h);

	switch (mVizType)
	{
		case kVizTypeWave:
		{
			ofPolyline line;
			for (int i = 0; i < mSize; ++i)
			{
				int s = (mHead + i) % mSize;
				float vx = x + ofMap(i, 0, mSize - 1, w, 0);
				float vy = y + h - h * ((float)mBuffer[s] / mMax);
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
				float bh = h * ((float)mBuffer[s] / mMax) * 0.5f;
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
				int s = ((float)mBuffer[(mHead + i) % mSize] / mMax) * sz;
				int sx = x + s % (int)w;
				int sy = y + h - s / (int)w;
				mesh.addVertex(ofVec3f(sx, sy, 0));
			}
			mesh.drawVertices();
		}
		break;
	}

}