/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2008 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "coord.h"
#include "debug.h"
#include "item.h"
#include "map.h"
#include "transform.h"

struct item_name {
    enum item_type item;
    char *name;
};

struct item_range item_range_all = { type_none, type_last };

struct default_flags {
    enum item_type type;
    int flags;
};

struct item busy_item;

struct default_flags default_flags2[]= {
    {type_street_nopass, AF_PBH},
    {type_street_0, AF_ALL},
    {type_street_1_city, AF_ALL},
    {type_street_2_city, AF_ALL},
    {type_street_3_city, AF_ALL},
    {type_street_4_city, AF_ALL},
    {type_highway_city, AF_MOTORIZED_FAST},
    {type_street_1_land, AF_ALL},
    {type_street_2_land, AF_ALL},
    {type_street_3_land, AF_ALL},
    {type_street_4_land, AF_ALL},
    {type_street_n_lanes, AF_MOTORIZED_FAST},
    {type_highway_land, AF_MOTORIZED_FAST},
    {type_ramp, AF_MOTORIZED_FAST},
    {type_roundabout, AF_ALL},
    {type_ferry, AF_ALL},
    {type_cycleway, AF_PBH},
    {type_track_paved, AF_ALL},
    {type_track_gravelled, AF_ALL},
    {type_track_unpaved, AF_ALL},
    {type_track_ground, AF_ALL},
    {type_track_grass, AF_ALL},
    {type_footway, AF_PBH},
    {type_living_street, AF_ALL},
    {type_street_service, AF_ALL},
    {type_street_parking_lane, AF_ALL},
    {type_bridleway, AF_PBH},
    {type_path, AF_PBH},
    {type_steps, AF_PBH},
    {type_street_pedestrian, AF_PBH},
    {type_hiking_mountain, AF_PEDESTRIAN},
    {type_hiking_mountain_demanding, AF_PEDESTRIAN},
    {type_hiking, AF_PEDESTRIAN},
    {type_hiking_alpine, AF_PEDESTRIAN},
    {type_hiking_alpine_demanding, AF_PEDESTRIAN},
    {type_hiking_alpine_difficult, AF_PEDESTRIAN},
};



struct item_name item_names[]= {
#define ITEM2(x,y) ITEM(y)
#define ITEM(x) { type_##x, #x },
#include "item_def.h"
#undef ITEM2
#undef ITEM
};

static GHashTable *default_flags_hash;

static GHashTable *item_hash;

void item_create_hash(void) {
    int i;
    item_hash=g_hash_table_new(g_str_hash, g_str_equal);
    for (i=0 ; i < sizeof(item_names)/sizeof(struct item_name) ; i++) {
        g_hash_table_insert(item_hash, item_names[i].name, GINT_TO_POINTER(item_names[i].item));
    }
}

void item_destroy_hash(void) {
    g_hash_table_destroy(item_hash);
    item_hash=NULL;
}

int *item_get_default_flags(enum item_type type) {
    if (!default_flags_hash) {
        int i;
        default_flags_hash=g_hash_table_new(NULL, NULL);
        for (i = 0 ; i < sizeof(default_flags2)/sizeof(struct default_flags); i++) {
            g_hash_table_insert(default_flags_hash, (void *)(long)default_flags2[i].type, &default_flags2[i].flags);
        }
    }
    return g_hash_table_lookup(default_flags_hash, (void *)(long)type);
}

void item_cleanup(void) {
    if (default_flags_hash)
        g_hash_table_destroy(default_flags_hash);
}

/**
 * @brief Resets the "coordinate pointer" of an item
 *
 * This function resets the "coordinate pointer" of an item to point to the first coordinate pair,
 * so that at the next call to {@code item_coord_get()} the first coordinates will be returned.
 *
 * This function is not safe to call after destroying the item's map rect, and doing so may cause errors
 * with some map implementations.
 *
 * @param it The map item whose pointer is to be reset. This must be the active item, i.e. the last one retrieved from the
 * {@code map_rect}. There can only be one active item per {@code map_rect}.
 */
void item_coord_rewind(struct item *it) {
    it->meth->item_coord_rewind(it->priv_data);
}

