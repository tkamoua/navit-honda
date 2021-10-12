#include <glib.h>
#include <stdlib.h>
#include "color.h"
#include "coord.h"
#include "point.h"
#include "callback.h"
#include "graphics.h"
#include "debug.h"
#include "navit.h"
#include "navit_nls.h"
#include "bookmarks.h"
#include "loadfile.h"
#include "gui_internal.h"
#include "gui_internal_widget.h"
#include "gui_internal_priv.h"
#include "gui_internal_menu.h"
#include "gui_internal_keyboard.h"


// Todo: Modify this function to load the route from file. 
// I think widghet -> text contains the user entered text, need to replace bookmarks_add_bookmark
// call with my own function that loads a route fro ma file.
static void gui_internal_cmd_load_route_do(struct gui_priv *this, struct widget *widget) {
    GList *l;
    struct attr attr;
    printf("Widget text: %s\n", widget->text);
    dbg(lvl_debug,"text='%s'", widget->text);
    if (widget->text && strlen(widget->text)) {
        navit_get_attr(this->nav, attr_route, &attr, NULL);
       loadfile_load_route(this->nav, &widget->c, widget->text);
    }
    g_free(widget->text);
    widget->text=NULL;
    l=g_list_previous(g_list_last(this->root.children));
    gui_internal_prune_menu(this, l->data);
}

static void gui_internal_cmd_load_route_clicked(struct gui_priv *this, struct widget *widget, void *data) {
    gui_internal_cmd_load_route_do(this, widget->data);
}

void gui_internal_cmd_add_route_folder2(struct gui_priv *this, struct widget *wm, void *data) {
    struct widget *w,*wb,*wk,*wl,*we,*wnext;
    char *name=data;
    wb=gui_internal_menu(this,_("Load route from file ... "));
    w=gui_internal_box_new(this, gravity_left_top|orientation_vertical|flags_expand|flags_fill);
    gui_internal_widget_append(wb, w);
    we=gui_internal_box_new(this, gravity_left_center|orientation_horizontal|flags_fill);
    gui_internal_widget_append(w, we);
    gui_internal_widget_append(we, wk=gui_internal_label_new(this, name));
    wk->state |= STATE_EDIT|STATE_EDITABLE|STATE_CLEAR;
    wk->background=this->background;
    wk->flags |= flags_expand|flags_fill;
    wk->func = gui_internal_call_linked_on_finish;
    wk->c=wm->c;
    gui_internal_widget_append(we, wnext=gui_internal_image_new(this, image_new_xs(this, "gui_active")));
    wnext->state |= STATE_SENSITIVE;
    wnext->func = gui_internal_cmd_load_route_clicked;
    wnext->data=wk;
    wk->data=wnext;
    wl=gui_internal_box_new(this, gravity_left_top|orientation_vertical|flags_expand|flags_fill);
    gui_internal_widget_append(w, wl);
    if (this->keyboard)
        gui_internal_widget_append(w, gui_internal_keyboard(this,
                                   VKBD_FLAG_2 | gui_internal_keyboard_init_mode(getenv("LANG"))));
    else
        gui_internal_keyboard_show_native(this, w, VKBD_FLAG_2 | gui_internal_keyboard_init_mode(getenv("LANG")),
                                          getenv("LANG"));
    gui_internal_menu_render(this);
}
