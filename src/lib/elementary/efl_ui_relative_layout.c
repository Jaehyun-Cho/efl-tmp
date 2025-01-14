#include "efl_ui_relative_layout_private.h"

#define MY_CLASS EFL_UI_RELATIVE_LAYOUT_CLASS
#define MY_CLASS_NAME "Efl.Ui.Relative_Layout"

#define LEFT               0
#define RIGHT              1
#define TOP                2
#define BOTTOM             3

#define START              (axis ? TOP : LEFT)
#define END                (axis ? BOTTOM : RIGHT)

static void _child_calc(Efl_Ui_Relative_Layout_Child *child, Eina_Bool axis);

static int
_chain_sort_cb(const void *l1, const void *l2)
{
   Efl_Ui_Relative_Layout_Calc *calc1, *calc2;

   calc1 = EINA_INLIST_CONTAINER_GET(l1, Efl_Ui_Relative_Layout_Calc);
   calc2 = EINA_INLIST_CONTAINER_GET(l2, Efl_Ui_Relative_Layout_Calc);

   return calc2->comp_factor <= calc1->comp_factor ? -1 : 1;
}

static void
_on_child_size_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Relative_Layout *obj = data;

   efl_pack_layout_request(obj);
}

static void
_on_child_hints_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Relative_Layout *obj = data;

   efl_pack_layout_request(obj);
}

static void
_on_child_del(void *data, const Efl_Event *event)
{
   Efl_Ui_Relative_Layout *obj = data;

   efl_pack_unpack(obj, event->object);
}

EFL_CALLBACKS_ARRAY_DEFINE(efl_ui_relative_layout_callbacks,
  { EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _on_child_size_changed },
  { EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _on_child_hints_changed },
  { EFL_EVENT_DEL, _on_child_del }
);

static Efl_Ui_Relative_Layout_Child *
_efl_ui_relative_layout_register(Efl_Ui_Relative_Layout_Data *pd, Eo *child)
{
   Efl_Ui_Relative_Layout_Child *rc;

   if (!efl_ui_widget_sub_object_add(pd->obj, child))
     return NULL;

   rc = calloc(1, sizeof(Efl_Ui_Relative_Layout_Child));
   if (!rc) return NULL;

   rc->obj = child;
   rc->layout = pd->obj;
   rc->rel[LEFT].to = rc->layout;
   rc->rel[LEFT].relative = 0.0;
   rc->rel[RIGHT].to = rc->layout;
   rc->rel[RIGHT].relative = 1.0;
   rc->rel[TOP].to = rc->layout;
   rc->rel[TOP].relative = 0.0;
   rc->rel[BOTTOM].to = rc->layout;
   rc->rel[BOTTOM].relative = 1.0;

   efl_key_data_set(child, "_elm_leaveme", pd->obj);
   efl_canvas_object_clipper_set(child, pd->clipper);
   efl_event_callback_array_add(child, efl_ui_relative_layout_callbacks(), pd->obj);
   efl_canvas_group_member_add(pd->obj, child);
   efl_canvas_group_change(pd->obj);

   eina_hash_add(pd->children, &child, rc);

   return rc;
}

static Efl_Ui_Relative_Layout_Child *
_relative_child_get(Efl_Ui_Relative_Layout_Data *pd, Eo *child)
{
   Efl_Ui_Relative_Layout_Child *rc;

   rc = eina_hash_find(pd->children, &child);
   if (!rc)
     rc = _efl_ui_relative_layout_register(pd, child);

   return rc;
}

static Efl_Ui_Relative_Layout_Child *
_relative_child_find(Efl_Ui_Relative_Layout_Data *pd, Eo *target)
{
   Efl_Ui_Relative_Layout_Child *child;

   if (pd->obj == target)
     return pd->base;

   child = eina_hash_find(pd->children, &target);
   if (!child)
     {
        ERR("target(%p(%s)) is not registered", target, efl_class_name_get(target));
        child = pd->base;
     }

   return child;
}