/**
 * @brief Gets the next coordinates from an item
 *
 * This function returns a list of coordinates from an item and advances the "coordinate pointer"
 * by the number of coordinates returned, so that at the next call the next coordinates will be returned.
 *
 * Coordinates are stored in the projection of the item's map. If you need them in a different projection,
 * call {@code item_coord_get_pro()} instead.
 *
 * This function is not safe to call after destroying the item's map rect, and doing so may cause errors
 * with some map implementations.
 *
 * @param it The map item whose coordinates to retrieve. This must be the active item, i.e. the last one retrieved from the
 * {@code map_rect}. There can only be one active item per {@code map_rect}.
 * @param c Points to a buffer that will receive the coordinates.
 * The buffer must be at least {@code count * sizeof(struct coord)} bytes in size.
 * @param count The number of coordinates to retrieve. Attempts to read past the
 * end are handled gracefully and only the available number of coordinates is
 * returned.
 *
 * @return The number of coordinates actually retrieved and stored in {@code c}
 */
int item_coord_get(struct item *it, struct coord *c, int count) {
    return it->meth->item_coord_get(it->priv_data, c, count);
}

/**
 * @brief Gets the number of coordinates left for this item
 *
 * This function returnes the number of coordinates left to get for this item. It does not
 * change the "coordinates pointer".
 *
 * @param it The map item
 * @returns The number of coordinates left. -1 if not supported by the actual map.
 */
int item_coords_left(struct item * it) {
    if(!it->meth->item_coords_left) {
        return (-1);
    }
    return it->meth->item_coords_left(it->priv_data);
}

/**
 * @brief Sets coordinates of an item
 *
 * This function supports different modes:
 *
 * \li `change_mode_delete`: Deletes the specified number of coordinates
 * \li `change_mode_modify`: Replaces existing coordinates with new ones
 * \li `change_mode_append`: Appends new coordinates
 * \li `change_mode_prepend`: Prepends new coordinates
 *
 * TODO which coordinates are deleted/modified? Starting from the last coordinate retrieved, or the one after, or...?
 *
 * TODO what if `count` in delete or modify mode is bigger then the number of coordinates left?
 *
 * TODO where are coordinates appended/prepended? Beginning/end or current position?
 *
 * This function is not safe to call after destroying the item's map rect, and doing so may cause errors
 * with some map implementations.
 *
 * @param it The map item whose coordinates to retrieve. This must be the active item, i.e. the last one retrieved from the
 * {@code map_rect}. There can only be one active item per {@code map_rect}.
 * @param c TODO new coordinates, also store for old coordinates (delete/modify)? Required in delete mode?
 * @param count TODO number of coordinates to add, delete or modify?
 * @param mode The change mode, see description
 */
int item_coord_set(struct item *it, struct coord *c, int count, enum change_mode mode) {
    if (!it->meth->item_coord_set)
        return 0;
    return it->meth->item_coord_set(it->priv_data, c, count, mode);
}

/**
 * @brief Get coordinates within selection
 * This function returns the coordinates of an item if at least one coordinate of that item is inside the
 * selection. If the given buffer is too small, it returns the buffer size if at least one coordinate
 * intersects with the selection, otherwise it returnes 0
 *
 * If the return value equals count, the content of c is undefined.
 *
 * @param it requested item
 * @param c preallocated buffer for the result
 * @param count number of coordinates in c
 * @param sel current map selection
 * @return number of coordinates read to c, or count if out of space, or 0 if not intersecting, or <0 on read error.
 */
int item_coord_get_within_selection(struct item *it, struct coord *c, int count, struct map_selection *sel) {
    int in_coords;
    int ret = 0;
    struct coord_rect bbox;
    int intersects=0;

    /* scan all coordinates. Even if buffer is too small */
    while ((in_coords=item_coord_get(it, c, count)) > 0) {
        int i;
        struct map_selection *curr;
        /* update ret */
        if(ret == 0) {
            ret=in_coords;
            /* init bbox */
            bbox.lu=c[0];
            bbox.rl=c[0];
        }

        /* update bbox */
        for (i = 0 ; i < in_coords ; i++) {
            if (bbox.lu.x > c[i].x)
                bbox.lu.x=c[i].x;
            if (bbox.rl.x < c[i].x)
                bbox.rl.x=c[i].x;
            if (bbox.rl.y > c[i].y)
                bbox.rl.y=c[i].y;
            if (bbox.lu.y < c[i].y)
                bbox.lu.y=c[i].y;
        }
        /* check if bbox intersects already with selection */
        curr=sel;
        while ((!intersects) && (curr)) {
            struct coord_rect *sr=&curr->u.c_rect;
            if (bbox.lu.x <= sr->rl.x && bbox.rl.x >= sr->lu.x &&
                    bbox.lu.y >= sr->rl.y && bbox.rl.y <= sr->lu.y)
                intersects=1;
            curr=curr->next;
        }

        /* abort on the special case to read only one coord. */
        if(count == 1)
            break;
        /* we can abort if already intersecting. */
        if(intersects)
            break;
    }
    if(!intersects)
        ret=0;
    return ret;
}

