#include "util/inputformatter.h"
#include "devicemanager.h"
#include "streams/vga.h"
#include "stdio.h"

InputFormatter::InputFormatter():
    _lineReady(0)
{
    _input.push_back(std::vector<char>());
}

void InputFormatter::handleVirtualKeyEvent(VirtualKeyEvent event)
{
    //we don't need releases keys :)
    if ((event.status & 0b00000001) == 0)
        return;

    //first lets test the special characters
    if (event.vkey == VirtualKeycode::ENTER) {
		deviceManager.getDevice(DeviceType::Screen, 0)->write('\n');
		_input.back().push_back('\0');
		_input.push_back(std::vector<char>());
		_lineReady = true;
		_logIndex = _input.size()-1;
		_lineIndex = 0;
    } else if (event.vkey == VirtualKeycode::BACKSPACE) {
        removeChars(1);
    } else if (event.vkey >= VirtualKeycode::A && event.vkey <= VirtualKeycode::Z) {
        if (event.status & 0b00000010) {
            char key = (u8)event.vkey - (u8)VirtualKeycode::A;
            addChar('A' + key);
        } else {
            char key = (u8)event.vkey - (u8)VirtualKeycode::A;
            addChar('a' + key);
        }
    } else if (event.vkey == VirtualKeycode::SPACE) {
        addChar(' ');
    } else if (event.vkey == VirtualKeycode::N_SLASH) {
        addChar('/');
    } else if (event.vkey == VirtualKeycode::N_MINUS) {
        addChar('-');
    } else if (event.vkey == VirtualKeycode::N_DOT) {
        addChar('.');
    } else if (event.vkey == VirtualKeycode::U_ARROW) {
		if (_logIndex > 0)
		{
			gotoLine(_logIndex-1);
		}
	} else if (event.vkey == VirtualKeycode::D_ARROW) {
		if (_logIndex < _input.size()-2)
		{
			gotoLine(_logIndex+1);
		}
	} else if (event.vkey == VirtualKeycode::L_ARROW) {
		moveCursorBy(-1);
	} else if (event.vkey == VirtualKeycode::R_ARROW) {
		moveCursorBy(1);
	} else if (event.vkey == VirtualKeycode::HOME) {
		moveCursorTo(0);
	} else if (event.vkey == VirtualKeycode::END) {
		moveCursorTo(_input.back().size());
	}
}

bool InputFormatter::isLineReady() const
{
    return _lineReady;
}

void InputFormatter::addChar(u8 character)
{
	_input.back().insert(_lineIndex++, character);
	for (int i = _lineIndex-1; i < _input.back().size(); i++)
		deviceManager.getDevice(DeviceType::Screen, 0)->write(_input.back()[i]);
	fseek(stdout, _lineIndex-_input.back().size(), SEEK_CUR);
}

void InputFormatter::gotoLine(int linenum)
{
	removeChars(_input.back().size());
	_input.back().clear();
	for (int i = 0; i < _input[linenum].size(); i++)
		_input.back().push_back(_input[linenum][i]);

	if (_input.back().back() == '\0')
		_input.back().pop_back();

	VgaDriver* vga = (VgaDriver*) deviceManager.getDevice(DeviceType::Screen, 0);
	vga->write(_input[linenum].data());
	_logIndex = linenum;
	_lineIndex = _input.back().size();
}

void InputFormatter::moveCursorBy(int amount)
{
	if (_lineIndex + amount < 0)
		amount = -_lineIndex;
	else if (_lineIndex + amount >= _input.back().size())
		amount = _input.back().size()-_lineIndex;

	fseek(stdout, amount, SEEK_CUR);
	_lineIndex += amount;
}

void InputFormatter::moveCursorTo(int position)
{
	moveCursorBy(position - _lineIndex);
}

void InputFormatter::removeChars(int num)
{
	if (_input.back().size() >= num)
	{
		fseek(stdout, -num, SEEK_CUR);
		for (int i = 0; i < num; i++)
			vgaDriver.write(" ", 1);
		fseek(stdout, -num, SEEK_CUR);
		_input.back().pop_back();
	}
}

std::string InputFormatter::getNextLine()
{
    std::vector<char> chars = _input[_input.size()-2];
	std::string string(chars.data(), chars.size());
	_lineReady = false;

    return string;
}
