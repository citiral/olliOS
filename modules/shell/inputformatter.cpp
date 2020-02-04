#include "inputformatter.h"
#include "keyboard/keyboard.h"
#include "bindings.h"
#include "stdio.h"
#include "string.h"

using namespace keyboard;

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
		putchar('\n');
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
    } else if (event.vkey == VirtualKeycode::TAB) {
        addChar('\t');
    } else if (event.vkey == VirtualKeycode::OPEN_SQUARE) {
        addChar(event.status & 0b00000010 ? '{' : '[');  
    } else if (event.vkey == VirtualKeycode::CLOSE_SQUARE) {
        addChar(event.status & 0b00000010 ? '}' : ']');  
    } else if (event.vkey == VirtualKeycode::TILDE) {
        addChar(event.status & 0b00000010 ? '~' : '`');  
    } else if (event.vkey == VirtualKeycode::COLON) {
        addChar(event.status & 0b00000010 ? ':' : ';');  
    } else if (event.vkey >= VirtualKeycode::T_0 && event.vkey <= VirtualKeycode::T_11) {
        if (event.status & 0b00000010) {
            const char* keys = "!@#$%^&*()_+";
            addChar(keys[((u8)event.vkey - (u8)VirtualKeycode::T_0)]);
        } else {
            if (event.vkey == VirtualKeycode::T_10)
                addChar('-');
            else if (event.vkey == VirtualKeycode::T_11)
                addChar('=');
            else
                addChar('0' + ((u8)event.vkey - (u8)VirtualKeycode::T_0));
        }
    }else if (event.vkey == VirtualKeycode::SPACE) {
        addChar(' ');
    } else if (event.vkey == VirtualKeycode::N_SLASH || event.vkey == VirtualKeycode::SLASH) {
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
	} else if (event.vkey == VirtualKeycode::DELETE) {
		deleteChars(1);
	}
}

bool InputFormatter::isLineReady() const
{
    return _lineReady;
}

void InputFormatter::addChar(u8 character)
{
	_input.back().insert(_lineIndex++, character);

	size_t index = 0;
	for (size_t i = _lineIndex-1; i < _input.back().size(); i++) {
		putchar(_input.back()[i]);
		index++;
	}
	while (index > 1) {
		putchar('\b');
		index--;
	}
}

void InputFormatter::gotoLine(int linenum)
{
	removeChars(_input.back().size());
	_input.back().clear();
	for (size_t i = 0; i < _input[linenum].size(); i++)
		_input.back().push_back(_input[linenum][i]);

	if (_input.back().back() == '\0')
		_input.back().pop_back();

	printf("%s", _input[linenum].data());
	_logIndex = linenum;
	_lineIndex = _input.back().size();
}

void InputFormatter::moveCursorBy(int amount)
{
	// Clamp movement inside the input
	if (amount < 0 && (unsigned int) -amount >= _lineIndex)
		amount = -_lineIndex;
	else if (_lineIndex + amount > _input.back().size())
		amount = _input.back().size()-_lineIndex;
	
	// Move cursor left if amount is negative
	while (amount < 0) {
		putchar('\b');
		amount++;
		_lineIndex--;
	}

	// Move cursor right if amount is positive
	while (amount > 0) {
		putchar(_input.back()[_lineIndex]);
		amount--;
		_lineIndex++;
	}
}

void InputFormatter::moveCursorTo(int position)
{
	moveCursorBy(position - _lineIndex);
}

void InputFormatter::removeChars(size_t num)
{
	// Never erase more than the input is bug
	if (_input.back().size() >= num)
	{
		// Dont remove when at the start of a line
		if (_lineIndex == 0)
			return;
			
		// Remove the character from the input buffer
		for (size_t i = 0; i < num; i++)
			_input.back().erase(_lineIndex-1);

		// move cursor to before deleted character
		moveCursorBy(-num);
		
		// Redraw characters to the right of the cursor, to move them one to the left
		size_t t = 0;
		for (size_t i = _lineIndex; i < _input.back().size(); i++) {
			putchar(_input.back()[i]);
			_lineIndex++;
			t++;
		}

		// Delete old characters by writing spaces over them
		for (size_t i = 0 ; i < num ; i++) {
			putchar(' ');
			_lineIndex++;
			t++;
		}

		// Move cursor back to old position
		moveCursorBy(-t);
	}
}

void InputFormatter::deleteChars(size_t num)
{
	// Never erase more than the input is bug
	if (_input.back().size() >= num)
	{
		// Dont remove when at the end of a line
		if (_lineIndex == _input.back().size())
			return;
			
		// Remove the character from the input buffer
		for (size_t i = 0; i < num; i++)
			_input.back().erase(_lineIndex);
		
		// Redraw characters to the right of the cursor, to move them one to the left
		size_t t = 0;
		for (size_t i = _lineIndex; i < _input.back().size(); i++) {
			putchar(_input.back()[i]);
			_lineIndex++;
			t++;
		}

		// Delete old characters by writing spaces over them
		for (size_t i = 0 ; i < num ; i++) {
			putchar(' ');
			_lineIndex++;
			t++;
		}

		// Move cursor back to old position
		moveCursorBy(-t);
	}
}

std::string InputFormatter::getNextLine()
{
    std::vector<char> chars = _input[_input.size()-2];
	std::string string(chars.data(), chars.size());
	_lineReady = false;

    return string;
}