static void
_child_aspect_calc(Efl_Ui_Relative_Layout_Child *child, Eina_Bool axis)
{
   Efl_Ui_Relative_Layout_Calc *calc = &child->calc;
   double temph;

   if ((calc->aspect[0] <= 0) || (calc->aspect[1] <= 0))
     {
        ERR("Invalid aspect parameter for obj(%p), aspect(%d, %d) ",
            child->obj, calc->aspect[0], calc->aspect[1]);
        return;
     }

   switch (calc->aspect_type)
     {
      case EFL_GFX_HINT_ASPECT_HORIZONTAL:
        if (axis) _child_calc(child, !axis);
        calc->want[1].length = calc->want[0].length * calc->aspect[1] / calc->aspect[0];
        break;
      case EFL_GFX_HINT_ASPECT_VERTICAL:
        if (!axis) _child_calc(child, !axis);
        calc->want[0].length = calc->want[1].length * calc->aspect[0] / calc->aspect[1];
        break;
      case EFL_GFX_HINT_ASPECT_BOTH:
        if (calc->state[!axis] != RELATIVE_CALC_ON)
          _child_calc(child, !axis);
        temph = calc->want[axis].length * calc->aspect[!axis] / calc->aspect[axis];
        if (temph > calc->want[!axis].length)
          {
             temph = calc->want[!axis].length;
             calc->want[axis].length = temph * calc->aspect[axis] / calc->aspect[!axis];
          }
        else
          calc->want[!axis].length = temph;
        break;
      default:
        if (calc->state[!axis] != RELATIVE_CALC_ON)
          _child_calc(child, !axis);
        temph = calc->want[axis].length * calc->aspect[!axis] / calc->aspect[axis];
        if (temph < calc->want[!axis].length)
          {
             temph = calc->want[!axis].length;
             calc->want[axis].length = temph * calc->aspect[axis] / calc->aspect[!axis];
          }
        else
          calc->want[!axis].length = temph;
     }

   //calculate max size
   if (calc->want[0].length > calc->max[0])
     {
        calc->want[0].length = calc->max[0];
        calc->want[1].length = calc->want[0].length * calc->aspect[1] / calc->aspect[0];
     }
   if (calc->want[1].length > calc->max[1])
     {
        calc->want[1].length = calc->max[1];
        calc->want[0].length = calc->want[1].length * calc->aspect[0] / calc->aspect[1];
     }
   //calculate min size
   if (calc->aspect[1] > calc->aspect[0])
     calc->min[1] = calc->min[0] * calc->aspect[1] / calc->aspect[0];
   else
     calc->min[0] = calc->min[1] * calc->aspect[0] / calc->aspect[1];

   if (calc->want[0].length < calc->min[0])
     {
        calc->want[0].length = calc->min[0];
        calc->want[1].length = calc->want[0].length * calc->aspect[1] / calc->aspect[0];
     }
   if (calc->want[1].length < calc->min[1])
     {
        calc->want[1].length = calc->min[1];
        calc->want[0].length = calc->want[1].length * calc->aspect[0] / calc->aspect[1];
     }

   //calculate align
   calc->want[!axis].position =
      calc->space[!axis].position +
      (calc->space[!axis].length - calc->want[!axis].length) * calc->align[!axis];
}

