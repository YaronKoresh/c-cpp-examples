#ifndef SHAPE_LIST_H
#define SHAPE_LIST_H

#include <stdlib.h>
#include <ctype.h>

#define MAX_SHAPES 100

typedef struct {
    int x, y, size;
    unsigned long color;
    int shapeType;
    int listed;
} Shape;

/* Array index of the shape that belongs to the listIndex-th list box item, or -1. */
static inline int ShapeIndexForListItem(const Shape *shapes, int numShapes, int listIndex) {
    int seen = 0;
    for (int i = 0; i < numShapes; i++) {
        if (shapes[i].listed) {
            if (seen == listIndex) {
                return i;
            }
            seen++;
        }
    }
    return -1;
}

/* Signed client coordinates packed in a mouse message lParam (like GET_X_LPARAM / GET_Y_LPARAM). */
static inline int MouseXFromLParam(long long lParam) {
    return (short)(lParam & 0xffff);
}

static inline int MouseYFromLParam(long long lParam) {
    return (short)((lParam >> 16) & 0xffff);
}

/* Random number in [0, limit). Returns 0 when limit is below 1 (for example a minimized window). */
static inline int RandomBelow(int limit) {
    if (limit < 1) {
        return 0;
    }
    return rand() % limit;
}

/* True when the left mouse button is held, from the wParam key flags of a mouse message (MK_LBUTTON). */
static inline int LeftButtonHeld(unsigned long long keyFlags) {
    return (keyFlags & 0x0001) != 0;
}

/* True when the text has at least one character that is not white space. */
static inline int HasVisibleText(const char *text) {
    for (; *text; text++) {
        if (!isspace((unsigned char)*text)) {
            return 1;
        }
    }
    return 0;
}

static inline void RemoveShapeAt(Shape *shapes, int *numShapes, int index) {
    if (index >= 0 && index < *numShapes) {
        for (int i = index; i < *numShapes - 1; i++) {
            shapes[i] = shapes[i + 1];
        }
        (*numShapes)--;
    }
}

/* Makes room for one more shape. When the array is full the oldest freehand
   (unlisted) shape is removed. Returns 1 if a slot is free, 0 if every shape is listed. */
static inline int MakeRoomForShape(Shape *shapes, int *numShapes) {
    if (*numShapes < MAX_SHAPES) {
        return 1;
    }
    for (int i = 0; i < *numShapes; i++) {
        if (!shapes[i].listed) {
            RemoveShapeAt(shapes, numShapes, i);
            return 1;
        }
    }
    return 0;
}

#endif
