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

void ProgramGUI::sizeChangedCB()
{
	if (parent)
	{
		parent->sizeChangedCB();
	}
	setNeedsRedraw();
}
