#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif


#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#define MY_CLASS      EFL_UI_ITEM_CONTAINER_CLASS

typedef struct {
   Eina_Accessor *content_acc, *size_acc;
   int size;
   Eina_Rect viewport;
   Eina_Size2D abs_size;
   Eina_Vector2 scroll_position, align, padding;
   Efl_Ui_Layout_Orientation dir;
   int *size_cache;
   int average_item_size;
   struct {
      int start_id, end_id;
   } prev_run;
} Efl_Ui_List_Position_Manager_Data;

static void
cache_require(Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd)
{
   if (pd->size_cache) return;

   if (pd->size == 0)
     {
        pd->size_cache = NULL;
        pd->average_item_size = 0;
        return;
     }

   pd->size_cache = calloc(pd->size + 1, sizeof(int));
   pd->size_cache[0] = 0;
   for (int i = 0; i < pd->size; ++i)
     {
        Eina_Size2D size;

        eina_accessor_data_get(pd->size_acc, i, (void**) &size);
        pd->size_cache[i + 1] = pd->size_cache[i] + size.h;
     }
   pd->average_item_size = pd->size_cache[pd->size]/pd->size;
}

static void
cache_invalidate(Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd)
{
   if (pd->size_cache)
     free(pd->size_cache);
   pd->size_cache = NULL;
}

static void
recalc_absolut_size(Eo *obj, Efl_Ui_List_Position_Manager_Data *pd)
{
   cache_require(obj, pd);

   pd->abs_size.w = pd->viewport.w;
   pd->abs_size.h = pd->size ? pd->size_cache[pd->size] : pd->viewport.h;

   efl_event_callback_call(obj, EFL_UI_ITEM_POSITION_MANAGER_EVENT_CONTENT_SIZE_CHANGED, &pd->abs_size);
}

static inline void
vis_change_segment(Efl_Ui_List_Position_Manager_Data *pd, int a, int b, Eina_Bool flag)
{
   for (int i = MIN(a, b); i < MAX(a, b); ++i)
     {
        Efl_Gfx_Entity *ent;

        eina_accessor_data_get(pd->content_acc, i, (void**) &ent);
        efl_gfx_entity_visible_set(ent, flag);
     }
}

static void
position_content(Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd)
{
   Eina_Rect geom;
   Eina_Size2D space_size;
   int start_id = 0, end_id = 0;

   if (!pd->size) return;

   //space size contains the amount of space that is outside the viewport (either to the top or to the left)
   space_size.w = (MAX(pd->abs_size.w - pd->viewport.w, 0))*pd->scroll_position.x;
   space_size.h = (MAX(pd->abs_size.h - pd->viewport.h, 0))*pd->scroll_position.y;
   start_id = space_size.h / pd->average_item_size;

   for (; pd->size_cache[start_id] >= space_size.h && start_id > 0; start_id --) { }
   end_id = start_id;
   for (; end_id <= pd->size && pd->size_cache[end_id] <= space_size.h+pd->viewport.h ; end_id ++) { }
   end_id = MIN(end_id, pd->size);
   end_id = MAX(end_id, start_id + 1);

   //printf("space_size %d : starting point : %d : cached_space_starting_point %d end point : %d cache_space_end_point %d\n", space_size.h, start_id, pd->size_cache[start_id], end_id, pd->size_cache[end_id]);
   //assertion, by that time, pd->size_cache[start_id] must be smaller than space_size
   EINA_SAFETY_ON_FALSE_RETURN(pd->size_cache[start_id] <= space_size.h);
   EINA_SAFETY_ON_FALSE_RETURN(pd->size_cache[end_id] >= space_size.h + pd->viewport.h);
   EINA_SAFETY_ON_FALSE_RETURN(start_id < end_id);

   if (end_id < pd->prev_run.start_id)
     {
        vis_change_segment(pd, pd->prev_run.start_id, pd->prev_run.end_id, EINA_FALSE);
        vis_change_segment(pd, start_id, end_id, EINA_TRUE);
     }
   else
     {
        vis_change_segment(pd, pd->prev_run.start_id, start_id, (pd->prev_run.start_id > start_id));
        vis_change_segment(pd, pd->prev_run.end_id, end_id, (pd->prev_run.end_id < end_id));
     }

   geom.y = pd->viewport.y - (space_size.h - pd->size_cache[start_id]);
   geom.x = pd->viewport.x;
   geom.w = pd->viewport.w;

   for (int i = start_id; i < end_id; ++i)
     {
        Eina_Size2D size;
        Efl_Gfx_Entity *ent;

        eina_accessor_data_get(pd->size_acc, i, (void**) &size);
        eina_accessor_data_get(pd->content_acc, i, (void**) &ent);
        geom.h = size.h;
        if (ent)
          efl_gfx_entity_geometry_set(ent, geom);
        geom.y += size.h;
     }
   pd->prev_run.start_id = start_id;
   pd->prev_run.end_id = end_id;
}

