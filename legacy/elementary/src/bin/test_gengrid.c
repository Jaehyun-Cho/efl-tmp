#include <Elementary_Cursor.h>
#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

Evas_Object * _focus_autoscroll_mode_frame_create(Evas_Object *parent);

static Elm_Gengrid_Item_Class *gic, *ggic;

Evas_Object *grid_content_get(void *data, Evas_Object *obj, const char *part);
char *grid_text_get(void *data, Evas_Object *obj EINA_UNUSED,
                    const char *part EINA_UNUSED);
Eina_Bool grid_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                         const char *part EINA_UNUSED);
void grid_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED);

typedef struct _Item_Data
{
   Elm_Object_Item *item;
   const char *path;
   int mode;
   int onoff;
} Item_Data;

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   Evas_Object *box;           /* Use this to get box content     */
   Evas_Object *grid;
   Elm_Gengrid_Item_Field_Type field_type;
};
typedef struct _api_data api_data;

struct _Show_Data
{
   int winmode;
   int scrollto;
};
typedef struct _Show_Data Show_Data;

enum _api_state
{
   GRID_ALIGN_SET,
   GRID_BRING_IN,
   GRID_NO_SELECT_MODE,
   GRID_NO_BOUNCE,
   GRID_PAGE_RELATIVE,
   GRID_PAGE_SIZE,
   GRID_TOOLTIP_SET_TEXT,
   GRID_TOOLTIP_UNSET,
   GRID_ITEM_CLASS_SET,
   GRID_ITEM_UPDATE_SET,
   GRID_PAGE_BRING_IN,
   GRID_PAGE_SHOW,
   GRID_TOOLTIP_CONTENT_CB,
   GRID_TOOLTIP_STYLE_SET,
   GRID_TOOLTIP_WINDOW_MODE_SET,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   Evas_Object *grid = api->grid;

   /* use elm_box_children_get() to get list of children */
   switch(api->state)
     { /* Put all api-changes under switch */
      case GRID_ALIGN_SET: /* 0 */
         elm_gengrid_align_set(grid, 0.2, 0.8);
         break;

      case GRID_BRING_IN: /* 1 */
         elm_gengrid_item_bring_in(elm_gengrid_first_item_get(grid), ELM_GENGRID_ITEM_SCROLLTO_IN);
         break;

      case GRID_NO_SELECT_MODE: /* 2 */
         elm_gengrid_select_mode_set(grid, ELM_OBJECT_SELECT_MODE_NONE);
         break;

      case GRID_NO_BOUNCE: /* 3 */
         elm_scroller_bounce_set(grid, EINA_TRUE, EINA_FALSE);
         break;

      case GRID_PAGE_RELATIVE: /* 4 */
         elm_scroller_bounce_set(grid, EINA_TRUE, EINA_TRUE);
         elm_scroller_page_relative_set(grid, 0.5, 0.5);
         break;

      case GRID_PAGE_SIZE: /* 5 */
         elm_scroller_page_size_set(grid, 50, 25);
         break;

      case GRID_TOOLTIP_SET_TEXT: /* 6 */
           {
              Elm_Object_Item *item = elm_gengrid_first_item_get(grid);
              elm_gengrid_item_tooltip_text_set(item, "This is the first item");
           }
         break;

      case GRID_TOOLTIP_UNSET: /* 7 */
           {
              Elm_Object_Item *item = elm_gengrid_first_item_get(grid);
              elm_gengrid_item_tooltip_unset(item);
           }
         break;

      case API_STATE_LAST:
         break;

      default:
         return;
     }
}

static void
_api_bt_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{  /* Will add here a SWITCH command containing code to modify test-object */
   /* in accordance a->state value. */
   api_data *a = data;
   char str[128];

   printf("clicked event on API Button: api_state=<%d>\n", a->state);
   set_api_state(a);
   a->state++;
   sprintf(str, "Next API function (%u)", a->state);
   elm_object_text_set(obj, str);
   elm_object_disabled_set(obj, a->state == API_STATE_LAST);
}

static const char *img[9] =
{
   "panel_01.jpg",
   "plant_01.jpg",
   "rock_01.jpg",
   "rock_02.jpg",
   "sky_01.jpg",
   "sky_02.jpg",
   "sky_03.jpg",
   "sky_04.jpg",
   "wood_01.jpg",
};

static const char *cur[4] =
{
   ELM_CURSOR_CIRCLE,
   ELM_CURSOR_CLOCK,
   ELM_CURSOR_COFFEE_MUG,
   ELM_CURSOR_CROSS,
};

static int n_current_pic = 0;
static void
_horizontal_grid(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *grid = data;
   elm_gengrid_horizontal_set(grid, elm_check_state_get(obj));
}

static void
_item_loop_enable_changed_cb(void *data, Evas_Object *obj,
                             void *event_info  EINA_UNUSED)
{
   Evas_Object *grid = data;
   elm_object_scroll_item_loop_enabled_set(grid, elm_check_state_get(obj));
}

static void
_focus_highlight_changed_cb(void *data, Evas_Object *obj,
                            void *event_info EINA_UNUSED)
{
   elm_win_focus_highlight_enabled_set(data, elm_check_state_get(obj));
}

static void
grid_drag_up(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag up: %p\n", event_info);
}

static void
grid_drag_right(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag right: %p\n", event_info);
}

static void
grid_drag_down(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag down: %p\n", event_info);
}

static void
grid_drag_left(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag left: %p\n", event_info);
}

static void
grid_drag_stop(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Drag stop: %p\n", event_info);
}

static void
grid_selected(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Selected: %p\n", event_info);
}

static void
grid_unselected(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Unselected: %p\n", event_info);
}

static void
grid_double_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Double clicked: %p\n", event_info);
}

static void
grid_right_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Right clicked: %p\n", event_info);
}

static void
grid_longpressed(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("longpressed %p\n", event_info);
}

static void
grid_pressed(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("pressed %p\n", event_info);
}

static void
grid_released(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("released %p\n", event_info);
}

static void
grid_moved(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("moved %p\n", event_info);
}

static void
grid_item_check_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Item_Data *id = data;
   id->onoff = elm_check_state_get(obj);
   printf("item %p onoff = %i\n", id, id->onoff);
}

char *
grid_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   const Item_Data *id = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Photo %s", id->path);
   return strdup(buf);
}

Evas_Object *
grid_content_get(void *data, Evas_Object *obj, const char *part)
{
   const Item_Data *id = data;
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *image = elm_image_add(obj);
        elm_image_file_set(image, id->path, NULL);
        elm_image_aspect_fixed_set(image, EINA_FALSE);
        evas_object_show(image);
        return image;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *ck = elm_check_add(obj);
        evas_object_propagate_events_set(ck, EINA_FALSE);
        elm_check_state_set(ck, id->onoff);
        evas_object_smart_callback_add(ck, "changed", grid_item_check_changed, data);
        evas_object_show(ck);
        return ck;
     }
   return NULL;
}

