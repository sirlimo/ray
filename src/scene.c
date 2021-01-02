#include "scene.h"

void scene_destroy(struct scene *scene, struct object_vect *objs)
{
  if(!scene)
    return;
  for (size_t i = 0; i < object_vect_size(objs); i++)
    {
        struct object *obj = object_vect_get(objs, i);
        if (obj->free)
            obj->free(obj);
    }

    object_vect_destroy(objs);
}
