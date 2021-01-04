#pragma once

#include "camera.h"
#include "object.h"
#include "bsptree.h"

#include "utils/pvect.h"

// this code creates a new type of vector using C's
// poor man template metaprogramming™
#define GVECT_NAME object_vect
#define GVECT_TYPE struct object *
#include "utils/pvect_wrap.h"
#undef GVECT_NAME
#undef GVECT_TYPE

/* The scene contains all the objects, lights, and cameras.
** for simplicity scene, this scene type only handles a single light and
** a single camera.
*/
struct scene
{
    // the list of objects in the scene
    struct bsptree *root;

    // a very hacky single light
    // TODO: handle multiple lights
    struct vec3 light_color;
    struct vec3 light_direction;
    double light_intensity;

    struct camera camera;
};

static inline void scene_init(struct scene *scene, struct
object_vect *list)
{
    struct vec3 tmp[2] = {{ 0, 0, 0 }, { 0, 0, 0 }};
    scene->root = bsp_init(tmp);
    object_vect_init(list, 42);
    return;
}

void scene_destroy(struct scene *scene, struct object_vect *objs);
