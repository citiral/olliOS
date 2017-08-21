#ifndef __INPUT_FORMATTER
#define __INPUT_FORMATTER

#include "types.h"
#include "devices/keyboard.h"
#include "kstd/vector.h"
#include "kstd/string.h"

class InputFormatter {
public:
	InputFormatter();

    // "handles" the virtual key. so the buffer is updated.
    void handleVirtualKeyEvent(VirtualKeyEvent event);
    // returns true if there is a line ready to be fetched
    bool isLineReady() const;
    // adds a char to the buffer and renders it
	void addChar(u8 character);
	// Remove a certain amount of characters from the current line.
	void removeChars(size_t amount);
    // gets the current line, null terminated
    std::string getNextLine();

private:
	// Go to a certain line in the log
	void gotoLine(int linenum);
	// Move the cursor with amount to right
	void moveCursorBy(int amount);
	// Move the cursor to a certain position
	void moveCursorTo(int position);

    std::vector<std::vector<char>> _input;
	bool _lineReady;
	size_t _logIndex = 0;
	size_t _lineIndex = 0;
};

#endif /* end of include guard: __INPUT_FORMATTER */
