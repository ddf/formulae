#include "ProgramGUI.h"



ProgramGUI::ProgramGUI()
: ofxGuiGroup()
{
}


ProgramGUI::ProgramGUI(const ofParameterGroup & parameters, const std::string& _filename /*= "settings.xml"*/, float x /*= 10*/, float y /*= 10*/)
: ofxGuiGroup(parameters, filename, x, y)
{
}

ProgramGUI::~ProgramGUI()
{
}

void ProgramGUI::setup(const std::string& collectionName /* = "" */, const std::string& filename /* = "settings.xml" */, float x /* = 10 */, float y /* = 10 */)
{
	ofxGuiGroup::setup(collectionName, filename, x, y);
	spacing = 0;
	spacingNextElement = -1;
}

void ProgramGUI::sizeChangedCB()
{
	if (parent)
	{
		parent->sizeChangedCB();
	}
	setNeedsRedraw();
}
