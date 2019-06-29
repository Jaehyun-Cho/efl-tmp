#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>

void test_efl_ui_item_container(void *data EINA_UNUSED,
                                   Evas_Object *obj EINA_UNUSED,
                                   void *event_info EINA_UNUSED)
{
   Efl_Ui_Win *win, *o;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Radio_Box"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   Eo *list = efl_new(EFL_UI_LIST_POSITION_MANAGER_CLASS);
   o = efl_add(EFL_UI_ITEM_CONTAINER_CLASS, win,
                 efl_ui_item_container_layouter_set(efl_added, list));
   efl_content_set(win, o);

   for (int i = 0; i < 200; ++i)
   {
      Eo *il = efl_add(EFL_CANVAS_RECTANGLE_CLASS, o);
      double r = 10+((double)190/(double)10)*(i%10);
      efl_gfx_color_set(il, r, 10, 10, 255);
      efl_gfx_hint_size_min_set(il, EINA_SIZE2D(40, 40+(i%2)*40));
      efl_pack_end(o, il);
   }

   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 200));
}