Eina_Bool
grid_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

void
grid_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   free(data);
}

static void
grid_sel(void *data, Evas_Object *obj, void *event_info)
{
   printf("sel item data [%p] on grid obj [%p], pointer [%p], position [%d]\n", data, obj, event_info, elm_gengrid_item_index_get(event_info));
}

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   free(data);
}

static void
reorder_mode_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_gengrid_reorder_mode_set(data, elm_check_state_get(obj));
}

static void
always_select_mode_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     elm_gengrid_select_mode_set(data, ELM_OBJECT_SELECT_MODE_ALWAYS);
   else
     elm_gengrid_select_mode_set(data, ELM_OBJECT_SELECT_MODE_DEFAULT);
}

static void
multi_select_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_gengrid_multi_select_set(data, elm_check_state_get(obj));
}

static void
wheel_disable_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_gengrid_wheel_disabled_set(data, elm_check_state_get(obj));
}

static void
clear_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   api_data *api = data;
   elm_gengrid_clear(api->grid);
}

static Evas_Object *
create_gengrid(Evas_Object *obj, int items)
{
   Evas_Object *grid = NULL;
   Item_Data *id = NULL;
   int i, n;
   char buf[PATH_MAX];

   grid = elm_gengrid_add(obj);
   elm_gengrid_item_size_set(grid, ELM_SCALE_SIZE(200), ELM_SCALE_SIZE(150));
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_smart_callback_add(grid, "unselected", grid_unselected, NULL);
   evas_object_smart_callback_add(grid, "clicked,double", grid_double_clicked, NULL);
   evas_object_smart_callback_add(grid, "clicked,right", grid_right_clicked, NULL);
   evas_object_smart_callback_add(grid, "longpressed", grid_longpressed, NULL);
   evas_object_smart_callback_add(grid, "pressed", grid_pressed, NULL);
   evas_object_smart_callback_add(grid, "released", grid_released, NULL);
   evas_object_smart_callback_add(grid, "moved", grid_moved, NULL);
   evas_object_smart_callback_add(grid, "drag,start,up", grid_drag_up, NULL);
   evas_object_smart_callback_add(grid, "drag,start,right", grid_drag_right, NULL);
   evas_object_smart_callback_add(grid, "drag,start,down", grid_drag_down, NULL);
   evas_object_smart_callback_add(grid, "drag,start,left", grid_drag_left, NULL);
   evas_object_smart_callback_add(grid, "drag,stop", grid_drag_stop, NULL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = grid_del;

   n = 0;
   for (i = 0; i < items; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id = calloc(1, sizeof(Item_Data));
        id->mode = i;
        id->path = eina_stringshare_add(buf);
        id->item = elm_gengrid_item_append(grid, gic, id, grid_sel, NULL);
        if (!(i % 5))
          elm_gengrid_item_selected_set(id->item, EINA_TRUE);
     }
   elm_gengrid_item_class_free(gic);

   return grid;
}

static void
restore_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   api_data *api = data;
   elm_box_clear(api->box);
   api->grid = create_gengrid(obj, (12 * 12));
   elm_box_pack_end(api->box, api->grid);
   evas_object_show(api->grid);
}

static void
filled_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *box = (Evas_Object *)data;
   Evas_Object *grid;

   elm_box_clear(box);
   grid = create_gengrid(box, 1);
   elm_gengrid_filled_set(grid, elm_check_state_get(obj));
   elm_box_pack_end(box, grid);
   evas_object_show(grid);
}

static void
filled_bt_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *content_box, *grid, *tg;

   win = elm_win_util_standard_add("test filled", "Test Filled");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   content_box = elm_box_add(win);
   evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, content_box);
   elm_box_pack_end(box, content_box);
   evas_object_show(content_box);

   grid = create_gengrid(win, 1);
   elm_box_pack_end(content_box, grid);
   evas_object_show(grid);

   tg = elm_check_add(win);
   elm_object_text_set(tg, "Filled");
   evas_object_smart_callback_add(tg, "changed", filled_cb, content_box);
   elm_box_pack_end(box, tg);
   evas_object_show(tg);

   evas_object_resize(win, 450, 200);
   evas_object_show(win);
}

static void
cursor_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *grid = (Evas_Object *)data;
   Elm_Object_Item *item = NULL;
   int i = 0;

   for ((item = elm_gengrid_first_item_get(grid)); item;
        (item = elm_gengrid_item_next_get(item)))
     {
        if (!elm_check_state_get(obj))
          {
             elm_gengrid_item_cursor_unset(item);
             continue;
          }

        elm_gengrid_item_cursor_set(item, cur[i]);
        elm_gengrid_item_cursor_engine_only_set(item, EINA_FALSE);
        i++;
     }
}

static void
cursor_bt_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *content_box, *hbox, *grid, *tg;

   win = elm_win_util_standard_add("test cursor", "Test Cursor");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   content_box = elm_box_add(win);
   evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, content_box);
   elm_box_pack_end(box, content_box);
   evas_object_show(content_box);

   grid = create_gengrid(win, 4);
   elm_box_pack_end(content_box, grid);
   evas_object_show(grid);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);

   tg = elm_check_add(win);
   elm_object_text_set(tg, "Cursor");
   evas_object_smart_callback_add(tg, "changed", cursor_cb, grid);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   evas_object_resize(win, 450, 450);
   evas_object_show(win);
}

