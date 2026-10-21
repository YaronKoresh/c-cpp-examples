#ifndef SHAPE_LIST_H
#define SHAPE_LIST_H

#include <stdlib.h>
#include <ctype.h>
#include <wctype.h>

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
#define BUTTON_MAX_WIDTH 150
#define BUTTON_GAP 10

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
    int buttonWidth, gap;
    if (left > LEFT_COLUMN_WIDTH) {
        left = LEFT_COLUMN_WIDTH;
    }
    /* the three buttons share the width of the left column, up to 150 each */
    buttonWidth = AtLeastZero((left - 2 * BUTTON_GAP) / 3);
    if (buttonWidth > BUTTON_MAX_WIDTH) {
        buttonWidth = BUTTON_MAX_WIDTH;
    }
    gap = buttonWidth > 0 ? BUTTON_GAP : 0;
    layout.edit = (LayoutBox){10, 10, left, 20};
    layout.list = (LayoutBox){10, 40, left, AtLeastZero(clientHeight - 150)};
    layout.addButton = (LayoutBox){10, buttonTop, buttonWidth, 30};
    layout.removeButton = (LayoutBox){10 + buttonWidth + gap, buttonTop, buttonWidth, 30};
    layout.clearButton = (LayoutBox){10 + 2 * (buttonWidth + gap), buttonTop, buttonWidth, 30};
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

/* Number of shapes of the given size that draw a continuous line from one point to the next:
   the distance divided by the size, rounded up, and at least 1, so that neighbouring shapes
   are never farther apart than their size. */
static inline int StrokeSteps(int x0, int y0, int x1, int y1, int size) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int distance = dx > dy ? dx : dy;
    if (size < 1) {
        return 1;
    }
    return (distance + size - 1) / size > 1 ? (distance + size - 1) / size : 1;
}

#define PEN_COLORS 6
#define PEN_STYLES (PEN_COLORS * 2)

/* The freehand pen styles that a right click steps through: every color as a rectangle
   (shape type 0), then every color as an ellipse (shape type 1), then the first style again.
   The colors are black, red, green, blue, orange and purple, as 0x00BBGGRR values. */
static inline void PenStyleAt(int index, unsigned long *color, int *shapeType) {
    static const unsigned long colors[PEN_COLORS] = {
        0x000000ul, 0x0000fful, 0x00c800ul, 0xff0000ul, 0x0080fful, 0x800080ul
    };
    index %= PEN_STYLES;
    if (index < 0) {
        index += PEN_STYLES;
    }
    *color = colors[index % PEN_COLORS];
    *shapeType = index / PEN_COLORS;
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

/* Same as HasVisibleText for the wide text of a Unicode edit control. */
static inline int HasVisibleWideText(const wchar_t *text) {
    for (; *text; text++) {
        if (!iswspace((wint_t)*text)) {
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
