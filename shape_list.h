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

/* One rectangle of the window layout, in client coordinates. */
typedef struct {
    int x, y, width, height;
} LayoutBox;

typedef struct {
    LayoutBox edit, list, addButton, removeButton, clearButton;
    LayoutBox canvas; /* free area on the right where the shapes are drawn */
} ListLayout;

#define LEFT_COLUMN_WIDTH 480

static inline int AtLeastZero(int value) {
    return value < 0 ? 0 : value;
}

/* Layout of list.c for a client area. The edit box, the list box and the buttons stay in the
   left column, so that the shapes of the list items are drawn in the canvas on the right
   instead of behind the list box. The canvas is empty when the window is too narrow. */
static inline ListLayout ComputeListLayout(int clientWidth, int clientHeight) {
    ListLayout layout;
    int left = AtLeastZero(clientWidth - 20);
    int buttonTop = clientHeight - 100;
    if (left > LEFT_COLUMN_WIDTH) {
        left = LEFT_COLUMN_WIDTH;
    }
    layout.edit = (LayoutBox){10, 10, left, 20};
    layout.list = (LayoutBox){10, 40, left, AtLeastZero(clientHeight - 150)};
    layout.addButton = (LayoutBox){10, buttonTop, 150, 30};
    layout.removeButton = (LayoutBox){170, buttonTop, 150, 30};
    layout.clearButton = (LayoutBox){330, buttonTop, 150, 30};
    layout.canvas.x = left + 20;
    layout.canvas.y = 0;
    layout.canvas.width = AtLeastZero(clientWidth - layout.canvas.x);
    layout.canvas.height = AtLeastZero(clientHeight);
    return layout;
}

/* Random top-left corner for a shape of the given size that lies completely inside the canvas.
   A canvas smaller than the shape (for example a minimized window) gives the canvas corner. */
static inline void RandomShapePosition(const LayoutBox *canvas, int size, int *x, int *y) {
    *x = canvas->x + RandomBelow(canvas->width - size + 1);
    *y = canvas->y + RandomBelow(canvas->height - size + 1);
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