static Eina_Bool
_child_chain_calc(Efl_Ui_Relative_Layout_Child *child, Eina_Bool axis)
{
   Efl_Ui_Relative_Layout_Child *head, *tail, *o;
   Efl_Gfx_Hint_Aspect aspect_type;
   int space, min_sum = 0;
   double weight_sum = 0, cur_pos;
   Eina_Inlist *chain = NULL;

   if (child->calc.chain_state[axis] == RELATIVE_CALC_DONE)
     return EINA_TRUE;

   if ((child != child->calc.to[START]->calc.to[END]) &&
       (child != child->calc.to[END]->calc.to[START]))
     return EINA_FALSE;

   // find head
   head = child;
   while (head == head->calc.to[START]->calc.to[END])
     {
        head = head->calc.to[START];
        if (head == child)
          {
             ERR("%c-axis circular dependency when calculating \"%s\"(%p).",
                 axis ? 'Y' : 'X', efl_class_name_get(child->obj), child->obj);
             return EINA_TRUE;
          }
     }

   //calculate weight_sum
   aspect_type = !axis ? EFL_GFX_HINT_ASPECT_VERTICAL : EFL_GFX_HINT_ASPECT_HORIZONTAL;
   o = head;
   do
     {
        if ((o->calc.aspect[0] > 0) && (o->calc.aspect[1] > 0) &&
            (o->calc.aspect_type == aspect_type))
          {
             _child_calc(o, !axis);
             if (o->calc.want[axis].length > o->calc.min[axis])
               o->calc.min[axis] = o->calc.want[axis].length;
          }
        else if ((o->calc.aspect[0] <= 0) ^ (o->calc.aspect[1] <= 0))
          {
             ERR("Invalid aspect parameter for obj(%p), aspect(%d, %d) ",
                 o->obj, o->calc.aspect[0], o->calc.aspect[1]);
          }

        o->calc.space[axis].length = o->calc.min[axis] +
                                     o->calc.margin[START] + o->calc.margin[END];
        min_sum += o->calc.space[axis].length;
        weight_sum += o->calc.weight[axis];

        tail = o;
        o = o->calc.to[END];
     }
   while (o->calc.to[START] == tail);

   _child_calc(head->calc.to[START], axis);
   _child_calc(tail->calc.to[END], axis);

   cur_pos = head->calc.to[START]->calc.want[axis].position +
             (head->calc.to[START]->calc.want[axis].length * head->rel[START].relative);
   space = tail->calc.to[END]->calc.want[axis].position +
           (tail->calc.to[END]->calc.want[axis].length * tail->rel[END].relative) - cur_pos;

   if ((space <= min_sum) || EINA_DBL_EQ(weight_sum, 0.0))
     cur_pos += (space - min_sum) * head->calc.align[axis];
   else
     {
        Efl_Ui_Relative_Layout_Calc *calc;
        double weight_len, orig_space = space, orig_weight = weight_sum;

        // Calculate compare factor
        for (o = head; o != tail->calc.to[END]; o = o->calc.to[END])
          {
             double denom;

             calc = &o->calc;
             denom = (calc->weight[axis] * orig_space) - (orig_weight * calc->min[axis]);
             if (denom > 0)
               {
                  calc->comp_factor = (calc->weight[axis] * orig_space) / denom;
                  chain = eina_inlist_sorted_insert(chain, EINA_INLIST_GET(calc),
                                                    _chain_sort_cb);
               }
             else
               {
                  space -= calc->space[axis].length;
                  weight_sum -= calc->weight[axis];
               }
          }

        EINA_INLIST_FOREACH(chain, calc)
          {
             weight_len = (space * calc->weight[axis]) / weight_sum;

             if (calc->space[axis].length < weight_len)
               calc->space[axis].length = weight_len;
             else
               {
                  weight_sum -= calc->weight[axis];
                  space -= calc->space[axis].length;
               }
          }
     }

   for (o = head; o != tail->calc.to[END]; o = o->calc.to[END])
     {
        o->calc.space[axis].position = cur_pos + o->calc.margin[START] + 0.5;
        cur_pos += o->calc.space[axis].length;
        o->calc.space[axis].length -= o->calc.margin[START] + o->calc.margin[END];
        o->calc.chain_state[axis] = RELATIVE_CALC_DONE;
        child->calc.m0[axis] += o->calc.min[axis];
     }

   child->calc.mi[axis] = head->rel[START].relative * (head->calc.to[START]->calc.mj[axis] -
                    head->calc.to[START]->calc.mi[axis]) + head->calc.to[START]->calc.mi[axis];
   child->calc.mj[axis] = tail->rel[END].relative * (tail->calc.to[END]->calc.mj[axis] -
                    tail->calc.to[END]->calc.mi[axis]) + tail->calc.to[END]->calc.mi[axis];
   child->calc.m0[axis] += -child->calc.min[axis] +
            (head->calc.to[START]->calc.m0[axis] * head->rel[START].relative) +
            (tail->calc.to[END]->calc.m0[axis] * (1 - tail->rel[END].relative));

   return EINA_TRUE;
}

