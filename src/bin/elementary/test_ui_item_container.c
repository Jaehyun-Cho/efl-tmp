#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>

static void
_selection_changed_cb(void *data, const Efl_Event *ev)
{
   if (efl_ui_check_selected_get(ev->object))
     efl_ui_layout_orientation_set(data, EFL_UI_LAYOUT_ORIENTATION_VERTICAL);
   else
     efl_ui_layout_orientation_set(data, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL);
}

void test_efl_ui_item_container(void *data EINA_UNUSED,
                                   Evas_Object *obj EINA_UNUSED,
                                   void *event_info EINA_UNUSED)
{
   Efl_Ui_Win *win, *o, *tbl, *item_container;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Item_Container List"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   tbl = efl_add(EFL_UI_TABLE_CLASS, win);
   efl_content_set(win, tbl);

   Eo *list = efl_new(EFL_UI_LIST_POSITION_MANAGER_CLASS);
   item_container = o = efl_add(EFL_UI_ITEM_CONTAINER_CLASS, win,
                 efl_ui_item_container_layouter_set(efl_added, list));
   for (int i = 0; i < 200; ++i)
     {
        Eo *il = efl_add(EFL_CANVAS_RECTANGLE_CLASS, o);
        double r = 10+((double)190/(double)10)*(i%10);
        efl_gfx_color_set(il, r, 10, 10, 255);
        efl_gfx_hint_size_min_set(il, EINA_SIZE2D(40, 40+(i%2)*40));
        efl_pack_end(o, il);
     }
   efl_pack_table(tbl, o, 1, 0, 1, 10);

   o = efl_add(EFL_UI_CHECK_CLASS, tbl);
   efl_text_set(o, "Vertical");
   efl_event_callback_add(o, EFL_UI_CHECK_EVENT_SELECTED_CHANGED, _selection_changed_cb, item_container);
   efl_ui_check_selected_set(o, EINA_TRUE);

   efl_pack_table(tbl, o, 0, 0, 1, 1);


   efl_gfx_entity_size_set(win, EINA_SIZE2D(260, 200));
}