/**
 * @brief Gets all the coordinates of an item within a specified range
 *
 * This will get all the coordinates of the item `i`, starting with `start` and ending with `end`, and
 * return them in `c`, up to `max` coordinates.
 *
 * If `i` does not contain the coordinates in `start`, no coordinates are retrieved and zero is returned.
 *
 * If `i` contains the coordinates in `start` but not those in `end`, all coordinates beginning with
 * `start` are retrieved, ending with the last coordinate of `i` or after `max` coordinates have been
 * retrieved, whichever occurs first.
 *
 * This function is not safe to call after destroying the item's map rect, and doing so may cause errors
 * with some map implementations.
 *
 * @important Make sure that `c` points to a buffer large enough to hold `max` coordinates!
 *
 * @param i The item to get the coordinates of
 * @param c Pointer to memory allocated for holding the coordinates
 * @param max Maximum number of coordinates to return
 * @param start First coordinate to get
 * @param end Last coordinate to get
 *
 * @return The number of coordinates stored in `c`
 */
int item_coord_get_within_range(struct item *i, struct coord *c, int max,
                                struct coord *start, struct coord *end) {
    struct map_rect *mr;
    struct item *item;
    int rc = 0, p = 0;
    struct coord c1;
    mr=map_rect_new(i->map, NULL);
    if (!mr)
        return 0;
    item = map_rect_get_item_byid(mr, i->id_hi, i->id_lo);
    if (item) {
        rc = item_coord_get(item, &c1, 1);
        while (rc && (c1.x != start->x || c1.y != start->y)) {
            rc = item_coord_get(item, &c1, 1);
        }
        while (rc && p < max) {
            c[p++] = c1;
            if (c1.x == end->x && c1.y == end->y)
                break;
            rc = item_coord_get(item, &c1, 1);
        }
    }
    map_rect_destroy(mr);
    return p;
}

/**
 * @brief Gets the next coordinates from an item and reprojects them
 *
 * This function returns a list of coordinates from an item and advances the "coordinate pointer"
 * by the number of coordinates returned, so that at the next call the next coordinates will be returned.
 *
 * @param it The map item whose coordinates to retrieve. This must be the active item, i.e. the last one retrieved from the
 * {@code map_rect}. There can only be one active item per {@code map_rect}.
 * @param c Points to a buffer that will receive the coordinates.
 * The buffer must be at least {@code count * sizeof(struct coord)} bytes in size.
 * @param count The number of coordinates to retrieve. Attempts to read past the
 * end are handled gracefully and only the available number of coordinates is
 * returned.
 * @param projection The projection into which the coordinates will be transformed
 *
 * @return The number of coordinates actually retrieved and stored in {@code c}
 */
int item_coord_get_pro(struct item *it, struct coord *c, int count, enum projection to) {
    int ret=item_coord_get(it, c, count);
    int i;
    enum projection from=map_projection(it->map);
    if (from != to)
        for (i = 0 ; i < count ; i++)
            transform_from_to(c+i, from, c+i, to);
    return ret;
}

/**
 * @brief Whether the current coordinates of an item correspond to a node.
 *
 * TODO which coordinates? Last retrieved or next (to be) retrieved?
 *
 * This function is not safe to call after destroying the item's map rect, and doing so may cause errors
 * with some map implementations.
 *
 * @param it The item
 *
 * @return True on success, false on failure
 */
int item_coord_is_node(struct item *it) {
    if (it->meth->item_coord_is_node)
        return it->meth->item_coord_is_node(it->priv_data);
    return 0;
}