static void
_child_calc(Efl_Ui_Relative_Layout_Child *child, Eina_Bool axis)
{
   Efl_Ui_Relative_Layout_Calc *calc = &child->calc;

   if (calc->state[axis] == RELATIVE_CALC_DONE)
     return;

   if (calc->state[axis] == RELATIVE_CALC_ON)
     {
        ERR("%c-axis circular dependency when calculating part \"%s\"(%p).",
            axis ? 'Y' : 'X', efl_class_name_get(child->obj), child->obj);
        return;
     }

   calc->state[axis] = RELATIVE_CALC_ON;

   if (!_child_chain_calc(child, axis))
     {
        _child_calc(calc->to[START], axis);
        _child_calc(calc->to[END], axis);

        calc->space[axis].position = calc->to[START]->calc.want[axis].position
                        + (calc->to[START]->calc.want[axis].length * child->rel[START].relative)
                        + calc->margin[START];
        calc->space[axis].length = calc->to[END]->calc.want[axis].position
                        + (calc->to[END]->calc.want[axis].length * child->rel[END].relative)
                        - calc->margin[END] - calc->space[axis].position;
     }

   if (calc->fill[axis] && (calc->weight[axis] > 0))
     calc->want[axis].length = calc->space[axis].length;

   if (!calc->aspect[0] && !calc->aspect[1])
     {
        if (calc->want[axis].length > calc->max[axis])
          calc->want[axis].length = calc->max[axis];

        if (calc->want[axis].length < calc->min[axis])
          calc->want[axis].length = calc->min[axis];
     }
   else
     {
        _child_aspect_calc(child, axis);
     }

   //calculate align
   calc->want[axis].position =
      calc->space[axis].position +
      (calc->space[axis].length - calc->want[axis].length) * calc->align[axis];

   child->calc.state[axis] = RELATIVE_CALC_DONE;
   if (child->calc.chain_state[axis] == RELATIVE_CALC_DONE)
     return;

   //calculate relative layout min
   calc->mi[axis] = child->rel[START].relative * (calc->to[START]->calc.mj[axis] -
                    calc->to[START]->calc.mi[axis]) + calc->to[START]->calc.mi[axis];
   calc->mj[axis] = child->rel[END].relative * (calc->to[END]->calc.mj[axis] -
                    calc->to[END]->calc.mi[axis]) + calc->to[END]->calc.mi[axis];
   calc->m0[axis] = calc->to[START]->calc.m0[axis] * child->rel[START].relative;

   if ((calc->to[START] == calc->to[END]) &&
       EINA_DBL_EQ(child->rel[START].relative, child->rel[END].relative))
     {
        double r, a; // relative, align
        r = calc->mi[axis] +
           (child->rel[START].relative * (calc->mj[axis] - calc->mi[axis]));
        a = calc->align[axis];
        calc->m0[axis] += (calc->min[axis] + calc->margin[START] + calc->margin[END]) *
           ((EINA_DBL_EQ(r, 0.0) || (!EINA_DBL_EQ(r, 1.0) && (a < r))) ?
            ((1 - a) / (1 - r)) : (a / r));
     }
   else
     {
        calc->m0[axis] += calc->to[END]->calc.m0[axis] * (1 - child->rel[END].relative);
     }

}

