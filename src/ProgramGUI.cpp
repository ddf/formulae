#include "ProgramGUI.h"



ProgramGUI::ProgramGUI()
: ofxGuiGroup()
, mbRunning(false)
{
}


ProgramGUI::ProgramGUI(const ofParameterGroup & parameters, const std::string& _filename /*= "settings.xml"*/, float x /*= 10*/, float y /*= 10*/)
: ofxGuiGroup(parameters, filename, x, y)
, mbRunning(false)
{
}

ProgramGUI::~ProgramGUI()
{
}

void ProgramGUI::setup(const std::string& collectionName /* = "" */, const std::string& code, const std::string& filename /* = "settings.xml" */, float x /* = 10 */, float y /* = 10 */)
{
	ofxGuiGroup::setup(collectionName, filename, x, y);
	mProgramCode = code;
	spacing = 0;
	spacingNextElement = -1;
	mW = b.width;
	mH = b.height;
}

void ProgramGUI::teardown()
{
	if (!collection.empty())
	{
		for (auto ui : collection)
		{
			delete ui;
		}
	}
	collection.clear();
	unregisterMouseEvents();
}

void ProgramGUI::setSize(float w, float h)
{	
	b.width = w;
	// only change height if not maximized, otherwise the background will appear
	if (!isMinimized())
	{		
		b.height = h;
	}
	if (!collection.empty())
	{
		float rw = w / mW;
		float rh = h / mH;
		auto pos = getPosition();
		for (auto ui : collection)
		{
			ofPoint dp = ui->getPosition() - pos;
			ui->setShape(pos.x + rw*dp.x, pos.y + rh*dp.y, rw*ui->getWidth(), rh*ui->getHeight());
		}
	}
	mW = w;
	mH = h;
	sizeChangedCB();
}

void ProgramGUI::setShape(ofRectangle r)
{
	setPosition(r.x, r.y);
	setSize(r.width, r.height);
}

void ProgramGUI::setShape(float x, float y, float w, float h)
{
	setPosition(x, y);
	setSize(w, h);
}

void ProgramGUI::sizeChangedCB()
{
	if (parent)
	{
		parent->sizeChangedCB();
	}
	setNeedsRedraw();
}

void ProgramGUI::generateDraw()
{
	border.clear();
	border.setFillColor(ofColor(thisBackgroundColor, 180));
	border.setFilled(true);
	border.rectangle(b.x, b.y + spacingNextElement, b.width + 1, b.height);

	headerBg.clear();
	headerBg.setFillColor(thisHeaderBackgroundColor);
	headerBg.setFilled(true);
	headerBg.rectangle(b.x, b.y + 1 + spacingNextElement, b.width, header);

	ofRectangle nameBox = getTextBoundingBox(getName(), textPadding + b.x, 0);
	textMesh = getTextMesh(getName(), textPadding + b.x, header / 2 + 4 + b.y + spacingNextElement);

	const char * transport = mbRunning ? "> stop" : "> run";

	mRunRect = getTextBoundingBox(transport, 30 + nameBox.getMaxX(), 0);
	textMesh.append(getTextMesh(transport, mRunRect.x, header / 2 + 4 + b.y + spacingNextElement));

	if (minimized) 
	{
		textMesh.append(getTextMesh("+", b.width - textPadding - 24 + b.x, header / 2 + 4 + b.y + spacingNextElement));

		if (!mbRunning)
		{
			textMesh.append(getTextMesh(mProgramCode, textPadding + b.x, header + b.y + 4));
		}
	}
	else 
	{
		textMesh.append(getTextMesh("-", b.width - textPadding - 24 + b.x, header / 2 + 4 + b.y + spacingNextElement));
	}

	textMesh.append(getTextMesh("x", b.width - textPadding - 8 + b.x, header / 2 + 3 + b.y + spacingNextElement));
}

void ProgramGUI::onMaximize()
{
	// maximize of ofxGuiGroup calculates height based on contents, but we want to just return to whatever was explicitly set.
	b.width = mW;
	b.height = mH;
}

bool ProgramGUI::setValue(float mx, float my, bool bCheck) {

	if (!isGuiDrawing()) {
		bGuiActive = false;
		return false;
	}

	if (bCheck) {
		if (b.inside(mx, my)) {
			bGuiActive = true;

			int x = b.width - textPadding - 8;
			ofRectangle closeButton(b.x + x, b.y, b.width - x, header);
			if (closeButton.inside(mx, my)) {
				ofNotifyEvent(closePressedE, this);
				return true;
			}
			ofRectangle runButton(mRunRect.x, b.y, mRunRect.width, header);
			if (runButton.inside(mx, my)) {
				ofNotifyEvent(transportPressedE, this);
				return true;
			}
		}
	}

	return ofxGuiGroup::setValue(mx, my, bCheck);
}