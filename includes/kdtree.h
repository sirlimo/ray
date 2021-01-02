#ifndef KDTREE_H
#define KDTREE_H

#include "vec3.h"
#include "object.h"
#include "ray.h"

enum split_axis
{
    X = 0,
    Y,
    Z
};

struct object_list
{
    struct object *elm;
    struct object_list *next;
};

struct kdtree
{
    int size;
    enum split_axis axe;
    struct object_list *list;
    struct kdtree *left;
    struct kdtree *right;
    struct vec3 coord[2];
};
void kd_print(struct kdtree *tree);
struct kdtree *kd_init(struct vec3 *coord);
void kd_destroy(struct kdtree *tree);
int kd_intersec(struct ray ray, struct kdtree *tree);
struct kdtree *find_box(struct kdtree *tree, struct ray ray);
void kd_add(struct kdtree *tree, struct object *obj);
void kd_cut(struct kdtree *tree);
void kd_build(struct kdtree *tree);


#endif
