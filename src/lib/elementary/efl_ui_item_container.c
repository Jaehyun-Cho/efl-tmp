#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#define MY_CLASS      EFL_UI_ITEM_CONTAINER_CLASS

#define MY_DATA_GET(obj, pd) \
  Efl_Ui_Item_Container_Data *pd = efl_data_scope_get(obj, MY_CLASS);

typedef struct {
   Efl_Ui_Scroll_Manager *smanager;
   Efl_Ui_Pan *pan;
   Eina_List *selected;
   Eina_List *items;
   Efl_Ui_Select_Mode mode;
   Efl_Ui_Layout_Orientation dir;
   struct {
     double horizontal;
     double vertical;
     double scalable;
   } padding;
   struct {
     double horizontal;
     double vertical;
   } align;
   Efl_Ui_Item_Position_Manager *pos_man;
   struct {
      Eina_Accessor pass_on;
      Eina_Accessor *real_acc;
   } obj_accessor;
   struct {
      Eina_Bool w;
      Eina_Bool h;
   } match_content;
   Eina_Accessor size_accessor;
   Efl_Gfx_Entity *sizer;
} Efl_Ui_Item_Container_Data;

static Eina_Bool register_item(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item);
static Eina_Bool unregister_item(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item);

static int
clamp_index(Efl_Ui_Item_Container_Data *pd, int index)
{
   if (index < ((int)eina_list_count(pd->items)) * -1)
     return -1;
   else if (index > (int)eina_list_count(pd->items) - 1)
     return 1;
   return 0;
}

static int
index_adjust(Efl_Ui_Item_Container_Data *pd, int index)
{
   int c = eina_list_count(pd->items);
   if (index < c * -1)
     return 0;
   else if (index > c - 1)
     return c - 1;
   else if (index < 0)
     return index + c;
   return index;
}

static Eina_Bool
_obj_accessor_get_at(Eina_Accessor *accessor, unsigned int idx, void **data)
{
   ptrdiff_t offset = offsetof(Efl_Ui_Item_Container_Data, obj_accessor);
   Efl_Ui_Item_Container_Data *pd = (void*)accessor - offset;

   if (!pd->items)
     {
        *data = NULL;
        return EINA_FALSE;
     }

   if (!pd->obj_accessor.real_acc)
     {
        pd->obj_accessor.real_acc = eina_list_accessor_new(pd->items);
     }
   return eina_accessor_data_get(pd->obj_accessor.real_acc, idx, data);
}

static Eina_Accessor*
_obj_clone(Eina_Accessor *accessor)
{
   ptrdiff_t offset = offsetof(Efl_Ui_Item_Container_Data, obj_accessor);
   Efl_Ui_Item_Container_Data *pd = (void*)accessor - offset;

   if (!pd->obj_accessor.real_acc)
     return NULL;

   return eina_accessor_clone(pd->obj_accessor.real_acc);
}

static Eina_List *
_null_container(Eina_Accessor *accessor EINA_UNUSED)
{
   ERR("Not allowed to get a container!");
   return NULL;
}

static void
_free(Eina_Accessor *accessor EINA_UNUSED)
{
   ERR("Freeing this accessor is not supported");
}

static void
_obj_accessor_init(Eina_Accessor *accessor)
{
   //this is the accessor for accessing the items
   //we have to workarround here the problem that
   //no accessor can be created for a not yet created list.
   accessor->version = EINA_ACCESSOR_VERSION;
   accessor->get_at = FUNC_ACCESSOR_GET_AT(_obj_accessor_get_at);
   accessor->clone = FUNC_ACCESSOR_CLONE(_obj_clone);
   accessor->get_container = FUNC_ACCESSOR_GET_CONTAINER(_null_container);
   accessor->free = FUNC_ACCESSOR_FREE(_free);
   EINA_MAGIC_SET(accessor, EINA_MAGIC_ACCESSOR);
}