static void
_btn_bring_in_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!data) return;
   Elm_Object_Item *it = elm_gengrid_selected_item_get(data);
   if (!it) return;
   elm_gengrid_item_bring_in(it, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

static void
_btn_show_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!data) return;
   Elm_Object_Item *it = elm_gengrid_selected_item_get(data);
   if (!it) return;
   elm_gengrid_item_show(it, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

void
test_gengrid(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bxx, *bx, *ck;
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("gengrid", "GenGrid");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   api->box = bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *)api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   /* Create GenGrid */
   api->grid = create_gengrid(win, (12 * 12));
   elm_box_pack_end(bxx, api->grid);
   evas_object_show(api->grid);

   /* Gengrid Options 1 */
   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_box_pack_end(bxx, bx);
   evas_object_show(bx);

   ck = elm_check_add(win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(ck, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(ck, "Reorder mode enable");
   evas_object_smart_callback_add(ck, "changed", reorder_mode_cb,
                                  api->grid);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(ck, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(ck, "Always Select Mode");
   evas_object_smart_callback_add(ck, "changed", always_select_mode_cb,
                                  api->grid);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(ck, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(ck, "Multi Select Mode");
   elm_check_state_set(ck, EINA_TRUE);
   elm_gengrid_multi_select_set(api->grid, EINA_TRUE);
   evas_object_smart_callback_add(ck, "changed", multi_select_cb,
                                  api->grid);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(ck, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(ck, "Wheel Disable");
   evas_object_smart_callback_add(ck, "changed", wheel_disable_cb,
                                  api->grid);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   /* Gengrid Options 2 */
   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_box_pack_end(bxx, bx);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", clear_bt_clicked,
                                  (void *)api);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Restore");
   evas_object_smart_callback_add(bt, "clicked", restore_bt_clicked,
                                  (void *) api);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Check Filled");
   evas_object_smart_callback_add(bt, "clicked", filled_bt_clicked, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Check Cursor");
   evas_object_smart_callback_add(bt, "clicked", cursor_bt_clicked,
                                  (void *) api);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bring in");
   evas_object_smart_callback_add(bt, "clicked", _btn_bring_in_clicked_cb, api->grid);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Show");
   evas_object_smart_callback_add(bt, "clicked", _btn_show_clicked_cb, api->grid);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

static void
_before_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   Elm_Object_Item *sel;
   char buf[PATH_MAX];

   sel = elm_gengrid_selected_item_get(grid);
   if (!sel)
       return;
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_insert_before(grid, gic, id, sel, grid_sel,
                                             NULL);
}

static int
compare_cb(const void *data1, const void *data2)
{
   Item_Data *ti1 = elm_object_item_data_get(data1);
   Item_Data *ti2 = elm_object_item_data_get(data2);
   return strlen(ti1->path) - strlen(ti2->path);
}

static void
_sorted_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_sorted_insert(grid, gic, id, compare_cb, grid_sel,
                                             NULL);
}

static void
_after_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   Elm_Object_Item *sel;
   char buf[PATH_MAX];

   sel = elm_gengrid_selected_item_get(grid);
   if (!sel)
       return;
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_insert_after(grid, gic, id, sel, grid_sel,
                                            NULL);
}

static void
_delete_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *grid = data;
   Eina_List *l, *l2, *l3;
   Elm_Object_Item *gg_it;

   l = (Eina_List*)elm_gengrid_selected_items_get(grid);
   if (!l) return;
   EINA_LIST_FOREACH_SAFE(l, l2, l3, gg_it)
     elm_object_item_del(gg_it);
}

static void
_prepend_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_prepend(grid, gic, id, grid_sel, NULL);
   if (id->item) elm_gengrid_item_show(id->item, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

static void
_append_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Data *id;
   Evas_Object *grid = data;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n_current_pic]);
   n_current_pic = ((n_current_pic +1) % 9);
   id = calloc(1, sizeof(*id));
   id->mode = 0;
   id->path = eina_stringshare_add(buf);
   id->item = elm_gengrid_item_append(grid, gic, id, grid_sel, NULL);
   if (id->item) elm_gengrid_item_show(id->item, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

static void
_size_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *grid = data;
   int size = elm_spinner_value_get(obj);
   elm_gengrid_item_size_set(grid, ELM_SCALE_SIZE(size), ELM_SCALE_SIZE(size));
}

void
test_gengrid2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid, *bx, *hbx, *bt, *ck;

   win = elm_win_util_standard_add("gengrid2", "GenGrid 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, ELM_SCALE_SIZE(150), ELM_SCALE_SIZE(150));
   elm_gengrid_multi_select_set(grid, EINA_FALSE);
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(grid, 600, 500);
   elm_box_pack_end(bx, grid);
   evas_object_show(grid);

   hbx = elm_box_add(win);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append");
   evas_object_smart_callback_add(bt, "clicked", _append_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Prepend");
   evas_object_smart_callback_add(bt, "clicked", _prepend_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Insert before");
   evas_object_smart_callback_add(bt, "clicked", _before_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Insert after");
   evas_object_smart_callback_add(bt, "clicked", _after_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Sorted insert");
   evas_object_smart_callback_add(bt, "clicked", _sorted_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Delete");
   evas_object_smart_callback_add(bt, "clicked", _delete_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_spinner_add(win);
   elm_spinner_min_max_set(bt, 10, 1024);
   elm_spinner_value_set(bt, 150);
   elm_spinner_label_format_set(bt, "Item size: %.0f");
   evas_object_smart_callback_add(bt, "changed", _size_changed, grid);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Reorder mode enable");
   evas_object_smart_callback_add(ck, "changed", reorder_mode_cb, grid);
   elm_box_pack_end(hbx, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Horizontal Mode");
   evas_object_smart_callback_add(ck, "changed", _horizontal_grid, grid);
   elm_box_pack_end(hbx, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Item loop enable");
   evas_object_smart_callback_add(ck, "changed", _item_loop_enable_changed_cb, grid);
   elm_box_pack_end(hbx, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Focus Highlight Set");
   evas_object_smart_callback_add(ck, "changed", _focus_highlight_changed_cb, win);
   elm_box_pack_end(hbx, ck);
   evas_object_show(ck);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = grid_del;

   /* item_class_ref is needed for gic. some items can be added in callbacks */
   elm_gengrid_item_class_ref(gic);
   elm_gengrid_item_class_free(gic);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

void
test_gengrid3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid;
   static Item_Data id[144];
   int i, n;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("gengrid_group", "GenGrid Group");
   elm_win_autodel_set(win, EINA_TRUE);

   grid = elm_gengrid_add(win);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   elm_gengrid_item_size_set(grid, ELM_SCALE_SIZE(150), ELM_SCALE_SIZE(150));
   elm_gengrid_group_item_size_set(grid, ELM_SCALE_SIZE(31), ELM_SCALE_SIZE(31));
   elm_gengrid_multi_select_set(grid, EINA_TRUE);
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_smart_callback_add(grid, "clicked,double", grid_double_clicked, NULL);
   evas_object_smart_callback_add(grid, "longpressed", grid_longpressed, NULL);
   evas_object_smart_callback_add(grid, "moved", grid_moved, NULL);
   evas_object_smart_callback_add(grid, "drag,start,up", grid_drag_up, NULL);
   evas_object_smart_callback_add(grid, "drag,start,right", grid_drag_right, NULL);
   evas_object_smart_callback_add(grid, "drag,start,down", grid_drag_down, NULL);
   evas_object_smart_callback_add(grid, "drag,start,left", grid_drag_left, NULL);
   evas_object_smart_callback_add(grid, "drag,stop", grid_drag_stop, NULL);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = NULL;

   ggic = elm_gengrid_item_class_new();
   ggic->item_style = "group_index";
   ggic->func.text_get = grid_text_get;
   ggic->func.content_get = NULL;
   ggic->func.state_get = NULL;
   ggic->func.del = NULL;

   n = 0;
   for (i = 0; i < 12 * 12; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id[i].mode = i;
        id[i].path = eina_stringshare_add(buf);
        if (i == 0 || i == 18 || i == 53 || i == 100)
          //if (i == 0 || i == 18)
          id[i].item = elm_gengrid_item_append(grid, ggic, &(id[i]), grid_sel, NULL);
        else
          id[i].item = elm_gengrid_item_append(grid, gic, &(id[i]), grid_sel, NULL);
        if (!(i % 5))
          elm_gengrid_item_selected_set(id[i].item, EINA_TRUE);
     }
   elm_gengrid_item_class_free(gic);
   elm_gengrid_item_class_free(ggic);

   evas_object_show(grid);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

/* test gengrid item styles */

static Evas_Object *
_gengrid_create(Evas_Object *obj, int items, const char *style)
{
   static Evas_Object *grid = NULL;
   Elm_Gengrid_Item_Class *ic;
   Item_Data *id;
   int i, n;
   char buf[PATH_MAX];

   if (grid)
     elm_gengrid_clear(grid);
   else
     {
        if (!obj) return NULL;
        grid = elm_gengrid_add(obj);
        elm_gengrid_item_size_set(grid, ELM_SCALE_SIZE(150), ELM_SCALE_SIZE(150));
        evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
     }

   ic = elm_gengrid_item_class_new();
   if (style) ic->item_style = style;
   else ic->item_style = "default";
   ic->func.text_get = grid_text_get;
   ic->func.content_get = grid_content_get;
   ic->func.state_get = NULL;
   ic->func.del = grid_del;

   n = 0;
   for (i = 0; i < items; i++)
     {
        id = calloc(1, sizeof(Item_Data));
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id->mode = i;
        id->path = eina_stringshare_add(buf);
        id->item = elm_gengrid_item_append(grid, ic, id, NULL, NULL);
     }
   elm_gengrid_item_class_free(ic);

   return grid;
}

static void
_item_style_sel_cb(void *data, Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   _gengrid_create(NULL, (12*12), data);
}

static Evas_Object *
_item_styles_list_create(Evas_Object *parent)
{
   Evas_Object *list;

   list = elm_list_add(parent);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_list_item_append(list, "default", NULL, NULL,
                        _item_style_sel_cb, "default");
   elm_list_item_append(list, "default_style", NULL, NULL,
                        _item_style_sel_cb, "default_style");
   elm_list_item_append(list, "up", NULL, NULL,
                        _item_style_sel_cb, "up");
   elm_list_item_append(list, "album-preview", NULL, NULL,
                        _item_style_sel_cb, "album-preview");
   elm_list_item_append(list, "thumb", NULL, NULL,
                        _item_style_sel_cb, "thumb");
   elm_list_go(list);

   return list;
}

/* Set elementary widget's min size.
 * We should not set min size hint to elementary widgets because elementary has
 * its own size policy/logic. This is an official trick from raster.
 * @param obj the actual object that you want to set the min size
 * @param parent parent object for elm_table_add
 * @param w min width
 * @param h min height
 */
Evas_Object *
_elm_min_set(Evas_Object *obj, Evas_Object *parent, Evas_Coord w, Evas_Coord h)
{
   Evas_Object *table, *rect;

   table = elm_table_add(parent);

   rect = evas_object_rectangle_add(evas_object_evas_get(table));
   evas_object_size_hint_min_set(rect, w, h);
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(table, rect, 0, 0, 1, 1);

   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(table, obj, 0, 0, 1, 1);

   return table;
}

void
test_gengrid_item_styles(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *gengrid, *list, *table;

   win = elm_win_util_standard_add("gengrid-styles", "Gengrid Item Styles");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   list = _item_styles_list_create(box);
   evas_object_show(list);

   table = _elm_min_set(list, box, 100, 0);
   evas_object_size_hint_weight_set(table, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, table);
   evas_object_show(table);

   gengrid = _gengrid_create(win, (12 * 12), NULL);
   elm_box_pack_end(box, gengrid);
   evas_object_show(gengrid);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

static void
_rd1_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Show_Data *sd = data;
   sd->winmode = elm_radio_state_value_get(obj);
}

static void
_rd2_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Show_Data *sd = data;
   sd->scrollto = elm_radio_state_value_get(obj);
}

static void
_bring_in_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid;
   static Item_Data ti[5000];
   int i, n;
   char buf[PATH_MAX];

   if (!data) return;

   Show_Data *sd = data;

   if (sd->winmode == 0)
     win = elm_win_util_standard_add("horz bring_in", "Horz Bring_in");
   else
     win = elm_win_util_standard_add("vertical bring_in", "Vertical Bring_in");
   elm_win_autodel_set(win, EINA_TRUE);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 150, 150);
   elm_gengrid_group_item_size_set(grid, 31, 31);
   if (sd->winmode == 0)
     elm_gengrid_horizontal_set(grid, EINA_TRUE);
   else if (sd->winmode == 1)
     elm_gengrid_horizontal_set(grid, EINA_FALSE);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   elm_gengrid_align_set(grid, 0.1, 0.1);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = NULL;

   n = 0;
   for (i = 0; i < 5000; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        ti[i].mode = i;
        ti[i].path = eina_stringshare_add(buf);
        ti[i].item = elm_gengrid_item_append(grid, gic, &(ti[i]), grid_sel, NULL);
        if (i == 1430)
          elm_gengrid_item_selected_set(ti[i].item, EINA_TRUE);
     }

   elm_gengrid_item_class_free(gic);
   elm_gengrid_item_bring_in(ti[1430].item, sd->scrollto);
   evas_object_show(grid);

   if (sd->winmode == 0)
     evas_object_resize(win, 600, 200);
   if (sd->winmode == 1)
     evas_object_resize(win, 600, 400);
   evas_object_show(win);
}

static void
_show_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *grid;
   static Item_Data ti[10000];
   int i, n;
   char buf[PATH_MAX];

   if (!data) return;

   Show_Data *sd = data;

   if (sd->winmode == 0)
     win = elm_win_util_standard_add("horz show", "Horz Show");
   else
     win = elm_win_util_standard_add("vertical show", "Vertical Show");
   elm_win_autodel_set(win, EINA_TRUE);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, ELM_SCALE_SIZE(150), ELM_SCALE_SIZE(150));
   elm_gengrid_group_item_size_set(grid, ELM_SCALE_SIZE(31), ELM_SCALE_SIZE(31));
   if (sd->winmode == 0)
     elm_gengrid_horizontal_set(grid, EINA_TRUE);
   else if (sd->winmode == 1)
     elm_gengrid_horizontal_set(grid, EINA_FALSE);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   elm_gengrid_align_set(grid, 0.1, 0.1);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = grid_text_get;
   gic->func.content_get = grid_content_get;
   gic->func.state_get = grid_state_get;
   gic->func.del = NULL;

   n = 0;
   for (i = 0; i < 10000; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        ti[i].mode = i;
        ti[i].path = eina_stringshare_add(buf);
        ti[i].item = elm_gengrid_item_append(grid, gic, &(ti[i]), grid_sel, NULL);
        if (i == 2579)
          elm_gengrid_item_selected_set(ti[i].item, EINA_TRUE);
     }

   elm_gengrid_item_class_free(gic);
   evas_object_show(grid);
   elm_gengrid_item_show(ti[2579].item, sd->scrollto);

   if (sd->winmode == 0)
     evas_object_resize(win, 600, 200);
   if (sd->winmode == 1)
     evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

static Evas_Object *
_window_mode_frame_new(Evas_Object *win, void *data)
{
   Evas_Object *fr, *bx, *rd, *rdg;
   Show_Data *sd = data;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(fr, "Direction");
   evas_object_show(fr);

   bx = elm_box_add(win);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_radio_state_value_set(rd, 0);
   elm_object_text_set(rd, "horizontal");
   evas_object_smart_callback_add(rd, "changed", _rd1_changed_cb, sd);
   evas_object_show(rd);
   elm_box_pack_end(bx, rd);
   rdg = rd;

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_radio_state_value_set(rd, 1);
   elm_object_text_set(rd, "vertical");
   elm_radio_group_add(rd, rdg);
   evas_object_smart_callback_add(rd, "changed", _rd1_changed_cb, sd);
   evas_object_show(rd);
   elm_box_pack_end(bx, rd);

   return fr;
}

static Evas_Object *
_scrollto_mode_frame_new(Evas_Object *win, void *data)
{
   Evas_Object *fr, *bx, *rd, *rdg;
   Show_Data *sd = data;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(fr, "Scrollto Mode");
   evas_object_show(fr);

   bx = elm_box_add(win);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_radio_state_value_set(rd, 0);
   elm_object_text_set(rd, "SCROLLTO_NONE");
   evas_object_smart_callback_add(rd, "changed", _rd2_changed_cb, sd);
   evas_object_show(rd);
   elm_box_pack_end(bx, rd);
   rdg = rd;

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_radio_state_value_set(rd, 1);
   elm_object_text_set(rd, "SCROLLTO_IN");
   elm_radio_group_add(rd, rdg);
   evas_object_smart_callback_add(rd, "changed", _rd2_changed_cb, sd);
   evas_object_show(rd);
   elm_box_pack_end(bx, rd);

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_radio_state_value_set(rd, 2);
   elm_object_text_set(rd, "SCROLLTO_TOP");
   elm_radio_group_add(rd, rdg);
   evas_object_smart_callback_add(rd, "changed", _rd2_changed_cb, sd);
   evas_object_show(rd);
   elm_box_pack_end(bx, rd);

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_radio_state_value_set(rd, 4);
   elm_object_text_set(rd, "SCROLLTO_MIDDLE");
   elm_radio_group_add(rd, rdg);
   evas_object_smart_callback_add(rd, "changed", _rd2_changed_cb, sd);
   evas_object_show(rd);
   elm_box_pack_end(bx, rd);

   return fr;
}

void
test_gengrid4(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bx, *bx2, *fr;
   Show_Data *sd = calloc(1, sizeof(Show_Data));

   win = elm_win_util_standard_add("gengrid-show-bringin", "GenGrid Show/Bring_in");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, sd);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bx2 = elm_box_add(bx);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   fr = _window_mode_frame_new(bx2, sd);
   elm_box_pack_end(bx2, fr);

   fr = _scrollto_mode_frame_new(bx2, sd);
   elm_box_pack_end(bx2, fr);

   bx2 = elm_box_add(bx);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Show");
   evas_object_smart_callback_add(bt, "clicked", _show_clicked_cb, sd);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Bring In");
   evas_object_smart_callback_add(bt, "clicked", _bring_in_clicked_cb, sd);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_show(win);
}

typedef struct _grid5_Event_Data grid5_Event_Data;
struct _grid5_Event_Data
{
   Evas_Object *grid_obj;
   Evas_Object *en_obj;
   Elm_Object_Item *last_item_found;
};

static const char *_grid5_items_text[] = {
   "Albany", "Annapolis",
   "Atlanta", "Augusta",
   "Austin", "Baton Rouge",
   "Bismarck", "Boise",
   "Boston", "Carson City",
   "Charleston", "Cheyenne",
   "Columbia", "Columbus",
   "Concord", "Denver",
   "Des Moines", "Dover",
   "Frankfort", "Harrisburg",
   "Hartford", "Helena",
   "Honolulu", "Indianapolis",
   "Jackson", "Jefferson City",
   "Juneau", "Lansing",
   "Lincoln", "Little Rock",
   "Madison", "Montgomery",
   "Montpelier", "Nashville",
   "Oklahoma City", "Olympia",
   "Phoenix", "Pierre",
   "Providence", "Raleigh",
   "Richmond", "Sacramento",
   "Saint Paul", "Salem",
   "Salt Lake City", "Santa Fe",
   "Springfield", "Tallahassee",
   "Topeka", "Trenton"
};

static char *
_grid5_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   Item_Data *id = data;
   char buf[64];
   snprintf(buf, sizeof(buf), "%s", _grid5_items_text[id->mode]);
   return strdup(buf);
}

static void
_grid5_search_item(grid5_Event_Data *event_data, Elm_Object_Item * it)
{
   const char *str = elm_entry_entry_get(event_data->en_obj);
   if (!str || !strlen(str)) return;

   printf("Looking for \"%s\". ", str);
   event_data->last_item_found = elm_gengrid_search_by_text_item_get
   (event_data->grid_obj, it, NULL, str, 0);

   if (event_data->last_item_found)
     {
        printf("Found.\n");
        elm_gengrid_item_selected_set(event_data->last_item_found, EINA_TRUE);
        elm_gengrid_item_bring_in(event_data->last_item_found, ELM_GENGRID_ITEM_SCROLLTO_MIDDLE);
        elm_object_focus_set(event_data->en_obj, EINA_TRUE);
     }
   else
     printf("Not Found.\n");
}

static void
_grid5_search_settings_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *einfo EINA_UNUSED)
{
   _grid5_search_item(data, NULL);
}

static void
_grid5_on_keydown(void *data, Evas *evas EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   grid5_Event_Data * event_data = (grid5_Event_Data *)data;

   if (!strcmp(ev->key, "Return"))
     {
        printf("Looking for next item\n");
        _grid5_search_item(data, event_data->last_item_found);
     }
}

static Elm_Gengrid_Item_Class *
_grid5_item_class_create(const char *item_style)
{
   Elm_Gengrid_Item_Class * itc = elm_gengrid_item_class_new();
   itc->item_style = item_style;
   itc->func.text_get = _grid5_text_get;
   itc->func.content_get = grid_content_get;
   itc->func.state_get = grid_state_get;
   itc->func.del = NULL;
   return itc;
}

void
test_gengrid5(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *grid;
   Evas_Object *fr, *lb_frame;
   Evas_Object *en, *bx_entry, *lb_entry;
   grid5_Event_Data *event_data;
   Elm_Gengrid_Item_Class *itc = NULL;
   static Item_Data id[50];
   char buf[PATH_MAX];
   int i, n;

   win = elm_win_util_standard_add("gengrid-item-search-by-text", "Gengrid Item Search By Text");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fr = elm_frame_add(bx);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_object_text_set(fr, "Information");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   lb_frame = elm_label_add(fr);
   elm_object_text_set(lb_frame,
   "<align=left>This is an example of elm_gengrid_search_by_text_item_get() usage.<br>"
   "Enter search string to the entry and press Enter to show next search result.<br>");
   elm_object_content_set(fr, lb_frame);
   evas_object_show(lb_frame);

   bx_entry = elm_box_add(win);
   elm_box_horizontal_set(bx_entry, EINA_TRUE);
   evas_object_size_hint_weight_set(bx_entry, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx_entry, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bx, bx_entry);
   evas_object_show(bx_entry);

   lb_entry = elm_label_add(win);
   elm_object_text_set(lb_entry, " Search:");
   evas_object_size_hint_weight_set(lb_entry, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(lb_entry, 0.0, EVAS_HINT_FILL);
   elm_box_pack_end(bx_entry, lb_entry);
   evas_object_show(lb_entry);

   en = elm_entry_add(win);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_object_part_text_set(en, "guide", "Type item's name here to search.");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx_entry, en);
   evas_object_show(en);
   elm_object_focus_set(en, EINA_TRUE);

   grid = elm_gengrid_add(bx);
   elm_gengrid_item_size_set(grid, ELM_SCALE_SIZE(200), ELM_SCALE_SIZE(150));
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_gengrid_select_mode_set(grid, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_box_pack_end(bx, grid);
   evas_object_show(grid);

   event_data = calloc(1, sizeof(grid5_Event_Data));
   event_data->en_obj = en;
   event_data->grid_obj = grid;
   event_data->last_item_found = NULL;

   evas_object_event_callback_add(en, EVAS_CALLBACK_KEY_DOWN, _grid5_on_keydown, (void*)event_data);
   evas_object_smart_callback_add(en, "changed,user", _grid5_search_settings_changed_cb, (void*)event_data);
   evas_object_event_callback_add(grid, EVAS_CALLBACK_FREE, _cleanup_cb, (void*)event_data);

   itc = _grid5_item_class_create("default");

   for (i = 0, n = 0; i < 50; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id[i].mode = i;
        id[i].path = eina_stringshare_add(buf);
        id[i].item = elm_gengrid_item_append(grid, itc, &(id[i]), grid_sel, NULL);
        if (!(i % 5))
          elm_gengrid_item_selected_set(id[i].item, EINA_TRUE);
     }
   elm_gengrid_item_class_free(itc);

   evas_object_resize(win, 320, 500);
   evas_object_show(win);
}

void
test_gengrid_speed(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fr, *bx;
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("gengrid", "Gengrid");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   api->box = bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.5);
   elm_frame_autocollapse_set(fr, EINA_TRUE);
   elm_object_text_set(fr, "Collapse me!");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   api->grid = create_gengrid(win, 5000);
   evas_object_size_hint_min_set(api->grid, 600, 600);
   elm_gengrid_item_size_set(api->grid,
                             ELM_SCALE_SIZE(30),
                             ELM_SCALE_SIZE(36));
   elm_object_content_set(fr, api->grid);
   evas_object_show(api->grid);
   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

static void
_gengrid_focus_item_cb(void *data, Evas_Object *obj EINA_UNUSED,
                       void *event_info)
{
   printf("%s: %p\n", (char *)data, event_info);
}

static void
_gengrid_focus_key_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   printf("\n=== Key Down : %s ===\n", ev->keyname);
}

static void
_gg_focus_focus_move_policy_changed_cb(void *data EINA_UNUSED,
                                       Evas_Object *obj,
                                       void *event_info EINA_UNUSED)
{
   int val = elm_radio_value_get(obj);

   if (val == 0)
     elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_CLICK);
   else if (val == 1)
     elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_IN);
}

static void
_gg_focus_focus_highlight_changed_cb(void *data,
                                     Evas_Object *obj,
                                     void *event_info EINA_UNUSED)
{
   elm_win_focus_highlight_enabled_set((Evas_Object *)data,
                                       elm_check_state_get(obj));
}

static void
_gg_focus_focus_animate_changed_cb(void *data,
                                   Evas_Object *obj,
                                   void *event_info EINA_UNUSED)
{
   elm_win_focus_highlight_animate_set((Evas_Object *)data,
                                       elm_check_state_get(obj));
}

static void
_grid_reorder_mode(void *data, Evas_Object *obj,
                   void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     elm_gengrid_reorder_mode_start((Evas_Object *)data,
                                    ECORE_POS_MAP_LINEAR);
   else
     elm_gengrid_reorder_mode_stop((Evas_Object *)data);
}

static void
_gg_focus_item_select_on_focus_disable_changed_cb(void *data EINA_UNUSED,
                                                  Evas_Object *obj,
                                                  void *event_info
                                                  EINA_UNUSED)
{
   elm_config_item_select_on_focus_disabled_set(elm_check_state_get(obj));
}

static void
_gg_first_item_focus_on_first_focus_in_cb(void *data EINA_UNUSED, Evas_Object *obj,
                                          void *event_info  EINA_UNUSED)
{
   elm_config_first_item_focus_on_first_focusin_set(elm_check_state_get(obj));
}

void
test_gengrid_focus(void *data EINA_UNUSED,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bx_horiz, *gengrid, *btn, *fr, *bx_mv, *bx_opt, *ck, *rdg, *rd;
   Elm_Gengrid_Item_Class *ic;
   Item_Data *id;
   char buf[PATH_MAX];
   int i, n;

   win = elm_win_util_standard_add("gengrid-focus", "Gengrid Focus");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   bx_horiz = elm_box_add(win);
   elm_box_horizontal_set(bx_horiz, EINA_TRUE);
   evas_object_size_hint_weight_set(bx_horiz, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx_horiz);
   evas_object_show(bx_horiz);

   btn = elm_button_add(bx_horiz);
   elm_object_text_set(btn, "Left");
   elm_box_pack_end(bx_horiz, btn);
   evas_object_show(btn);

   bx = elm_box_add(bx_horiz);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx_horiz, bx);
   evas_object_show(bx);

   btn = elm_button_add(bx);
   elm_object_text_set(btn, "Up");
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);
   elm_object_focus_set(btn, EINA_TRUE);

   gengrid = elm_gengrid_add(bx);
   elm_gengrid_item_size_set(gengrid,
                             ELM_SCALE_SIZE(150),
                             ELM_SCALE_SIZE(150));
   evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, gengrid);
   evas_object_show(gengrid);
   evas_object_smart_callback_add(gengrid, "item,focused", _gengrid_focus_item_cb, "item,focused");
   evas_object_smart_callback_add(gengrid, "item,unfocused", _gengrid_focus_item_cb, "item,unfocused");
   evas_object_smart_callback_add(gengrid, "selected", _gengrid_focus_item_cb, "selected");
   evas_object_smart_callback_add(gengrid, "unselected", _gengrid_focus_item_cb, "unselected");
   evas_object_smart_callback_add(gengrid, "activated", _gengrid_focus_item_cb, "activated");
   evas_object_smart_callback_add(gengrid, "highlighted", _gengrid_focus_item_cb, "highlighted");
   evas_object_smart_callback_add(gengrid, "unhighlighted", _gengrid_focus_item_cb, "unhighlighted");
   evas_object_event_callback_add(gengrid, EVAS_CALLBACK_KEY_DOWN, _gengrid_focus_key_down_cb, NULL);

   btn = elm_button_add(bx);
   elm_object_text_set(btn, "Down");
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   btn = elm_button_add(bx_horiz);
   elm_object_text_set(btn, "Right");
   elm_box_pack_end(bx_horiz, btn);
   evas_object_show(btn);

   //Options
   fr = elm_frame_add(bx);
   elm_object_text_set(fr, "Options");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx_opt = elm_box_add(fr);
   elm_box_horizontal_set(bx_opt, EINA_TRUE);
   elm_object_content_set(fr, bx_opt);
   evas_object_show(bx_opt);

   ck = elm_check_add(bx_opt);
   elm_object_text_set(ck, "Focus Highlight");
   elm_check_state_set(ck, EINA_TRUE);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(ck, "changed",
                                  _gg_focus_focus_highlight_changed_cb,
                                  win);
   elm_box_pack_end(bx_opt, ck);
   evas_object_show(ck);

   ck = elm_check_add(bx_opt);
   elm_object_text_set(ck, "Focus Animation");
   elm_check_state_set(ck, EINA_TRUE);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(ck, "changed",
                                  _gg_focus_focus_animate_changed_cb,
                                  win);
   elm_box_pack_end(bx_opt, ck);
   evas_object_show(ck);

   ck = elm_check_add(bx_opt);
   elm_object_text_set(ck, "Horizontal Mode");
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(ck, "changed", _horizontal_grid, gengrid);
   elm_box_pack_end(bx_opt, ck);
   evas_object_show(ck);

   ck = elm_check_add(bx_opt);
   elm_object_text_set(ck, "Rorder mode enable");
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(ck, "changed", _grid_reorder_mode, gengrid);
   elm_box_pack_end(bx_opt, ck);
   evas_object_show(ck);

   ck = elm_check_add(bx_opt);
   elm_object_text_set(ck, "Item Select on Focus disable");
   elm_check_state_set(ck, elm_config_item_select_on_focus_disabled_get());
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(ck, "changed",
                                  _gg_focus_item_select_on_focus_disable_changed_cb,
                                  NULL);
   elm_box_pack_end(bx_opt, ck);
   evas_object_show(ck);

   ck = elm_check_add(bx_opt);
   elm_object_text_set(ck, "First item focus on first focus in");
   elm_check_state_set(ck, elm_config_first_item_focus_on_first_focusin_get());
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(ck, "changed",
                                  _gg_first_item_focus_on_first_focus_in_cb,
                                  NULL);
   elm_box_pack_end(bx_opt, ck);
   evas_object_show(ck);

   // Focus Autoscroll Mode
   fr = _focus_autoscroll_mode_frame_create(bx);
   elm_box_pack_end(bx, fr);

   //Focus movement policy
   fr = elm_frame_add(bx);
   elm_object_text_set(fr, "Focus Movement Policy");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx_mv = elm_box_add(fr);
   elm_box_horizontal_set(bx_mv, EINA_TRUE);
   elm_object_content_set(fr, bx_mv);
   evas_object_show(bx_mv);

   rdg = rd = elm_radio_add(bx_mv);
   elm_object_text_set(rd, "Focus Move by Click");
   elm_radio_state_value_set(rd, 0);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(rd, "changed",
                                  _gg_focus_focus_move_policy_changed_cb,
                                  NULL);
   elm_box_pack_end(bx_mv, rd);
   evas_object_show(rd);

   rd = elm_radio_add(bx_mv);
   elm_object_text_set(rd, "Focus Move by Mouse-In");
   elm_radio_group_add(rd, rdg);
   elm_radio_state_value_set(rd, 1);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(rd, "changed",
                                  _gg_focus_focus_move_policy_changed_cb,
                                  NULL);
   elm_box_pack_end(bx_mv, rd);
   evas_object_show(rd);

   ic = elm_gengrid_item_class_new();
   ic->item_style = "default";
   ic->func.text_get = grid_text_get;
   ic->func.content_get = grid_content_get;
   ic->func.state_get = NULL;
   ic->func.del = grid_del;

   n = 0;
   for (i = 0; i < 24; i++)
     {
        id = calloc(1, sizeof(Item_Data));
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        id->mode = i;
        id->path = eina_stringshare_add(buf);
        id->item = elm_gengrid_item_append(gengrid, ic, id, NULL, NULL);
        if (i == 4)
          elm_object_item_disabled_set(id->item, EINA_TRUE);
     }
   elm_gengrid_item_class_free(ic);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

