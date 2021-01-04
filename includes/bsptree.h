#ifndef BSPTREE_H
#define BSPTREE_H

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

struct bsptree
{
    int size;
    enum split_axis axe;
    struct object_list *list;
    struct bsptree *left;
    struct bsptree *right;
    struct vec3 coord[2];
};
void bsp_print(struct bsptree *tree);
struct bsptree *bsp_init(struct vec3 *coord);
void bsp_destroy(struct bsptree *tree);
int bsp_intersec(struct ray ray, struct bsptree *tree);
struct bsptree *find_box(struct bsptree *tree, struct ray ray);
void bsp_add(struct bsptree *tree, struct object *obj);
void bsp_cut(struct bsptree *tree);
void bsp_build(struct bsptree *tree);


#endif