static Eina_Bool
_size_accessor_get_at(Eina_Accessor *accessor, unsigned int idx, void **data)
{
   Eina_Size2D *size = (Eina_Size2D*)data;
   ptrdiff_t offset = offsetof(Efl_Ui_Item_Container_Data, size_accessor);
   Efl_Ui_Item_Container_Data *pd = (void*)accessor - offset;

   if (idx > eina_list_count(pd->items))
     return EINA_FALSE;

   Eo *subobj = eina_list_nth(pd->items, idx);

   *size = efl_gfx_hint_size_combined_min_get(subobj);

   return EINA_TRUE;
}

static Eina_Accessor*
_size_clone(Eina_Accessor *accessor EINA_UNUSED)
{
   return NULL;
}

static void
_size_accessor_init(Eina_Accessor *accessor)
{
   accessor->version = EINA_ACCESSOR_VERSION;
   accessor->get_at = FUNC_ACCESSOR_GET_AT(_size_accessor_get_at);
   accessor->clone = FUNC_ACCESSOR_CLONE(_size_clone);
   accessor->get_container = FUNC_ACCESSOR_GET_CONTAINER(_null_container);
   accessor->free = FUNC_ACCESSOR_FREE(_free);
   EINA_MAGIC_SET(accessor, EINA_MAGIC_ACCESSOR);
}

static void
_pan_viewport_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   MY_DATA_GET(data, pd);
   Eina_Rect rect = efl_ui_scrollable_viewport_geometry_get(data);

   efl_ui_item_position_manager_viewport_set(pd->pos_man, rect);
}

static void
_pan_position_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   MY_DATA_GET(data, pd);
   Eina_Position2D pos = efl_ui_pan_position_get(pd->pan);
   Eina_Position2D max = efl_ui_pan_position_max_get(pd->pan);
   Eina_Vector2 rpos = {0.0, 0.0};

   if (max.x > 0.0)
     rpos.x = (double)pos.x/(double)max.x;
   if (max.y > 0.0)
     rpos.y = (double)pos.y/(double)max.y;

   efl_ui_item_position_manager_scroll_positon_set(pd->pos_man, rpos.x, rpos.y);
}

EFL_CALLBACKS_ARRAY_DEFINE(pan_events_cb,
  {EFL_UI_PAN_EVENT_PAN_POSITION_CHANGED, _pan_position_changed_cb},
  {EFL_UI_PAN_EVENT_PAN_VIEWPORT_CHANGED, _pan_viewport_changed_cb},
)

static void
_item_scroll_internal(Eo *obj EINA_UNUSED,
                      Efl_Ui_Item_Container_Data *pd,
                      Efl_Ui_Item *item,
                      double align,
                      Eina_Bool anim)
{
   Eina_Rect ipos, view;
   Eina_Position2D vpos;

   if (!pd->smanager) return;

   efl_ui_item_position_manager_position_single_item(pd->pos_man, eina_list_data_idx(pd->items, item));
   ipos = efl_gfx_entity_geometry_get(item);
   view = efl_ui_scrollable_viewport_geometry_get(pd->smanager);
   vpos = efl_ui_scrollable_content_pos_get(pd->smanager);

   ipos.x = ipos.x + vpos.x - view.x;
   ipos.y = ipos.y + vpos.y - view.y;

   //FIXME scrollable needs some sort of align, the docs do not even garantee to completly move in the element
   efl_ui_scrollable_scroll(pd->smanager, ipos, anim);
}

EOLIAN static void
_efl_ui_item_container_item_scroll(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item, Eina_Bool animation)
{
   _item_scroll_internal(obj, pd, item, -1.0, animation);
}

EOLIAN static void
_efl_ui_item_container_item_scroll_align(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item, double align, Eina_Bool animation)
{
   _item_scroll_internal(obj, pd, item, align, animation);
}

EOLIAN static Efl_Ui_Item*
_efl_ui_item_container_last_selected_item_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return eina_list_last_data_get(pd->selected);
}

EOLIAN static Eina_Iterator*
_efl_ui_item_container_selected_items_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return eina_list_iterator_new(pd->selected);
}

