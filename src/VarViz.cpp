#include "VarViz.h"

#include "ofGraphics.h"
#include "ofPolyline.h"
#include "ofMesh.h"

const Program::Value kMin = 1;

VarViz* VarViz::setup(const std::string& guiName, Program::Value var, const VarVizType vizType, const size_t bufferSize, const size_t sampleRate, const Program::Value range)
{
	mVar = var;
	mVizType = vizType;
	mHead = 0;
	mSize = vizType == kVizTypeValue ? 1 : bufferSize;
	mSampleRate = sampleRate;
	mColumns = 16;
	mRange = range;
	mCount = 1;
	mMax = std::max(kMin, range);

	mBuffer.resize(mSize);

	setSize(defaultWidth, defaultHeight);
	setName(guiName);

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
		if (mVizType == kVizTypeValue)
		{
			if (mBuffer[0] != value)
			{
				setNeedsRedraw();
			}
			mBuffer[0] = value;
		}
		else
		{
			mBuffer[mHead] = mRange ? value % mRange : value;
			mMax = std::max(mBuffer[mHead], mMax);
			mHead = (mHead + 1) % mSize;			

			setNeedsRedraw();
		}

		mCount = mSampleRate;
	}
}

ofAbstractParameter & VarViz::getParameter()
{
	return mVar;
}

void VarViz::generateDraw()
{
	mBack.clear();

	mBack.setFillColor(thisBackgroundColor);
	mBack.setFilled(true);

	if (mVizType != kVizTypeBlocks)
	{		
		mBack.rectangle(b);
	}

	mViz.clear();
	mViz.setColor(thisBorderColor);
	mViz.setFilled(false);
	mViz.setStrokeWidth(1);

	mLabel.clear();

	ofPoint pos = getPosition();
	float h = b.getHeight();
	float w = b.getWidth();

	switch (mVizType)
	{
		case kVizTypeValue:
		{
			std::string name;
			if (!getName().empty()) {
				name = getName() + ": ";
			}
			name += std::to_string(mBuffer[0]);

			// resize the string inside the max width
			if (font.isLoaded()) { // using font
				while (font.stringWidth(name) > (getWidth() - textPadding * 2.0f)) {
					name.resize(name.size() - 1);
				}
			}
			else { // using bitmap font
				int max = (getWidth() - textPadding * 2.0f) / 8.0f;
				name.resize(max);
			}

			mLabel = getTextMesh(name, b.x + textPadding, b.y + b.height / 2 + 4);
		}
		break;

		case kVizTypeWave:
		{
			mViz.moveTo(b.getBottomLeft());
			
			for (int i = 0; i < mSize; ++i)
			{
				int s = (mHead + i) % mSize;
				float vx = pos.x + ofMap(i, 0, mSize-1, 0, w);
				float vy = pos.y + h - h * ((float)mBuffer[s] / mMax);
				mViz.lineTo(vx, vy);
			}
		}
		break;

		case kVizTypeBars:
		{			
			if (w >= h)
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
			else
			{
				const float cx = pos.x + w * 0.5f;
				for (int i = 0; i < mSize; ++i)
				{
					int s = (mHead + i) % mSize;
					float by = pos.y + ofMap(i, 0, mSize - 1, h, 0);
					float bw = w * ((float)mBuffer[s] / mMax) * 0.5f;
					mViz.moveTo(cx + bw, by);
					mViz.lineTo(cx - bw, by);
				}
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
			float step = w / mColumns;
			float radius = step * 0.85f * 0.5f;
			pos.x += step / 2;
			pos.y += step / 2;

			for (int i = 0; i < mRange; ++i)
			{
				float cx = pos.x + (i % mColumns) * step;
				float cy = pos.y + (i / mColumns) * step;
				mBack.rectangle(cx - radius, cy - radius, radius * 2, radius * 2);
			}

			mViz.setFilled(true);
			for (int i = 0; i < mSize; ++i)
			{
				auto s = mBuffer[(mHead + i) % mSize];
				float cx = pos.x + (s % mColumns) * step;
				float cy = pos.y + (s / mColumns) * step;
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
	ofColor c = ofGetStyle().color;

	mBack.draw();

	ofBlendMode blendMode = ofGetStyle().blendingMode;
	if (blendMode != OF_BLENDMODE_ALPHA) 
	{
		ofEnableAlphaBlending();
	}

	if (mLabel.hasVertices())
	{
		ofSetColor(thisTextColor);

		bindFontTexture();
		mLabel.draw();
		unbindFontTexture();
	}

	mViz.draw();

	ofSetColor(c);
	if (blendMode != OF_BLENDMODE_ALPHA) 
	{
		ofEnableBlendMode(blendMode);
	}	
}