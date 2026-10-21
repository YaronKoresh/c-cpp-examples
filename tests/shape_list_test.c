#include <assert.h>
#include <stdio.h>
#include "../shape_list.h"

static void fill(Shape *s, int x, int listed) {
    s->x = x;
    s->listed = listed;
}

int main(void) {
    Shape shapes[MAX_SHAPES] = {0};
    int num = 0;

    /* shape 0: drawn freehand, shape 1: listed "a", shape 2: unlisted, shape 3: listed "b" */
    fill(&shapes[num++], 0, 0);
    fill(&shapes[num++], 1, 1);
    fill(&shapes[num++], 2, 0);
    fill(&shapes[num++], 3, 1);

    assert(ShapeIndexForListItem(shapes, num, 0) == 1);
    assert(ShapeIndexForListItem(shapes, num, 1) == 3);
    assert(ShapeIndexForListItem(shapes, num, 2) == -1);
    assert(ShapeIndexForListItem(shapes, num, -1) == -1);

    /* removing list item 1 ("b") must remove shape x=3 and keep the others */
    RemoveShapeAt(shapes, &num, ShapeIndexForListItem(shapes, num, 1));
    assert(num == 3);
    assert(shapes[0].x == 0 && shapes[1].x == 1 && shapes[2].x == 2);

    /* an unknown list item removes nothing */
    RemoveShapeAt(shapes, &num, ShapeIndexForListItem(shapes, num, 5));
    assert(num == 3);

    /* mouse coordinates left of or above the window are negative, packed as 16-bit halves */
    assert(MouseXFromLParam(((long long)(unsigned short)-8 << 16) | (unsigned short)-5) == -5);
    assert(MouseYFromLParam(((long long)(unsigned short)-8 << 16) | (unsigned short)-5) == -8);
    assert(MouseXFromLParam(((long long)300 << 16) | 120) == 120);
    assert(MouseYFromLParam(((long long)300 << 16) | 120) == 300);

    /* a full array of freehand shapes loses the oldest one to make room */
    num = MAX_SHAPES;
    for (int i = 0; i < MAX_SHAPES; i++) {
        fill(&shapes[i], i, 0);
    }
    assert(MakeRoomForShape(shapes, &num) == 1);
    assert(num == MAX_SHAPES - 1 && shapes[0].x == 1);

    /* a full array where every shape is listed stays unchanged */
    num = MAX_SHAPES;
    for (int i = 0; i < MAX_SHAPES; i++) {
        fill(&shapes[i], i, 1);
    }
    assert(MakeRoomForShape(shapes, &num) == 0);
    assert(num == MAX_SHAPES && shapes[0].x == 0);

    /* an array with a free slot is not touched */
    num = 3;
    assert(MakeRoomForShape(shapes, &num) == 1 && num == 3);

    /* a zero-size window must not divide by zero */
    assert(RandomBelow(0) == 0);
    assert(RandomBelow(-5) == 0);
    for (int i = 0; i < 100; i++) {
        int v = RandomBelow(7);
        assert(v >= 0 && v < 7);
    }

    /* MK_LBUTTON is bit 0 of the mouse message key flags */
    assert(LeftButtonHeld(0x0001) == 1);
    assert(LeftButtonHeld(0x0005) == 1);
    assert(LeftButtonHeld(0x0004) == 0);
    assert(LeftButtonHeld(0) == 0);

    puts("ok");
    return 0;
}