EOLIAN static void
_efl_ui_list_position_manager_efl_ui_item_position_manager_data_access_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd, Eina_Accessor *content_access, Eina_Accessor *size_access, int size)
{
   pd->content_acc = content_access;
   pd->size_acc = size_access;
   pd->size = size;
   cache_invalidate(obj, pd);
}

EOLIAN static void
_efl_ui_list_position_manager_efl_ui_item_position_manager_viewport_set(Eo *obj, Efl_Ui_List_Position_Manager_Data *pd, Eina_Rect size)
{
   pd->viewport = size;

   recalc_absolut_size(obj, pd);
   position_content(obj, pd);
}

EOLIAN static void
_efl_ui_list_position_manager_efl_ui_item_position_manager_scroll_positon_set(Eo *obj, Efl_Ui_List_Position_Manager_Data *pd, double x, double y)
{
   pd->scroll_position.x = x;
   pd->scroll_position.y = y;
   position_content(obj, pd);
}

EOLIAN static void
_efl_ui_list_position_manager_efl_ui_item_position_manager_item_added(Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd, int added_index, Efl_Gfx_Entity *subobj)
{
   pd->size ++;
   efl_gfx_entity_visible_set(subobj, EINA_FALSE);
   cache_invalidate(obj, pd);
}

EOLIAN static void
_efl_ui_list_position_manager_efl_ui_item_position_manager_item_removed(Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd, int removed_index, Efl_Gfx_Entity *subobj)
{
   pd->size --;
   efl_gfx_entity_visible_set(subobj, EINA_TRUE);
   cache_invalidate(obj, pd);
}

EOLIAN static void
_efl_ui_list_position_manager_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd, Efl_Ui_Layout_Orientation dir)
{
   pd->dir = dir;
   recalc_absolut_size(obj, pd);
   position_content(obj, pd);
}

EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_list_position_manager_efl_ui_layout_orientable_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd)
{
   return pd->dir;
}

EOLIAN static void
_efl_ui_list_position_manager_efl_gfx_arrangement_content_align_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd, double align_horiz, double align_vert)
{
   pd->align.x = align_vert;
   pd->align.y = align_horiz;
}

EOLIAN static void
_efl_ui_list_position_manager_efl_gfx_arrangement_content_align_get(const Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd, double *align_horiz, double *align_vert)
{
   if (*align_vert)
     *align_vert = pd->align.x;
   if (*align_horiz)
     *align_horiz = pd->align.y;
}

EOLIAN static void
_efl_ui_list_position_manager_efl_gfx_arrangement_content_padding_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd, double pad_horiz, double pad_vert, Eina_Bool scalable)
{
   pd->padding.x = pad_vert;
   pd->padding.y = pad_horiz;
}

EOLIAN static void
_efl_ui_list_position_manager_efl_gfx_arrangement_content_padding_get(const Eo *obj EINA_UNUSED, Efl_Ui_List_Position_Manager_Data *pd, double *pad_horiz, double *pad_vert, Eina_Bool *scalable)
{
   if (*pad_vert)
     *pad_vert = pd->padding.x;
   if (*pad_horiz)
     *pad_horiz = pd->padding.y;
}

#include "efl_ui_list_position_manager.eo.c"