/**
 * @brief Resets the "attribute pointer" of an item
 *
 * This function resets the "attribute pointer" of an item to point to the first attribute,
 * so that at the next call to {@code item_attr_get()} the first attribute will be returned.
 *
 * This function is not safe to call after destroying the item's map rect, and doing so may cause errors
 * with some map implementations.
 *
 * @param it The map item whose pointer is to be reset. This must be the active item, i.e. the last one retrieved from the
 * {@code map_rect}. There can only be one active item per {@code map_rect}.
 */
void item_attr_rewind(struct item *it) {
    it->meth->item_attr_rewind(it->priv_data);
}

/**
 * @brief Gets the next matching attribute from an item
 *
 * This function returns the next attribute matching `attr_type` from an item and advances the
 * "attribute pointer" accordingly, so that at the next call the next attribute will be returned.
 *
 * IMPORTANT: Unless you are iterating over attributes, or operating on a ???fresh??? item (from which no
 * other code has had a chance to retrieve an attribute), be sure to call
 * {@link item_attr_rewind(struct item *)} before each call to this method. Not doing so may result in
 * unpredictable behavior, i.e. attributes not being found if the attribute pointer is already past the
 * requested attribute (which depends on the physical ordering of attributes and on the last attribute
 * retrieved from the item).
 *
 * This function is not safe to call after destroying the item's map rect, and doing so may cause errors
 * with some map implementations.
 *
 * @param[in] it The map item whose attribute to retrieve. This must be the active item, i.e. the last one retrieved from the
 * {@code map_rect}. There can only be one active item per {@code map_rect}.
 * @param[in] attr_type The attribute type to retrieve, or `attr_any` to retrieve the next attribute
 * @param[out] attr Receives the attribute retrieved
 *
 * @return True on success, false on failure
 */
int item_attr_get(struct item *it, enum attr_type attr_type, struct attr *attr) {
    return it->meth->item_attr_get(it->priv_data, attr_type, attr);
}

/**
 * @brief Sets an attribute of an item
 *
 * This function supports different modes:
 *
 * \li `change_mode_delete`: Deletes the attribute
 * \li `change_mode_modify`: Replaces an attribute
 * \li `change_mode_append`: Appends an attribute
 * \li `change_mode_prepend`: Prepends an attribute
 *
 * TODO which attribute is deleted/modified? The last one retrieved, the next one, the first matching one?
 *
 * TODO where are attributes appended/prepended? Beginning/end or current position?
 *
 * This function is not safe to call after destroying the item's map rect, and doing so may cause errors
 * with some map implementations.
 *
 * @param it The map item whose coordinates to retrieve. This must be the active item, i.e. the last one retrieved from the
 * {@code map_rect}. There can only be one active item per {@code map_rect}.
 * @param attr TODO new attr, also store for old attr (delete/modify)? Required in delete mode (type of attr to delete)?
 * @param mode The change mode, see description
 */
int item_attr_set(struct item *it, struct attr *attr, enum change_mode mode) {
    if (!it->meth->item_attr_set)
        return 0;
    return it->meth->item_attr_set(it->priv_data, attr, mode);
}

/**
 * @brief Sets the type of a map item.
 *
 * @param it The item
 * @param type The new type for the item. Setting it to type_none is expected to delete item from the map.
 *
 * @return Non-zero if this action is supported by the map and type is set successfully, 0 on error.
 */
int item_type_set(struct item *it, enum item_type type) {
    if (!it->meth->item_type_set)
        return 0;
    return it->meth->item_type_set(it->priv_data, type);
}

struct item * item_new(char *type, int zoom) {
    struct item * it;

    it = g_new0(struct item, 1);

    /* FIXME evaluate arguments */

    return it;
}

enum item_type item_from_name(const char *name) {
    int i;

    if (item_hash)
        return GPOINTER_TO_INT(g_hash_table_lookup(item_hash, name));

    for (i=0 ; i < sizeof(item_names)/sizeof(struct item_name) ; i++) {
        if (! strcmp(item_names[i].name, name))
            return item_names[i].item;
    }
    return type_none;
}

char *item_to_name(enum item_type item) {
    int i;

    for (i=0 ; i < sizeof(item_names)/sizeof(struct item_name) ; i++) {
        if (item_names[i].item == item)
            return item_names[i].name;
    }
    return NULL;
}