char *
_gg_update_text_get(void *data EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED,
                    const char *part EINA_UNUSED)
{
   char *txt[] = {"Sky", "Stone", "Water", "Flower", "Sand", "Sun", "Moon", "Star", "Cloud", NULL};
   int n = rand() % 9;

   return strdup(txt[n]);
}

Evas_Object *
_gg_update_content_get(void *data, Evas_Object *obj, const char *part)
{
   const Item_Data *id = data;
   char buf[256];
   int n = 0;

   if (!strcmp(part, "elm.swallow.icon"))
     {
        n = rand() % 9;

        Evas_Object *image = elm_image_add(obj);
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        elm_image_file_set(image, buf, NULL);
        elm_image_aspect_fixed_set(image, EINA_FALSE);
        evas_object_show(image);
        return image;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *ck = elm_check_add(obj);
        evas_object_propagate_events_set(ck, EINA_FALSE);
        elm_check_state_set(ck, id->onoff);
        evas_object_smart_callback_add(ck, "changed", grid_item_check_changed, data);
        evas_object_show(ck);
        return ck;
     }
   return NULL;
}

Eina_Bool
_gg_update_state_get(void *data EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_gg_item_update_clicked_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   api_data *api = (api_data *)data;

   Evas_Object *gengrid = api->grid;
   Eina_List *l = elm_gengrid_realized_items_get(gengrid);
   Elm_Object_Item *item = NULL;
   Item_Data *id = NULL;

   EINA_LIST_FREE(l, item)
     {
        id = elm_object_item_data_get(item);
        if (id && id->onoff) elm_gengrid_item_update(item);
     }
}