static void
_hash_free_cb(void *data)
{
   Efl_Ui_Relative_Layout_Child *child = data;

   efl_canvas_group_member_remove(child->layout, child->obj);
   efl_canvas_object_clipper_set(child->obj, NULL);
   efl_key_data_set(child->obj, "_elm_leaveme", NULL);
   efl_event_callback_array_del(child->obj, efl_ui_relative_layout_callbacks(),
                                child->layout);

   if (!efl_invalidated_get(child->obj))
     _elm_widget_sub_object_redirect_to_top(child->layout, child->obj);

   free(child);
}

static void
_hash_clear_cb(void *data)
{
   Efl_Ui_Relative_Layout_Child *child = data;

   efl_event_callback_array_del(child->obj, efl_ui_relative_layout_callbacks(),
                                child->layout);
   efl_del(child->obj);
}

static Eina_Bool
_hash_child_calc_foreach_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
                            void *data, void *fdata)
{
   Efl_Ui_Relative_Layout_Child *child = data;
   Efl_Ui_Relative_Layout_Calc *calc = &(child->calc);
   Efl_Ui_Relative_Layout_Data *pd = fdata;
   Eina_Rect want;
   int axis, layout_min;
   double min_len;

   _child_calc(child, 0);
   _child_calc(child, 1);

   want.x = calc->want[0].position;
   want.w = calc->want[0].length;
   want.y = calc->want[1].position;
   want.h = calc->want[1].length;

   for (axis = 0; axis < 2; axis++)
     {
        layout_min = 0;
        min_len = calc->mj[axis] - calc->mi[axis];
        if (EINA_DBL_EQ(min_len, 0.0))
          layout_min = calc->m0[axis];
        else
          layout_min = ((calc->min[axis] + calc->margin[START] +
                   calc->margin[END] + calc->m0[axis]) / fabs(min_len)) + 0.5;

        if (pd->base->calc.min[axis] < layout_min)
          pd->base->calc.min[axis] = layout_min;
     }

   efl_gfx_entity_geometry_set(child->obj, want);
   return EINA_TRUE;
}


static Eina_Bool
_hash_child_init_foreach_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
                            void *data, void *fdata)
{
   Eina_Size2D max, min, aspect;
   Efl_Ui_Relative_Layout_Child *child = data;
   Efl_Ui_Relative_Layout_Calc *calc = &(child->calc);
   Efl_Ui_Relative_Layout_Data *pd = fdata;

   calc->to[LEFT] = _relative_child_find(pd, child->rel[LEFT].to);
   calc->to[RIGHT] = _relative_child_find(pd, child->rel[RIGHT].to);
   calc->to[TOP] = _relative_child_find(pd, child->rel[TOP].to);
   calc->to[BOTTOM] = _relative_child_find(pd, child->rel[BOTTOM].to);

   calc->state[0] = RELATIVE_CALC_NONE;
   calc->state[1] = RELATIVE_CALC_NONE;
   calc->chain_state[0] = RELATIVE_CALC_NONE;
   calc->chain_state[1] = RELATIVE_CALC_NONE;

   efl_gfx_hint_weight_get(child->obj, &calc->weight[0], &calc->weight[1]);
   efl_gfx_hint_align_get(child->obj, &calc->align[0], &calc->align[1]);
   efl_gfx_hint_fill_get(child->obj, &calc->fill[0], &calc->fill[1]);
   efl_gfx_hint_aspect_get(child->obj, &calc->aspect_type, &aspect);
   calc->aspect[0] = aspect.w;
   calc->aspect[1] = aspect.h;
   efl_gfx_hint_margin_get(child->obj, &calc->margin[LEFT], &calc->margin[RIGHT],
                           &calc->margin[TOP], &calc->margin[BOTTOM]);
   max = efl_gfx_hint_size_max_get(child->obj);
   min = efl_gfx_hint_size_combined_min_get(child->obj);
   calc->max[0] = max.w;
   calc->max[1] = max.h;
   calc->min[0] = min.w;
   calc->min[1] = min.h;
   calc->m0[0] = 0.0;
   calc->m0[1] = 0.0;

   calc->want[0].position = 0;
   calc->want[0].length = 0;
   calc->want[1].position = 0;
   calc->want[1].length = 0;
   calc->space[0].position = 0;
   calc->space[0].length = 0;
   calc->space[1].position = 0;
   calc->space[1].length = 0;

   if (calc->weight[0] < 0) calc->weight[0] = 0;
   if (calc->weight[1] < 0) calc->weight[1] = 0;

   if (calc->align[0] < 0) calc->align[0] = 0;
   if (calc->align[1] < 0) calc->align[1] = 0;
   if (calc->align[0] > 1) calc->align[0] = 1;
   if (calc->align[1] > 1) calc->align[1] = 1;

   if (calc->max[0] < 0) calc->max[0] = INT_MAX;
   if (calc->max[1] < 0) calc->max[1] = INT_MAX;
   if (calc->aspect[0] < 0) calc->aspect[0] = 0;
   if (calc->aspect[1] < 0) calc->aspect[1] = 0;

   return EINA_TRUE;
}

