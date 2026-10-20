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

    puts("ok");
    return 0;
}
