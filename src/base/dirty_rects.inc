#include "base/widget_vtable.h"
#include "base/dirty_rects.h"

static inline ret_t dirty_rects_paint(dirty_rects_t* dirty_rects, widget_t* widget, canvas_t* c,
                                      widget_on_paint_t on_paint) {
  uint32_t i = 0;
  uint32_t cost = 0;
  rect_t* iter = NULL;
  uint64_t start = time_now_us();
  return_value_if_fail(dirty_rects != NULL, RET_BAD_PARAMS);
  return_value_if_fail(widget != NULL && c != NULL && on_paint != NULL, RET_BAD_PARAMS);
#if 0
  iter = &(dirty_rects->max);
  widget_paint_with_clip(widget, iter, c, on_paint);
  
  (void)i;
  cost = time_now_us() - start;
  log_debug("paint max rect(%d %d %d %d) cost=%u\n", iter->x, iter->y, iter->w, iter->h, cost);
#else
  dirty_rects_dump(dirty_rects);
  for (i = 0; i < dirty_rects->nr; i++) {
    rect_t* iter = dirty_rects->rects + i;
    uint64_t start1 = time_now_us();
    widget_paint_with_clip(widget, iter, c, on_paint);

    cost = time_now_us() - start1;
    log_debug("paint %u rect(%d %d %d %d) cost=%u\n", i, iter->x, iter->y, iter->w, iter->h, cost);
  }

  iter = &(dirty_rects->max);
  cost = time_now_us() - start;
  log_debug("paint total rect(%d %d %d %d) cost=%u\n", iter->x, iter->y, iter->w, iter->h, cost);
#endif/*WITH_ONE_RECT*/
  log_debug("\n\n");
  return RET_OK;
}
