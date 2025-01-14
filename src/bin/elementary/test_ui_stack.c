#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>

static void _third_layout_push(void *data, const Efl_Event *ev EINA_UNUSED);

static void
_stack_remove(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *stack = data;
   Eo *top_layout = efl_pack_content_get(stack, 0);
   efl_del(top_layout);
}

static void
_stack_pop(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *stack = data;
   efl_ui_active_view_pop(stack, EINA_TRUE);
}

static void
_stack_double_push(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *stack = data;
   _third_layout_push(stack, NULL);
}

static void
_stack_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *stack = data;
   Eo *top_layout = efl_pack_content_get(stack, 0);
   efl_del(top_layout);
}

static void
_win_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   efl_del(win);
}

static Eo *
_navigation_layout_create(Eo *stack, const char *text, Eo *content)
{
   Eo *nl = efl_add(EFL_UI_NAVIGATION_LAYOUT_CLASS, stack);

   Eo *bn = efl_add(EFL_UI_NAVIGATION_BAR_CLASS, nl);
   efl_text_set(bn, text);
   efl_gfx_entity_visible_set(efl_part(bn, "back_button"), EINA_TRUE);
   efl_ui_navigation_layout_bar_set(nl, bn);

   efl_content_set(nl, content);

   printf("Create content(%p).\n\n", nl);

   return nl;
}

static void
_bar_left_btn_set(Eo *navigation_layout, Efl_Event_Cb clicked_cb, void *data)
{
   Eo *bn = efl_ui_navigation_layout_bar_get(navigation_layout);

   Eo *left_btn = efl_add(EFL_UI_BUTTON_CLASS, bn);
   efl_text_set(left_btn, "Prev");
   efl_content_set(efl_part(bn, "left_content"), left_btn);

   efl_event_callback_add(left_btn, EFL_UI_EVENT_CLICKED, clicked_cb, data);

   //Positions of "left_content" and "back_button" are the same.
   efl_gfx_entity_visible_set(efl_part(bn, "back_button"), EINA_FALSE);
}

static void
_bar_right_btn_set(Eo *navigation_layout, Efl_Event_Cb clicked_cb, void *data)
{
   Eo *bn = efl_ui_navigation_layout_bar_get(navigation_layout);

   Eo *right_btn = efl_add(EFL_UI_BUTTON_CLASS, bn);
   efl_text_set(right_btn, "Next");
   efl_content_set(efl_part(bn, "right_content"), right_btn);

   efl_event_callback_add(right_btn, EFL_UI_EVENT_CLICKED, clicked_cb, data);
}

static void
_fifth_layout_insert(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *stack = data;

   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, stack);
   efl_text_set(btn, "Press to remove top layout");
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _stack_remove, stack);

   Eo *nl = _navigation_layout_create(stack, "5th layout", btn);

   efl_ui_active_view_push(stack, nl);
}

static void
_third_layout_push(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *stack = data;

   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, stack);
   efl_text_set(btn, "Press to pop");
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _stack_pop, stack);

   Eo *nl = _navigation_layout_create(stack, "3rd layout", btn);

   _bar_right_btn_set(nl, _fifth_layout_insert, stack);

   efl_ui_active_view_push(stack, nl);
}

static void
_second_layout_push(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *stack = data;

   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, stack);
   efl_text_set(btn, "Press to double push");
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _stack_double_push, stack);

   Eo *nl = _navigation_layout_create(stack, "2nd layout", btn);

   _bar_right_btn_set(nl, _third_layout_push, stack);

   efl_ui_active_view_push(stack, nl);
}

static void
_first_layout_push(Eo *win, Eo *stack)
{
   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, stack);
   efl_text_set(btn, "Press to delete stack");
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _stack_del, stack);

   Eo *nl = _navigation_layout_create(stack, "1st layout", btn);

   _bar_left_btn_set(nl, _win_del, win);
   _bar_right_btn_set(nl, _second_layout_push, stack);

   efl_ui_active_view_push(stack, nl);
}

void
test_ui_stack(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                     efl_text_set(efl_added, "Efl.Ui.Stack"),
                     efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 500));

   Eo *stack = efl_ui_active_view_util_stack_gen(win);

   efl_content_set(win, stack);

   _first_layout_push(win, stack);
}
