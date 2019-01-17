#include "VarViz.h"

#include "ofGraphics.h"
#include "ofPolyline.h"
#include "ofMesh.h"

const Program::Value kMin = 1;

VarViz* VarViz::setup(const char varName, float width, float height, const VarVizType vizType, const size_t bufferSize, const size_t sampleRate, const Program::Value range)
{
	mVar = varName;
	mVizType = vizType;
	mHead = 0;
	mSize = bufferSize;
	mSampleRate = sampleRate;
	mRange = range;
	mCount = 1;
	mMax = std::max(kMin, range);

	b.width = width;
	b.height = height;
	setNeedsRedraw();

	mBuffer.resize(mSize);

	return this;
}


VarViz::~VarViz()
{
}

void VarViz::push(Program::Value value)
{
	--mCount;
	if (mCount == 0)
	{		
		mBuffer[mHead] = mRange ? value % mRange : value;
		mMax = std::max(mBuffer[mHead], mMax);
		mHead = (mHead + 1) % mSize;
		mCount = mSampleRate;

		setNeedsRedraw();
	}
}

ofAbstractParameter & VarViz::getParameter()
{
	return mVar;
}

void VarViz::generateDraw()
{
	mBack.clear();

	if (mVizType != kVizTypeBlocks)
	{
		mBack.setFillColor(thisBackgroundColor);
		mBack.setFilled(true);
		mBack.rectangle(b);
	}

	mViz.clear();
	mViz.setColor(thisFillColor);
	mViz.setFilled(false);

	ofPoint pos = getPosition();
	float h = b.getHeight();
	float w = b.getWidth();

	switch (mVizType)
	{
		case kVizTypeWave:
		{
			mViz.moveTo(pos);
			
			for (int i = 0; i < mSize; ++i)
			{
				int s = (mHead + i) % mSize;
				float vx = pos.x + ofMap(i, 0, mSize - 1, w, 0);
				float vy = pos.y + h - h * ((float)mBuffer[s] / mMax);
				mViz.lineTo(vx, vy);
			}
		}
		break;

		case kVizTypeBars:
		{
			const float cy = pos.y + h * 0.5f;
			for (int i = 0; i < mSize; ++i)
			{
				int s = (mHead + i) % mSize;
				float bx = pos.x + ofMap(i, 0, mSize - 1, w, 0);
				float bh = h * ((float)mBuffer[s] / mMax) * 0.5f;
				mViz.moveTo(bx, cy + bh);
				mViz.lineTo(bx, cy - bh);
			}
		}
		break;

		case kVizTypeScatter:
		{
			for (int i = 0; i < mSize; ++i)
			{
				int sz = w * h;
				int s = ((float)mBuffer[(mHead + i) % mSize] / mMax) * sz;
				int sx = pos.x + s % (int)w;
				int sy = pos.y + h - s / (int)w;
				mViz.rectangle(sx, sy, 1, 1);
			}
		}
		break;

		case kVizTypeBlocks:
		{
			float step = w / 16.0f;
			float radius = step * 0.85f;
			pos.x += radius / 2;
			pos.y += radius / 2;

			for (int i = 0; i < mRange; ++i)
			{
				float cx = pos.x + (i % 16) * step;
				float cy = pos.y + (i / 16) * step;
				mBack.rectangle(cx - radius, cy - radius, radius * 2, radius * 2);
			}

			mViz.setFilled(true);
			for (int i = 0; i < mSize; ++i)
			{
				auto s = mBuffer[(mHead + i) % mSize];
				float cx = pos.x + (s % 16) * step;
				float cy = pos.y + (s / 16) * step;
				mViz.rectangle(cx - radius, cy - radius, radius * 2, radius * 2);
			}
		}
		break;

		default:
			break;

	}
}

void VarViz::render()
{
	mBack.draw();
	mViz.draw();
}