EOLIAN static Efl_Object*
_efl_ui_item_container_efl_object_constructor(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED)
{
   Eo *o;

   _obj_accessor_init(&pd->obj_accessor.pass_on);
   _size_accessor_init(&pd->size_accessor);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "grid"); //FIXME this needs its own theme

   o = efl_constructor(efl_super(obj, MY_CLASS));

   pd->sizer = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(obj));
   efl_gfx_color_set(pd->sizer, 200, 200, 200, 200);

   pd->pan = efl_add(EFL_UI_PAN_CLASS, obj);
   efl_content_set(pd->pan, pd->sizer);
   efl_event_callback_array_add(pd->pan, pan_events_cb(), obj);

   pd->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
   efl_composite_attach(obj, pd->smanager);
   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));
   efl_ui_scroll_manager_pan_set(pd->smanager, pd->pan);

   elm_layout_sizing_eval(obj);
   efl_ui_scroll_connector_bind(obj, pd->smanager);

   return o;
}

EOLIAN static Efl_Object*
_efl_ui_item_container_efl_object_finalize(Eo *obj, Efl_Ui_Item_Container_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->pos_man, NULL);

   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_ui_item_container_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Item_Container_Data *pd)
{
   Eina_Error res;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_ERROR_GENERIC);
   res = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (res == EFL_UI_THEME_APPLY_ERROR_GENERIC) return res;
   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));
   efl_content_set(efl_part(wd->resize_obj, "efl.content"), pd->pan);

   return res;
}

EOLIAN static void
_efl_ui_item_container_efl_object_destructor(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_item_container_efl_object_invalidate(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED)
{
   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Iterator*
_efl_ui_item_container_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return eina_list_iterator_new(pd->items);
}

EOLIAN static int
_efl_ui_item_container_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return eina_list_count(pd->items);
}

EOLIAN static void
_efl_ui_item_container_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Layout_Orientation dir)
{
   pd->dir = dir;
   efl_ui_layout_orientation_set(pd->pos_man, dir);
}

EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_item_container_efl_ui_layout_orientable_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return pd->dir;
}

EOLIAN static void
_efl_ui_item_container_efl_gfx_arrangement_content_padding_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, double pad_horiz, double pad_vert, Eina_Bool scalable)
{
   pd->padding.horizontal = pad_horiz;
   pd->padding.vertical = pad_vert;
   efl_gfx_arrangement_content_padding_set(pd->pos_man, pad_horiz, pad_vert, scalable);
}

EOLIAN static void
_efl_ui_item_container_efl_gfx_arrangement_content_padding_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, double *pad_horiz, double *pad_vert, Eina_Bool *scalable)
{
   if (pad_horiz)
     *pad_horiz = pd->padding.horizontal;
   if (pad_vert)
     *pad_vert = pd->padding.vertical;
   if (scalable)
     *scalable = pd->padding.scalable;
}

EOLIAN static void
_efl_ui_item_container_efl_gfx_arrangement_content_align_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, double align_horiz, double align_vert)
{
   pd->align.horizontal = align_horiz;
   pd->align.vertical = align_vert;
   efl_gfx_arrangement_content_align_set(pd->pos_man, align_horiz, align_vert);
}

EOLIAN static void
_efl_ui_item_container_efl_gfx_arrangement_content_align_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, double *align_horiz, double *align_vert)
{
   if (align_horiz)
     *align_horiz = pd->align.horizontal;
   if (align_vert)
     *align_vert = pd->align.vertical;
}

EOLIAN static void
_efl_ui_item_container_efl_ui_scrollable_interactive_match_content_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, Eina_Bool w, Eina_Bool h)
{
   if (pd->match_content.w == w && pd->match_content.h == h)
     return;

   pd->match_content.w = w;
   pd->match_content.h = h;

   efl_ui_scrollable_match_content_set(pd->smanager, w, h);
   elm_layout_sizing_eval(obj);
}

static void
deselect_all(Efl_Ui_Item_Container_Data *pd)
{
   while(pd->selected)
     {
        Eo *item = eina_list_data_get(pd->selected);
        efl_ui_item_selected_set(item, EINA_FALSE);
        EINA_SAFETY_ON_TRUE_RETURN(eina_list_data_get(pd->selected) == item);
     }
}