static void
_gg_item_fields_update_clicked_cb(void *data,
                                  Evas_Object *obj EINA_UNUSED,
                                  void *event_info EINA_UNUSED)
{
   api_data *api = (api_data *)data;

   Evas_Object *gengrid = api->grid;
   Eina_List *l = elm_gengrid_realized_items_get(gengrid);
   Elm_Object_Item *item = NULL;
   Item_Data *id = NULL;

   EINA_LIST_FREE(l, item)
     {
        id = elm_object_item_data_get(item);
        if (id && id->onoff)
          {
             elm_gengrid_item_fields_update(item, "*", api->field_type);
          }
     }
}

static void
_gg_text_update_changed_cb(void *data,
                           Evas_Object *obj,
                           void *event_info EINA_UNUSED)
{
   api_data *api = (api_data *)data;

   if (elm_check_state_get(obj))
     api->field_type |= ELM_GENGRID_ITEM_FIELD_TEXT;
   else
     api->field_type ^= ELM_GENGRID_ITEM_FIELD_TEXT;
}

static void
_gg_content_update_changed_cb(void *data,
                              Evas_Object *obj,
                              void *event_info EINA_UNUSED)
{
   api_data *api = (api_data *)data;

   if (elm_check_state_get(obj))
     api->field_type |= ELM_GENGRID_ITEM_FIELD_CONTENT;
   else
     api->field_type ^= ELM_GENGRID_ITEM_FIELD_CONTENT;
}

