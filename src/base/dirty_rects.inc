#include "base/widget_vtable.h"
#include "base/dirty_rects.h"

static inline ret_t dirty_rects_paint(dirty_rects_t* dirty_rects, widget_t* widget, canvas_t* c,
                                      widget_on_paint_t on_paint) {
  uint32_t i = 0;
  return_value_if_fail(dirty_rects != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget != NULL && c != NULL && on_paint != NULL, RET_BAD_PARAMS);

  dirty_rects_dump(dirty_rects);

  for (i = 0; i < dirty_rects->nr; i++) {
    rect_t* iter = dirty_rects->rects + i;
    log_debug("paint rect(%d %d %d %d)\n", iter->x, iter->y, iter->w, iter->h);
    widget_paint_with_clip(widget, iter, c, on_paint);
  }

  return RET_OK;
}