EOLIAN static void
_efl_ui_item_container_efl_ui_multi_selectable_select_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Select_Mode mode)
{
   pd->mode = mode;
   if ((mode == EFL_UI_SELECT_MODE_SINGLE_ALWAYS || mode == EFL_UI_SELECT_MODE_SINGLE) &&
       eina_list_count(pd->selected) > 0)
     {
        Efl_Ui_Item *last = eina_list_last_data_get(pd->selected);

        pd->selected = eina_list_remove_list(pd->selected, eina_list_last(pd->selected));
        deselect_all(pd);
        pd->selected = eina_list_append(pd->selected, last);
     }
   else if (mode == EFL_UI_SELECT_MODE_NONE && pd->selected)
     {
        deselect_all(pd);
     }
}

EOLIAN static Efl_Ui_Select_Mode
_efl_ui_item_container_efl_ui_multi_selectable_select_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   return pd->mode;
}

static void
_selected_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);

   if (pd->mode == EFL_UI_SELECT_MODE_SINGLE_ALWAYS || pd->mode == EFL_UI_SELECT_MODE_SINGLE)
     {
        deselect_all(pd);
     }
   else if (pd->mode == EFL_UI_SELECT_MODE_NONE)
     {
        ERR("Selection while mode is NONE, uncaught state!");
        return;
     }
   pd->selected = eina_list_append(pd->selected, ev->object);
}

static void
_unselected_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);

   pd->selected = eina_list_remove(pd->selected, ev->object);
}

static void
_invalidate_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);

   unregister_item(obj, pd, ev->object);
}

static void
_hints_changed_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;
   MY_DATA_GET(obj, pd);
   int idx = eina_list_data_idx(pd->items, ev->object);

   efl_ui_item_position_manager_item_size_changed(pd->pos_man, idx, ev->object);
}

static void
_redirect_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;

#define REDIRECT_EVT(item_evt, item) \
   if (item_evt == ev->desc) efl_event_callback_call(obj, item, ev->object);
   REDIRECT_EVT(EFL_UI_EVENT_PRESSED, EFL_UI_EVENT_ITEM_PRESSED);
   REDIRECT_EVT(EFL_UI_EVENT_UNPRESSED, EFL_UI_EVENT_ITEM_UNPRESSED);
   REDIRECT_EVT(EFL_UI_EVENT_LONGPRESSED, EFL_UI_EVENT_ITEM_LONGPRESSED);
   REDIRECT_EVT(EFL_UI_EVENT_CLICKED_ANY, EFL_UI_EVENT_ITEM_CLICKED_ANY);
   REDIRECT_EVT(EFL_UI_EVENT_CLICKED, EFL_UI_EVENT_ITEM_CLICKED);
#undef REDIRECT_EVT
}

EFL_CALLBACKS_ARRAY_DEFINE(active_item,
  {EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _hints_changed_cb},
  {EFL_UI_EVENT_ITEM_SELECTED, _selected_cb},
  {EFL_UI_EVENT_ITEM_UNSELECTED, _unselected_cb},
  {EFL_UI_EVENT_PRESSED, _redirect_cb},
  {EFL_UI_EVENT_UNPRESSED, _redirect_cb},
  {EFL_UI_EVENT_LONGPRESSED, _redirect_cb},
  {EFL_UI_EVENT_CLICKED, _redirect_cb},
  {EFL_UI_EVENT_CLICKED_ANY, _redirect_cb},
  {EFL_EVENT_INVALIDATE, _invalidate_cb},
)

static Eina_Bool
register_item(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item)
{
   //We do not have items yet :/
   //EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(item, EFL_UI_ITEM_CLASS), EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!eina_list_data_find(pd->items, item), EINA_FALSE);

   if (!efl_ui_widget_sub_object_add(obj, item))
     return EINA_FALSE;


   efl_canvas_group_member_add(pd->pan, item);
   efl_event_callback_array_add(item, active_item(), obj);

   return EINA_TRUE;
}

