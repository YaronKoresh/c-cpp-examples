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

    /* the shapes of list items are drawn where no control covers them */
    for (int width = 0; width <= 1600; width += 37) {
        for (int height = 0; height <= 1200; height += 41) {
            ListLayout layout = ComputeListLayout(width, height);
            const LayoutBox *controls[5] = {&layout.edit, &layout.list, &layout.addButton,
                                            &layout.removeButton, &layout.clearButton};
            assert(layout.canvas.width >= 0 && layout.canvas.height >= 0);
            for (int c = 0; c < 5; c++) {
                const LayoutBox *b = controls[c];
                int overlapX = b->x < layout.canvas.x + layout.canvas.width && layout.canvas.x < b->x + b->width;
                int overlapY = b->y < layout.canvas.y + layout.canvas.height && layout.canvas.y < b->y + b->height;
                assert(!(overlapX && overlapY && b->width > 0 && b->height > 0 &&
                         layout.canvas.width > 0 && layout.canvas.height > 0));
            }
            if (width >= 800 && height >= 300) {
                assert(layout.canvas.width >= 100);
            }
            for (int i = 0; i < 20; i++) {
                int x, y, size = 20 + i * 2;
                RandomShapePosition(&layout.canvas, size, &x, &y);
                if (layout.canvas.width >= size) {
                    assert(x >= layout.canvas.x && x + size <= layout.canvas.x + layout.canvas.width);
                } else {
                    assert(x == layout.canvas.x);
                }
                if (layout.canvas.height >= size) {
                    assert(y >= layout.canvas.y && y + size <= layout.canvas.y + layout.canvas.height);
                } else {
                    assert(y == layout.canvas.y);
                }
            }
        }
    }
    /* the buttons stay inside the left column, in order, without overlapping */
    for (int width = 0; width <= 1600; width++) {
        ListLayout layout = ComputeListLayout(width, 600);
        int right = layout.edit.x + layout.edit.width;
        assert(layout.addButton.x + layout.addButton.width <= layout.removeButton.x);
        assert(layout.removeButton.x + layout.removeButton.width <= layout.clearButton.x);
        assert(layout.clearButton.x + layout.clearButton.width <= right);
        assert(layout.addButton.width == layout.removeButton.width && layout.removeButton.width == layout.clearButton.width);
        assert(layout.addButton.width <= 150);
        if (width >= 500) {
            assert(layout.addButton.width == 150 && layout.clearButton.x == 330);
        }
        if (width >= 100) {
            assert(layout.addButton.width > 0);
        }
    }
    {
        ListLayout layout = ComputeListLayout(800, 600);
        assert(layout.list.width == LEFT_COLUMN_WIDTH && layout.canvas.x == 500 && layout.canvas.width == 300);
    }

    /* a stroke has a shape for every size the mouse moved (rounded up), and at least one */
    assert(StrokeSteps(5, 5, 5, 5, 10) == 1);
    assert(StrokeSteps(0, 0, 9, 0, 10) == 1);
    assert(StrokeSteps(0, 0, 100, 0, 10) == 10);
    assert(StrokeSteps(100, 0, 0, 30, 10) == 10);
    assert(StrokeSteps(0, 50, 20, -50, 10) == 10);
    assert(StrokeSteps(0, 0, 100, 0, 0) == 1);
    /* the shapes of a stroke are never farther apart than their size, so no gap is left */
    for (int distance = 0; distance <= 400; distance++) {
        int steps = StrokeSteps(0, 0, distance, 0, 10);
        assert(steps >= 1);
        assert(distance <= steps * 10);
        assert(steps == 1 || distance > (steps - 1) * 10);
    }
    assert(StrokeSteps(0, 0, 11, 0, 10) == 2);
    assert(StrokeSteps(0, 0, 19, 0, 10) == 2);
    assert(StrokeSteps(0, 0, 29, 0, 10) == 3);

    /* MK_LBUTTON is bit 0 of the mouse message key flags */
    assert(LeftButtonHeld(0x0001) == 1);
    assert(LeftButtonHeld(0x0005) == 1);
    assert(LeftButtonHeld(0x0004) == 0);
    assert(LeftButtonHeld(0) == 0);

    /* white space only is not a list item */
    assert(HasVisibleText("") == 0);
    assert(HasVisibleText("   \t ") == 0);
    assert(HasVisibleText(" a ") == 1);
    assert(HasVisibleText("\xd7\x90") == 1);
    assert(HasVisibleWideText(L"") == 0);
    assert(HasVisibleWideText(L"   \t ") == 0);
    assert(HasVisibleWideText(L" a ") == 1);
    assert(HasVisibleWideText(L"\x05d0") == 1);

    /* the pen starts black as a rectangle, steps through the colors, then the ellipses, then wraps */
    {
        unsigned long color;
        int type;
        PenStyleAt(0, &color, &type);
        assert(color == 0 && type == 0);
        PenStyleAt(1, &color, &type);
        assert(color == 0x0000fful && type == 0);
        PenStyleAt(PEN_COLORS, &color, &type);
        assert(color == 0 && type == 1);
        PenStyleAt(PEN_STYLES, &color, &type);
        assert(color == 0 && type == 0);
        PenStyleAt(-1, &color, &type);
        assert(color == 0x800080ul && type == 1);
        for (int i = 0; i < PEN_STYLES; i++) {
            unsigned long other;
            int otherType;
            PenStyleAt(i, &color, &type);
            PenStyleAt(i + 1, &other, &otherType);
            assert(color != other || type != otherType);
            assert(type == 0 || type == 1);
        }
    }

    puts("ok");
    return 0;
}
