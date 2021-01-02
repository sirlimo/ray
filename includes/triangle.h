#pragma once

#include "object.h"
#include "utils/alloc.h"
#include "vec3.h"
#include "kdtree.h"

#include <stdio.h>
#include <stddef.h>

/*
** The facing side of the triangle is the one where the points appear
** in counter clockwise order.
*/
struct triangle
{
    struct object base;
    struct vec3 points[3];
    struct material *material;
    struct vec3 hitbox[2];
};


double object_triangle_ray_intersect(struct object_intersection *inter,
                                     const struct object *obj,
                                     const struct ray *ray);

void triangle_free(struct object *obj);

struct vec3 *triangle_hitbox(struct object *obj);

static inline struct triangle *triangle_create(struct vec3 points[3],
                                               struct material *mat)
{
    struct triangle *trian = zalloc(sizeof(*trian));
    object_init(&trian->base, object_triangle_ray_intersect, triangle_free,
    triangle_hitbox);
    trian->points[0] = points[0];
    trian->points[1] = points[1];
    trian->points[2] = points[2];
    struct vec3 temp = {(max(max(points[0].x, points[1].x), points[2].x))
                        , (max(max(points[0].y, points[1].y), points[2].y))
                        , (max(max(points[0].z, points[1].z), points[2].z))};
    trian->hitbox[1] = temp;
    struct vec3 temp2 = {(min(min(points[0].x, points[1].x), points[2].x))
                        , (min(min(points[0].y, points[1].y), points[2].y))
                        , (min(min(points[0].z, points[1].z), points[2].z))};
    trian->hitbox[0] = temp2;
    if(temp.x>2)
      printf("hb max = %f %f %f\n",temp.x,temp.y,temp.z);
    trian->material = material_get(mat);
    return trian;
 }
