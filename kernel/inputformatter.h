#ifndef __INPUT_FORMATTER
#define __INPUT_FORMATTER

#include "types.h"
#include "keyboard.h"

#define INPUTFORMATTER_BUFFER_SIZE 64 //TODO dynamically allocate this for infinite size

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
    const char* getLine();
private:
    char _buffer[INPUTFORMATTER_BUFFER_SIZE + 1];
    u32 _bufferIndex;
    bool _lineReady;
};

#endif /* end of include guard: __INPUT_FORMATTER */
