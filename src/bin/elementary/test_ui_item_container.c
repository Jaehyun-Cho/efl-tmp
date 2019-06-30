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

static void
_scroll_to_animated_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Widget *element_1154 = efl_key_data_get(ev->object, "__to_element");

   EINA_SAFETY_ON_NULL_RETURN(element_1154);

   efl_ui_item_container_item_scroll(data, element_1154, EINA_TRUE);
}

static void
_scroll_to_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Widget *element_10 = efl_key_data_get(ev->object, "__to_element");

   EINA_SAFETY_ON_NULL_RETURN(element_10);

   efl_ui_item_container_item_scroll(data, element_10, EINA_FALSE);
}

static void
_change_min_size_cb(void *data, const Efl_Event *ev)
{
   static Eina_Bool b = EINA_FALSE;
   Efl_Ui_Widget *element_0 = efl_key_data_get(ev->object, "__to_element");

   EINA_SAFETY_ON_NULL_RETURN(element_0);

   if (b)
     {
        b = EINA_FALSE;
        efl_gfx_hint_size_min_set(element_0, EINA_SIZE2D(40, 200));
     }
   else
     {
        b = EINA_TRUE;
        efl_gfx_hint_size_min_set(element_0, EINA_SIZE2D(40, 40));
     }
}

void test_efl_ui_item_container(void *data EINA_UNUSED,
                                   Evas_Object *obj EINA_UNUSED,
                                   void *event_info EINA_UNUSED)
{
   Efl_Ui_Win *win, *o, *tbl, *item_container, *element_1154, *element_10, *element_0;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Item_Container List"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   tbl = efl_add(EFL_UI_TABLE_CLASS, win);
   efl_content_set(win, tbl);

   Eo *list = efl_new(EFL_UI_LIST_POSITION_MANAGER_CLASS);
   item_container = o = efl_add(EFL_UI_ITEM_CONTAINER_CLASS, win,
                 efl_ui_item_container_layouter_set(efl_added, list));
   for (int i = 0; i < 2000; ++i)
     {
        char buf[PATH_MAX];
        Eo *il = efl_add(EFL_UI_BUTTON_CLASS, o);

        //FIXME wtf this is not required
        efl_ui_widget_focus_allow_set(il, EINA_FALSE);

        snprintf(buf, sizeof(buf), "%d - Test %d", i, i%13);
        efl_gfx_hint_size_min_set(il, EINA_SIZE2D(40, 40+(i%2)*40));
        efl_text_set(il, buf);
        efl_pack_end(o, il);
        if (i == 1154)
          element_1154 = il;
        if (i == 10)
          element_10 = il;
        if (i == 0)
          element_0 = il;
     }
   efl_pack_table(tbl, o, 1, 0, 1, 10);

   o = efl_add(EFL_UI_CHECK_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Vertical");
   efl_event_callback_add(o, EFL_UI_CHECK_EVENT_SELECTED_CHANGED, _selection_changed_cb, item_container);
   efl_ui_check_selected_set(o, EINA_TRUE);
   efl_pack_table(tbl, o, 0, 0, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Scroll to 1154 ANIMATED");
   efl_key_data_set(o, "__to_element", element_1154);
   efl_event_callback_add(o, EFL_UI_EVENT_CLICKED, _scroll_to_animated_cb, item_container);
   efl_pack_table(tbl, o, 0, 1, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Scroll to 1154");
   efl_key_data_set(o, "__to_element", element_10);
   efl_event_callback_add(o, EFL_UI_EVENT_CLICKED, _scroll_to_cb, item_container);
   efl_pack_table(tbl, o, 0, 2, 1, 1);

   o = efl_add(EFL_UI_BUTTON_CLASS, tbl,
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_gfx_hint_align_set(efl_added, 0, 0.5));
   efl_text_set(o, "Change min size of 0");
   efl_key_data_set(o, "__to_element", element_0);
   efl_event_callback_add(o, EFL_UI_EVENT_CLICKED, _change_min_size_cb, item_container);
   efl_pack_table(tbl, o, 0, 3, 1, 1);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(260, 200));
}

void test_efl_ui_item_container_update_speed(void *data EINA_UNUSED,
                                   Evas_Object *obj EINA_UNUSED,
                                   void *event_info EINA_UNUSED)
{
   Efl_Ui_Win *win, *o, *tbl;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Item_Container List"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   tbl = efl_add(EFL_UI_TABLE_CLASS, win);
   efl_content_set(win, tbl);

   Eo *list = efl_new(EFL_UI_LIST_POSITION_MANAGER_CLASS);
   o = efl_add(EFL_UI_ITEM_CONTAINER_CLASS, win,
                 efl_ui_item_container_layouter_set(efl_added, list),
                 efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   printf("Building 20'000 Elements this takes a while:\n");
   for (int i = 0; i < 20000; ++i)
     {
        Eo *il = efl_add(EFL_CANVAS_RECTANGLE_CLASS, o);
        double r = 10+((double)190/(double)10)*(i%10);
        efl_gfx_color_set(il, r, 10, 10, 255);
        efl_gfx_hint_size_min_set(il, EINA_SIZE2D(40, 40+(i%2)*40));
        efl_pack_end(o, il);
     }
   printf("DONE\n");
   efl_pack_table(tbl, o, 1, 0, 1, 10);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(260, 200));
}
