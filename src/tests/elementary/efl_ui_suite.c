#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "efl_ui_suite.h"
#include "eo_internal.h"

static const Efl_Test_Case etc[] = {
  //{ "elm_focus", elm_test_focus},
  //{ "elm_focus_sub", elm_test_focus_sub},
  //{ "elm_widget_focus", elm_test_widget_focus},
  { "efl_ui_atspi", efl_ui_test_atspi},
  { "efl_ui_callback", efl_ui_test_callback},
  { "efl_ui_focus", efl_ui_test_focus},
  { "efl_ui_focus_sub", efl_ui_test_focus_sub},
  { "efl_ui_box", efl_ui_test_box},
  { "efl_ui_box_flow", efl_ui_test_box_flow},
  { "efl_ui_box_stack", efl_ui_test_box_stack},
  { "efl_ui_table", efl_ui_test_table},
  { "efl_ui_grid", efl_ui_test_grid},
  { "efl_ui_relative_layout", efl_ui_test_relative_layout},
  { "efl_ui_image", efl_ui_test_image},
  { "efl_ui_image_zoomable", efl_ui_test_image_zoomable},
  { "efl_ui_layout", efl_ui_test_layout},
  { "Efl_Ui_Model", efl_ui_model },
  { "efl_ui_widget", efl_ui_test_widget },
  { "efl_ui_active_view", efl_ui_test_active_view},
  { "efl_ui_check", efl_ui_test_check },
  { "efl_ui_radio_group", efl_ui_test_radio_group },
  { "efl_ui_win", efl_ui_test_win },
  { NULL, NULL }
};

EFL_CLASS_SIMPLE_CLASS(efl_ui_widget, "efl_ui_widget", EFL_UI_WIDGET_CLASS);

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   failed_count = suite_setup(EINA_FALSE);

   failed_count += _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Efl_Ui", etc, SUITE_INIT_FN(elm2), SUITE_SHUTDOWN_FN(elm));

   return (failed_count == 0) ? 0 : 255;
}
