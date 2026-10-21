#ifndef SHAPE_LIST_H
#define SHAPE_LIST_H

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

static inline void RemoveShapeAt(Shape *shapes, int *numShapes, int index) {
    if (index >= 0 && index < *numShapes) {
        for (int i = index; i < *numShapes - 1; i++) {
            shapes[i] = shapes[i + 1];
        }
        (*numShapes)--;
    }
}

#endif
