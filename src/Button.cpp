#include "Button.h"



Button::Button()
	: ofxButton()
	, hover(false)
	, focused(false)
{
}

Button::Button(ofParameter<int> _bVal, float width /* = defaultWidth */, float height /* = defaultHeight */)
	: ofxButton()
{
	setup(_bVal, width, height);
}

Button::~Button()
{
}

Button* Button::setup(ofParameter<int> _bVal, float width /* = defaultWidth */, float height /* = defaultHeight */)
{
	ofxButton::setup(_bVal.getName(), width, height);

	id = _bVal;
	hover = false;
	focused = false;
	checkboxRect.set(1, 1, b.width - 2, b.height - 2);

	addListener(this, &Button::onClick);

	return this;
}

Button* Button::setup(const std::string& toggleName, int bval, float width /* = defaultWidth */, float height /* = defaultHeight */)
{
	ofxButton::setup(toggleName, width, height);

	id = bval;
	hover = false;
	focused = false;
	checkboxRect.set(1, 1, b.width - 2, b.height - 2);

	addListener(this, &Button::onClick);

	return this;
}

bool Button::mouseMoved(ofMouseEventArgs & args)
{
	bool inside = ofxButton::mouseMoved(args);
	if (inside != hover)
	{
		hover = inside;
		if (hover)
		{
			ofNotifyEvent(hoveredE, id, this);
		}
	}

	return inside;
}

void Button::setFocused(bool focus)
{
	if (focused != focus)
	{
		setNeedsRedraw();
	}

	focused = focus;
}

void Button::generateDraw()
{
	bg.clear();
	bg.setFillColor(thisBackgroundColor);
	bg.rectangle(b);

	fg.clear();
	//if (hover) 
	//{
	//	fg.setFilled(true);
	//	fg.setFillColor(thisFillColor);
	//	fg.rectangle(b.getPosition() + checkboxRect.getTopLeft(), checkboxRect.width, checkboxRect.height);
	//}
	//else {
	//	fg.setFilled(false);
	//	fg.setStrokeWidth(1);
	//	fg.setStrokeColor(thisFillColor);
	//}	

	std::string name;
	auto textX = b.x + textPadding;
	if (getTextBoundingBox(getName(), textX, 0).getMaxX() > b.getMaxX() - textPadding) {
		for (auto c : ofUTF8Iterator(getName())) {
			auto next = name;
			ofUTF8Append(next, c);
			if (getTextBoundingBox(next, textX, 0).getMaxX() > b.getMaxX() - textPadding) {
				break;
			}
			else {
				name = next;
			}
		}
	}
	else {
		name = getName();
	}

	textMesh = getTextMesh(name, textX, b.y + b.height / 2 + 4);

	if (focused)
	{
		textX -= getTextBoundingBox("> ", textX, 0).getWidth();
		textMesh.append(getTextMesh("> ", textX, b.y + b.height / 2 + 4));
	}
}

void Button::onClick()
{
	ofNotifyEvent(clickedE, id, this);
}
