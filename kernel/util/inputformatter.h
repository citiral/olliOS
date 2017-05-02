#ifndef __INPUT_FORMATTER
#define __INPUT_FORMATTER

#include "types.h"
#include "streams/keyboard.h"
#include "kstd/vector.h"

class InputFormatter {
public:
    InputFormatter();

    // "handles" the virtual key. so the buffer is updated.
    void handleVirtualKeyEvent(VirtualKeyEvent event);
    // returns true if there is a line ready to be fetched
    bool isLineReady() const;
    // adds a char to the buffer and renders it
    void addChar(u8 character);
    // gets the current line, null terminated
    std::vector<char> getNextLine();

private:
    std::vector<std::vector<char>> _input;
    bool _lineReady;
};

#endif /* end of include guard: __INPUT_FORMATTER */