static void
_gg_state_update_changed_cb(void *data,
                            Evas_Object *obj,
                            void *event_info EINA_UNUSED)
{
   api_data *api = (api_data *)data;

   if (elm_check_state_get(obj))
     api->field_type |= ELM_GENGRID_ITEM_FIELD_STATE;
   else
     api->field_type ^= ELM_GENGRID_ITEM_FIELD_STATE;
}

void
test_gengrid_update(void *data EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *gengrid, *btn, *fr, *bx_opt, *ck;
   Elm_Gengrid_Item_Class *ic;
   api_data *api = calloc(1, sizeof(api_data));
   Item_Data *id;
   int i;

   win = elm_win_util_standard_add("gengrid-update", "Gengrid Update");
   elm_win_autodel_set(win, EINA_TRUE);

   api->box = bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   api->grid = gengrid = elm_gengrid_add(bx);
   elm_gengrid_item_size_set(gengrid,
                             ELM_SCALE_SIZE(150),
                             ELM_SCALE_SIZE(150));
   evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, gengrid);
   evas_object_show(gengrid);
   evas_object_smart_callback_add(gengrid, "item,focused", _gengrid_focus_item_cb, "item,focused");
   evas_object_smart_callback_add(gengrid, "item,unfocused", _gengrid_focus_item_cb, "item,unfocused");
   evas_object_smart_callback_add(gengrid, "selected", _gengrid_focus_item_cb, "selected");
   evas_object_smart_callback_add(gengrid, "unselected", _gengrid_focus_item_cb, "unselected");
   evas_object_smart_callback_add(gengrid, "activated", _gengrid_focus_item_cb, "activated");
   evas_object_smart_callback_add(gengrid, "highlighted", _gengrid_focus_item_cb, "highlighted");
   evas_object_smart_callback_add(gengrid, "unhighlighted", _gengrid_focus_item_cb, "unhighlighted");
   evas_object_event_callback_add(gengrid, EVAS_CALLBACK_KEY_DOWN, _gengrid_focus_key_down_cb, NULL);

   //initialize field type
   api->field_type = ELM_GENGRID_ITEM_FIELD_ALL;

   //Field Options
   fr = elm_frame_add(bx);
   elm_object_text_set(fr, "Field Options");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx_opt = elm_box_add(fr);
   elm_box_horizontal_set(bx_opt, EINA_TRUE);
   elm_object_content_set(fr, bx_opt);
   evas_object_show(bx_opt);

   ck = elm_check_add(bx_opt);
   elm_object_text_set(ck, "TEXT");
   elm_check_state_set(ck, EINA_FALSE);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(ck, "changed",
                                  _gg_text_update_changed_cb,
                                  api);
   elm_box_pack_end(bx_opt, ck);
   evas_object_show(ck);

   ck = elm_check_add(bx_opt);
   elm_object_text_set(ck, "CONTENT");
   elm_check_state_set(ck, EINA_FALSE);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(ck, "changed",
                                  _gg_content_update_changed_cb,
                                  api);
   elm_box_pack_end(bx_opt, ck);
   evas_object_show(ck);

   ck = elm_check_add(bx_opt);
   elm_object_text_set(ck, "STATE");
   elm_check_state_set(ck, EINA_FALSE);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(ck, "changed",
                                  _gg_state_update_changed_cb,
                                  api);
   elm_box_pack_end(bx_opt, ck);
   evas_object_show(ck);

   //Update Buttons
   fr = elm_frame_add(bx);
   elm_object_text_set(fr, "Update Options");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx_opt = elm_box_add(fr);
   elm_box_horizontal_set(bx_opt, EINA_TRUE);
   elm_object_content_set(fr, bx_opt);
   evas_object_show(bx_opt);

   btn = elm_button_add(bx_opt);
   elm_object_text_set(btn, "Update");
   evas_object_smart_callback_add(btn, "clicked", _gg_item_update_clicked_cb, api);
   elm_box_pack_end(bx_opt, btn);
   evas_object_show(btn);

   btn = elm_button_add(bx_opt);
   elm_object_text_set(btn, "Fields Update");
   evas_object_smart_callback_add(btn, "clicked", _gg_item_fields_update_clicked_cb, api);
   elm_box_pack_end(bx_opt, btn);
   evas_object_show(btn);


   //Gengrid Item Append
   ic = elm_gengrid_item_class_new();
   ic->item_style = "default";
   ic->func.text_get = _gg_update_text_get;
   ic->func.content_get = _gg_update_content_get;
   ic->func.state_get = _gg_update_state_get;
   ic->func.del = grid_del;

   for (i = 0; i < 24; i++)
     {
        id = calloc(1, sizeof(Item_Data));
        id->mode = i;
        id->item = elm_gengrid_item_append(gengrid, ic, id, NULL, NULL);
        if (i == 4)
          elm_object_item_disabled_set(id->item, EINA_TRUE);
     }
   elm_gengrid_item_class_free(ic);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}
