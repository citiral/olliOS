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
		logIndex = _input.size()-1;
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
		if (logIndex > 0)
		{
			gotoLine(logIndex-1);
		}
	} else if (event.vkey == VirtualKeycode::D_ARROW) {
		if (logIndex < _input.size()-2)
		{
			gotoLine(logIndex+1);
		}
	}
}

bool InputFormatter::isLineReady() const
{
    return _lineReady;
}

void InputFormatter::addChar(u8 character)
{
    _input.back().push_back(character);
    deviceManager.getDevice(DeviceType::Screen, 0)->write(character);
}

void InputFormatter::gotoLine(size_t linenum)
{
	removeChars(_input.back().size());
	_input.back().clear();
	for (int i = 0; i < _input[linenum].size(); i++)
		_input.back().push_back(_input[linenum][i]);

	if (_input.back().back() == '\0')
		_input.back().pop_back();

	VgaDriver* vga = (VgaDriver*) deviceManager.getDevice(DeviceType::Screen, 0);
	vga->write(_input[linenum].data());
	logIndex = linenum;
}

void InputFormatter::removeChars(size_t num)
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
