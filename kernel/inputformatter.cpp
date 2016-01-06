#include "InputFormatter.h"
#include "vga.h"

InputFormatter::InputFormatter():
    _bufferIndex(0), _lineReady(0)
{

}

void InputFormatter::handleVirtualKeyEvent(VirtualKeyEvent event)
{
    //we don't need releases keys :)
    if ((event.status & 0b00000001) == 0)
        return;

    //first lets test the special characters
    if (event.vkey == VirtualKeycode::ENTER) {
        addChar('\n');
        _lineReady = true;
        _bufferIndex = 0;
    } else if (event.vkey == VirtualKeycode::BACKSPACE) {
        if (_bufferIndex > 0) {
            vgaDriver.seek(-1, SeekType::CURRENT);
            vgaDriver.write(" ", 1);
            vgaDriver.seek(-1, SeekType::CURRENT);
            _bufferIndex--;
        }
    } else if (event.vkey >= VirtualKeycode::A && event.vkey <= VirtualKeycode::Z) {
        char key = (u8)event.vkey - (u8)VirtualKeycode::A;
        addChar('a' + key);
    } else if (event.vkey == VirtualKeycode::SPACE) {
        addChar(' ');
    }
}

bool InputFormatter::isLineReady() const
{
    return _lineReady;
}

void InputFormatter::addChar(u8 character)
{
    if (_bufferIndex >= INPUTFORMATTER_BUFFER_SIZE) {
        _bufferIndex--;
    }
    _buffer[_bufferIndex] = character;
    //TODO dynamically get the proper stream to write to
    vgaDriver.write(_buffer + _bufferIndex, 1);
    _bufferIndex++;
}

const char* InputFormatter::getLine()
{
    _buffer[_bufferIndex] = '\0';
    return _buffer;
}