static void
_efl_ui_relative_layout_hints_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   efl_pack_layout_request(ev->object);
}

EOLIAN static void
_efl_ui_relative_layout_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   Eina_Rect want = efl_gfx_entity_geometry_get(obj);
   pd->base->calc.want[0].position = want.x;
   pd->base->calc.want[0].length = want.w;
   pd->base->calc.want[1].position = want.y;
   pd->base->calc.want[1].length = want.h;
   pd->base->calc.min[0] = 0;
   pd->base->calc.min[1] = 0;

   eina_hash_foreach(pd->children, _hash_child_init_foreach_cb, pd);
   eina_hash_foreach(pd->children, _hash_child_calc_foreach_cb, pd);

   efl_gfx_hint_size_restricted_min_set(obj, EINA_SIZE2D(pd->base->calc.min[0], pd->base->calc.min[1]));

   efl_event_callback_call(obj, EFL_PACK_EVENT_LAYOUT_UPDATED, NULL);
}

EOLIAN static void
_efl_ui_relative_layout_efl_pack_layout_layout_request(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED)
{
   efl_canvas_group_need_recalculate_set(obj, EINA_TRUE);
}

EOLIAN static void
_efl_ui_relative_layout_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED)
{
   efl_pack_layout_update(obj);
}

EOLIAN static void
_efl_ui_relative_layout_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED, Eina_Size2D sz)
{
   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_relative_layout_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED, Eina_Position2D pos)
{
   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_relative_layout_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED)
{
   pd->clipper = efl_add(EFL_CANVAS_RECTANGLE_CLASS, obj);
   evas_object_static_clip_set(pd->clipper, EINA_TRUE);
   efl_gfx_entity_geometry_set(pd->clipper, EINA_RECT(-49999, -49999, 99999, 99999));
   efl_canvas_group_member_add(obj, pd->clipper);
   efl_ui_widget_sub_object_add(obj, pd->clipper);

   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_relative_layout_hints_changed_cb, NULL);
   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
}

EOLIAN static Eo *
_efl_ui_relative_layout_efl_object_constructor(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   efl_access_object_access_type_set(obj, EFL_ACCESS_TYPE_SKIPPED);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   pd->obj = obj;
   pd->children = eina_hash_pointer_new(_hash_free_cb);

   pd->base = calloc(1, sizeof(Efl_Ui_Relative_Layout_Child));
   if (!pd->base) return NULL;

   pd->base->obj = obj;
   pd->base->layout = obj;
   pd->base->rel[LEFT].to = obj;
   pd->base->rel[LEFT].relative = 0.0;
   pd->base->rel[RIGHT].to = obj;
   pd->base->rel[RIGHT].relative = 1.0;
   pd->base->rel[TOP].to = obj;
   pd->base->rel[TOP].relative = 0.0;
   pd->base->rel[BOTTOM].to = obj;
   pd->base->rel[BOTTOM].relative = 1.0;
   pd->base->calc.mi[0] = pd->base->calc.mi[1] = 0.0;
   pd->base->calc.mj[0] = pd->base->calc.mj[1] = 1.0;
   pd->base->calc.state[0] = RELATIVE_CALC_DONE;
   pd->base->calc.state[1] = RELATIVE_CALC_DONE;
   pd->base->calc.chain_state[0] = RELATIVE_CALC_DONE;
   pd->base->calc.chain_state[1] = RELATIVE_CALC_DONE;

   return obj;
}

