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

#define DEST_SIZE 100
int loadfile_load_route(struct navit *nav, struct pcoord *c, const char *description);
int gui_internal_coordinate_parse(char *s, char plus, char minus, double *x) {
    int sign=0;
    char *degree, *minute, *second;
    double tmp;

    if(!s)
        return 0;

    if (strchr(s, minus)!=NULL)
        sign=-1;
    else if (strchr(s, plus)!=NULL)
        sign=1;

    if(!sign)
        return 0;


    /* Can't just use strtok here because ° is multibyte sequence in utf8 */
    degree=s;
    minute=strstr(s,"°");
    if(minute) {
        *minute=0;
        minute+=strlen("°");
    }

    sscanf(degree, "%lf", x);

    if(strchr(degree, plus) || strchr(degree, minus)) {
        dbg(lvl_debug,"degree %c/%c found",plus,minus);
    } else {/* DEGREES_MINUTES */
        if(!minute)
            return 0;
        minute = strtok(minute,"'");
        sscanf(minute, "%lf", &tmp);
        *x+=tmp/60;
        if(strchr(minute, plus) || strchr(minute, minus)) {
            dbg(lvl_debug,"minute %c/%c found",plus,minus);
        } else { /* DEGREES_MINUTES_SECONDS */
            second=strtok(NULL,"");
            if(!second)
                return 0;
            sscanf(second, "%lf", &tmp);
            *x+=tmp/3600;
        }
    }
    *x *= sign;
    return 1;
}

//TODO: Load route in correct oder (should call visit_before starting from last element of file)
// Probably easier to do this as a preprocessing step in python 
// TODO: need to lower number of gps points or make routing algorithm more efficient, currently super slow
// TODO: This function should execute "stop navigation" funcationality at start to get rid of any currently displayed routes
// so we cna display the new route from the file 
// Note: all route files need to be placed into "navit-build/navit/routes" folder

int loadfile_load_route(struct navit *nav, struct pcoord *c, const char *description){
    //$NAVIT_SHAREDIR/routes/
    //Read route from file into route object, which should just be list of lat/lon pairs 
    printf("Loadfile function! \n");
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    char dest[DEST_SIZE] = "routes/";
    strcat(dest,description);
    FILE *in_file  = fopen(dest, "r"); // read only 
    if (in_file == NULL) 
            {   
              printf("Error! Could not open file\n"); 
            } 
    
    int count = 0; 
    while ((read = getline(&line, &len, in_file)) != -1) {
        
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
        char *lat=strtok(line," ");
        char *lng=strtok(NULL,"");
        double latitude, longitude;
        printf("lat2, lon2: %s, %s\n", lat, lng);
        
        if( gui_internal_coordinate_parse(lat, 'N', 'S', &latitude)
            && gui_internal_coordinate_parse(lng, 'E', 'W', &longitude) ) {
        char *widgettext=g_strdup_printf("%lf %lf", longitude, latitude);
        pcoord_parse(widgettext, projection_mg, c );
        if (count == 0){
            popup_set_position(nav, c);
        }
        else{
            popup_set_visitbefore(nav,c,0);
        }
        count+=1;
        
    } 
    }
    fclose(in_file);
    //call visit before on each point
   
   printf("popup_set_visitbefore\n");
   return 1;
}

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
