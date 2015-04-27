#ifndef POINTERCHANGESINK_H
#define POINTERCHANGESINK_H

#include <winpr/wtypes.h>
typedef struct rdp_pointer rdpPointer;

/**
 * The PointerChangeSink interface provides a sink where changes to mouse
 * cursor/pointer style can be fed into.
 */
class PointerChangeSink {
public:
    /**
     * Returns size (in bytes) of the struct referenced by rdpPointer that this
     * sink expects it to be.
     */
    virtual int getPointerStructSize() const = 0;

    /**
     * Adds new pointer style to the sink.
     */
    virtual BOOL addPointer(rdpPointer* pointer) = 0;

    /**
     * Removes previously added pointer style.
     */
    virtual void removePointer(rdpPointer* pointer) = 0;

    /**
     * Changes current pointer style to given pointer.
     */
    virtual BOOL changePointer(rdpPointer* pointer) = 0;
};

#endif // POINTERCHANGESINK_H