EOLIAN static void
_efl_ui_relative_layout_efl_object_invalidate(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   efl_invalidate(efl_super(obj, MY_CLASS));

   eina_hash_free_buckets(pd->children);
}

EOLIAN static void
_efl_ui_relative_layout_efl_object_destructor(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_relative_layout_hints_changed_cb, NULL);
   eina_hash_free(pd->children);
   if (pd->base) free(pd->base);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_efl_ui_relative_layout_efl_pack_pack(Eo *obj EINA_UNUSED, Efl_Ui_Relative_Layout_Data *pd, Efl_Gfx_Entity *subobj)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(subobj, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!eina_hash_find(pd->children, &subobj), EINA_FALSE);

   return !!_efl_ui_relative_layout_register(pd, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_relative_layout_efl_pack_unpack(Eo *obj, Efl_Ui_Relative_Layout_Data *pd, Efl_Object *child)
{
   if (!eina_hash_del_by_key(pd->children, &child))
     {
        ERR("child(%p(%s)) is not registered", child, efl_class_name_get(child));
        return EINA_FALSE;
     }

   efl_pack_layout_request(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_relative_layout_efl_pack_unpack_all(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   eina_hash_free_buckets(pd->children);
   efl_pack_layout_request(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_relative_layout_efl_pack_pack_clear(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   eina_hash_free_cb_set(pd->children, _hash_clear_cb);
   eina_hash_free_buckets(pd->children);
   eina_hash_free_cb_set(pd->children, _hash_free_cb);

   efl_pack_layout_request(obj);

   return EINA_TRUE;
}

static Eina_Bool
_efl_ui_relative_layout_content_iterator_next(Efl_Ui_Relative_Layout_Content_Iterator *it, void **data)
{
   Efl_Ui_Relative_Layout_Child *child;

   if (!eina_iterator_next(it->real_iterator, (void **) &child))
     return EINA_FALSE;

   if (data) *data = child->obj;
   return EINA_TRUE;
}

static Eo *
_efl_ui_relative_layout_content_iterator_get_container(Efl_Ui_Relative_Layout_Content_Iterator *it)
{
   return it->relative_layout;
}

static void
_efl_ui_relative_layout_content_iterator_free(Efl_Ui_Relative_Layout_Content_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_ui_relative_layout_efl_container_content_iterate(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   Efl_Ui_Relative_Layout_Content_Iterator *it;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->relative_layout = obj;
   it->real_iterator = eina_hash_iterator_data_new(pd->children);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_efl_ui_relative_layout_content_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
     _efl_ui_relative_layout_content_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_efl_ui_relative_layout_content_iterator_free);

   return &it->iterator;
}

EOLIAN static int
_efl_ui_relative_layout_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Relative_Layout_Data *pd)
{
   return eina_hash_population(pd->children);
}

EFL_UI_RELATIVE_LAYOUT_RELATION_SET_GET(left, LEFT);
EFL_UI_RELATIVE_LAYOUT_RELATION_SET_GET(right, RIGHT);
EFL_UI_RELATIVE_LAYOUT_RELATION_SET_GET(top, TOP);
EFL_UI_RELATIVE_LAYOUT_RELATION_SET_GET(bottom, BOTTOM);

/* Internal EO APIs and hidden overrides */

#define EFL_UI_RELATIVE_LAYOUT_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_relative_layout)

#include "efl_ui_relative_layout.eo.c"