struct item_hash {
    GHashTable *h;
};

static guint item_hash_hash(gconstpointer key) {
    const struct item *itm=key;
    gconstpointer hashkey=(gconstpointer)GINT_TO_POINTER(itm->id_hi^itm->id_lo^(GPOINTER_TO_INT(itm->map)));
    return g_direct_hash(hashkey);
}

static gboolean item_hash_equal(gconstpointer a, gconstpointer b) {
    const struct item *itm_a=a;
    const struct item *itm_b=b;
    if (item_is_equal(*itm_a, *itm_b))
        return TRUE;
    return FALSE;
}

unsigned int item_id_hash(const void *key) {
    const struct item_id *id=key;
    return id->id_hi^id->id_lo;
}

int item_id_equal(const void *a, const void *b) {
    const struct item_id *id_a=a;
    const struct item_id *id_b=b;
    return (id_a->id_hi == id_b->id_hi && id_a->id_lo == id_b->id_lo);
}

/**
 * @brief Derive item id_lo and id_hi from pointer, considering pointer could be 32 or 64 bit wide but both ids are 32 bit.
 *
 * @param it reference to the item.
 * @param id pointer to derive item id from.
 * @return  Nothing.
 */
void item_id_from_ptr(struct item *item, void *id) {
#if !defined(__LP64__) && !defined(__LLP64__) && !defined(WIN64)
    item->id_lo=(int) id;
    item->id_hi=0;
#else
#	ifndef _MSC_VER
    item->id_lo=((long long)id)&0xFFFFFFFFll;
#	else
    item->id_lo=((long long)id)&0xFFFFFFFFi64;
#	endif
    item->id_hi=((long long)id)>>32;
#endif
}


struct item_hash *
item_hash_new(void) {
    struct item_hash *ret=g_new(struct item_hash, 1);

    ret->h=g_hash_table_new_full(item_hash_hash, item_hash_equal, g_free, NULL);
    return ret;
}

void item_hash_insert(struct item_hash *h, struct item *item, void *val) {
    struct item *hitem=g_new(struct item, 1);
    *hitem=*item;
    dbg(lvl_info,"inserting (0x%x,0x%x) into %p", item->id_hi, item->id_lo, h->h);
    g_hash_table_insert(h->h, hitem, val);
}

int item_hash_remove(struct item_hash *h, struct item *item) {
    int ret;

    dbg(lvl_info,"removing (0x%x,0x%x) from %p", item->id_hi, item->id_lo, h->h);
    ret=g_hash_table_remove(h->h, item);
    dbg(lvl_info,"ret=%d", ret);

    return ret;
}

void *item_hash_lookup(struct item_hash *h, struct item *item) {
    return g_hash_table_lookup(h->h, item);
}


void item_hash_destroy(struct item_hash *h) {
    g_hash_table_destroy(h->h);
    g_free(h);
}

int item_range_intersects_range(struct item_range *range1, struct item_range *range2) {
    if (range1->max < range2->min)
        return 0;
    if (range1->min > range2->max)
        return 0;
    return 1;
}
int item_range_contains_item(struct item_range *range, enum item_type type) {
    if (type >= range->min && type <= range->max)
        return 1;
    return 0;
}

void item_dump_attr(struct item *item, struct map *map, FILE *out) {
    struct attr attr;
    fprintf(out,"type=%s", item_to_name(item->type));
    while (item_attr_get(item, attr_any, &attr))
        fprintf(out," %s='%s'", attr_to_name(attr.type), attr_to_text(&attr, map, 1));
}

void item_dump_filedesc(struct item *item, struct map *map, FILE *out) {

    int i,count,max=16384;
    struct coord *ca=g_alloca(sizeof(struct coord)*max);

    count=item_coord_get(item, ca, item->type < type_line ? 1: max);
    if (item->type < type_line)
        fprintf(out,"mg:0x%x 0x%x ", ca[0].x, ca[0].y);
    item_dump_attr(item, map, out);
    fprintf(out,"\n");
    if (item->type >= type_line)
        for (i = 0 ; i < count ; i++)
            fprintf(out,"mg:0x%x 0x%x\n", ca[i].x, ca[i].y);
}
