#include "MenuGUI.h"



MenuGUI::MenuGUI()
{
}


MenuGUI::~MenuGUI()
{
}

MenuGUI * MenuGUI::setup(const std::string& collectionName /*= ""*/, const std::string& filename /*= "menu.xml"*/, float x /*= 10*/, float y /*= 10*/)
{
	ofxGuiGroup::setup(collectionName, filename, x, y);
	setHeaderBackgroundColor(ofColor::black);
	setBorderColor(ofColor::black);
	spacing = 4;

	return this;
}

bool MenuGUI::setValue(float mx, float my, bool bCheck)
{
	if (!isGuiDrawing()) 
	{
		bGuiActive = false;
		return false;
	}

	if (bCheck) 
	{
		if (b.inside(mx, my)) 
		{
			bGuiActive = true;
		}
	}

	return false;
}

void MenuGUI::generateDraw()
{
	border.clear();
	border.setFillColor(ofColor(thisBorderColor, 180));
	border.setFilled(true);
	border.rectangle(b.x, b.y + spacingNextElement, b.width + 1, b.height);


	headerBg.clear();
	headerBg.setFillColor(thisHeaderBackgroundColor);
	headerBg.setFilled(true);
	headerBg.rectangle(b.x, b.y + 1 + spacingNextElement, b.width, header);

	textMesh = getTextMesh(getName(), textPadding + b.x, header / 2 + 4 + b.y + spacingNextElement);
}