static Eina_Bool
unregister_item(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item *item)
{
   if (!efl_ui_widget_sub_object_del(obj, item))
     return EINA_FALSE;

   int id = eina_list_data_idx(pd->items, item);
   pd->items = eina_list_remove(pd->items, item);
   pd->selected = eina_list_remove(pd->selected, item);
   efl_event_callback_array_del(item, active_item(), obj);
   efl_ui_item_position_manager_item_removed(pd->pos_man, id, item);

   return EINA_TRUE;
}

static void
update_pos_man(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   int id = eina_list_data_idx(pd->items, subobj);
   efl_ui_item_position_manager_item_added(pd->pos_man, id, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_pack_clear(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
   while(pd->items)
     {
        efl_ui_item_position_manager_item_removed(pd->pos_man, 0, pd->items->data);
        efl_del(pd->items->data);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_unpack_all(Eo *obj, Efl_Ui_Item_Container_Data *pd)
{
   while(pd->items)
     {
        if (!unregister_item(obj, pd, pd->items->data))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_item_container_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Item_Container_Data *pd, int index)
{
   Efl_Ui_Item *it = eina_list_nth(pd->items, index_adjust(pd, index));

   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);

   if (!unregister_item(obj, pd, it))
     return NULL;

   return it;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_unpack(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   return unregister_item(obj, pd, subobj);
}


EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_pack(Eo *obj, Efl_Ui_Item_Container_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   return efl_pack_end(obj, subobj);
}


EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_append(pd->items, subobj);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}


EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_prepend(pd->items, subobj);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_before(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   Eina_List *subobj_list = eina_list_data_find_list(pd->items, existing);
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj_list, EINA_FALSE);

   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_prepend_relative_list(pd->items, subobj, subobj_list);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_after(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   Eina_List *subobj_list = eina_list_data_find_list(pd->items, existing);
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj_list, EINA_FALSE);

   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   pd->items = eina_list_append_relative_list(pd->items, subobj, subobj_list);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_item_container_efl_pack_linear_pack_at(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   Eina_List *subobj_list = eina_list_data_find_list(pd->items, subobj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj_list, EINA_FALSE);
   int clamp = clamp_index(pd, index);

   index = index_adjust(pd, index);
   if (!register_item(obj, pd, subobj))
     return EINA_FALSE;
   if (clamp == 0)
     pd->items = eina_list_prepend_relative(pd->items, subobj, subobj_list);
   else if (clamp == 1)
     pd->items = eina_list_append(pd->items, subobj);
   else
     pd->items = eina_list_prepend(pd->items, subobj);
   update_pos_man(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static int
_efl_ui_item_container_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(pd->items, (void*)subobj);
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_item_container_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd, int index)
{
   return eina_list_nth(pd->items, index_adjust(pd, index));
}

static void
_pos_content_size_changed_cb(void *data, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;
   MY_DATA_GET(data, pd);

   efl_gfx_entity_size_set(pd->sizer, *size);
}

EOLIAN static void
_efl_ui_item_container_layouter_set(Eo *obj, Efl_Ui_Item_Container_Data *pd, Efl_Ui_Item_Position_Manager *layouter)
{
   if (pd->pos_man)
     {
        efl_event_callback_del(pd->pos_man, EFL_UI_ITEM_POSITION_MANAGER_EVENT_CONTENT_SIZE_CHANGED, _pos_content_size_changed_cb, obj);
        efl_ui_item_position_manager_data_access_set(pd->pos_man, NULL, NULL, 0);
     }
   pd->pos_man = layouter;
   if (pd->pos_man)
     {
        efl_event_callback_add(pd->pos_man, EFL_UI_ITEM_POSITION_MANAGER_EVENT_CONTENT_SIZE_CHANGED, _pos_content_size_changed_cb, obj);
        efl_ui_item_position_manager_data_access_set(pd->pos_man, &pd->obj_accessor.pass_on, &pd->size_accessor, eina_list_count(pd->items));
        efl_ui_item_position_manager_viewport_set(pd->pos_man, efl_ui_scrollable_viewport_geometry_get(obj));
     }
}

EOLIAN static Efl_Ui_Item_Position_Manager*
_efl_ui_item_container_layouter_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Container_Data *pd)
{
  return pd->pos_man;
}

#include "efl_ui_item_container.eo.c"
