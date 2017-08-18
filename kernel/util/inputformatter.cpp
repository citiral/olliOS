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
        _input.front().push_back('\0');
        _lineReady = true;
        _input.push_back(std::vector<char>());
    } else if (event.vkey == VirtualKeycode::BACKSPACE) {
        if (_input.front().size() > 0) {
            fseek(stdout, -1, SEEK_CUR);
            vgaDriver.write(" ", 1);
            fseek(stdout, -1, SEEK_CUR);
            _input.front().pop_back();
        }
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
    }
}

bool InputFormatter::isLineReady() const
{
    return _input.size() > 1;
}

void InputFormatter::addChar(u8 character)
{
    _input.front().push_back(character);
    deviceManager.getDevice(DeviceType::Screen, 0)->write(character);
}

std::vector<char> InputFormatter::getNextLine()
{
    std::vector<char> string = _input.front();
    _input.erase(0);
    return string;
}
