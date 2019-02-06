#include "MenuGUI.h"
#include "ofEvent.h"

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
	headerTextBox = getTextBoundingBox(collectionName, 0, 0);
	header = headerTextBox.height;
	b.height = header;
	spacing = 4;
	spacingNextElement = -1;
	focusedButton = nullptr;	

	return this;
}

void MenuGUI::add(std::string & buttonName)
{
	auto button = new Button(ofParameter<int>(buttonName, collection.size()));
	ofAddListener(button->clickedE, this, &MenuGUI::onButtonClick);
	ofAddListener(button->hoveredE, this, &MenuGUI::onButtonHover);
	ofxGuiGroup::add(button);
	button->setPosition(button->getPosition() + ofPoint(30, 0));
	if (focusedButton == nullptr)
	{
		button->setFocused(true);
		focusedButton = button;
	}
}

bool MenuGUI::keyPressed(ofKeyEventArgs& args)
{
	if (focusedButton == nullptr) return false;

	const int key = args.key;

	if (key == OF_KEY_RETURN)
	{
		int id = focusedButton->getID();
		onButtonClick(id);
		return true;
	}
	else if (key == OF_KEY_DOWN && focusedButton->getID() < collection.size())
	{
		int next = focusedButton->getID() + 1;
		onButtonHover(next);
		return true;
	}
	else if (key == OF_KEY_UP && focusedButton->getID() > 0)
	{
		int next = focusedButton->getID() - 1;
		onButtonHover(next);
		return true;
	}
	else
	{
		int idx = key - '1';
		if (idx >= 0 && idx < collection.size())
		{
			onButtonHover(idx);
			return true;
		}
	}

	return false;
}

void MenuGUI::keyReleased(ofKeyEventArgs& args)
{

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

	textMesh = getTextMesh(getName(), textPadding + b.x, header / 2 - headerTextBox.height / 2 + 4 + b.y + spacingNextElement);
}

void MenuGUI::onButtonClick(int& id)
{
	ofNotifyEvent(buttonClickedE, id, this);
}

void MenuGUI::onButtonHover(int& id)
{
	Button* button = dynamic_cast<Button*>(getControl(id));
	if (button != nullptr && focusedButton != button)
	{
		focusedButton->setFocused(false);
		button->setFocused(true);
		focusedButton = button;
	}
}

void MenuGUI::onMinimize()
{
	ofUnregisterKeyEvents(this, defaultEventsPriority);
}

void MenuGUI::onMaximize()
{
	ofRegisterKeyEvents(this, defaultEventsPriority);
}
