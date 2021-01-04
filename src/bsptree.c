#include "bsptree.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

struct bsptree *bsp_init(struct vec3 *coord)
{
    struct bsptree *new = malloc(sizeof(*new));
    if (!new)
        errx(1, "Malloc failed !");
    new->list = malloc(sizeof(struct object_list));
    if (!new->list)
        errx(1, "Malloc failed !");
    new->list->elm = NULL;
    new->list->next = NULL;
    struct vec3 temp = { min(coord[0].x, coord[1].x),
                         min(coord[0].y, coord[1].y),
                         min(coord[0].z, coord[1].z) };
    new->coord[0] = temp;
    struct vec3 temp2 = { max(coord[0].x, coord[1].x),
                          max(coord[0].y, coord[1].y),
                          max(coord[0].z, coord[1].z) };
    new->coord[1] = temp2;
    new->size = 0;
    new->axe = X;
    new->left = NULL;
    new->right = NULL;
    return new;
}
void bsp_print(struct bsptree *tree)
{
    if (!tree)
        return;
    printf("node { ");
    if (!tree->left)
    {
        printf("%i items", tree->size);
    }
    else
    {
        bsp_print(tree->left);
        printf(" ; ");
        bsp_print(tree->right);
    }
    printf(" }");
}
void bsp_destroy(struct bsptree *tree)
{
    if (!tree)
        return;
    bsp_destroy(tree->left);
    bsp_destroy(tree->right);
    struct object_list *tmp = tree->list->next;
    while (tmp)
    {
        puts("no");
        tree->list->next = tmp->next;
        printf("%p\n", (void *)tmp->elm->free);
        tmp->elm->free(tmp->elm);
        puts("c");
        free(tmp);
        puts("d");
        tmp = tree->list->next;
    }
    puts("yes");
    free(tree->list);
    free(tree);
    return;
}

static void swap(double *a, double *b)
{
    double temp = *a;
    *a = *b;
    *b = temp;
}

int bsp_intersec(struct ray r, struct bsptree *tree)
{
    /*    if (pose.x > tree->coord[1].x || pose.x < tree->coord[0].x)
            return 0;
        if (pose.y > tree->coord[1].y || pose.y < tree->coord[0].y)
            return 0;
         if (pose.z > tree->coord[1].z || pose.z < tree->coord[0].z)
            return 0;*/
    double tmin = (tree->coord[0].x - r.source.x) / r.direction.x;
    double tmax = (tree->coord[1].x - r.source.x) / r.direction.x;

    if (tmin > tmax)
        swap(&tmin, &tmax);

    double tymin = (tree->coord[0].y - r.source.y) / r.direction.y;
    double tymax = (tree->coord[1].y - r.source.y) / r.direction.y;

    if (tymin > tymax)
        swap(&tymin, &tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return 0;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    double tzmin = (tree->coord[0].z - r.source.z) / r.direction.z;
    double tzmax = (tree->coord[1].z - r.source.z) / r.direction.z;

    if (tzmin > tzmax)
        swap(&tzmin, &tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return 0;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    return 1;
}

static int hitbox_collide(struct object *obj, struct bsptree *tree)
{
    struct vec3 *hb = obj->hitbox(obj);
    /*printf("tree coord_max: %f %f %f \n",tree->coord[1].x, tree->coord[1].y,
    tree->coord[1].z); printf("hb coord_max: %f %f %f \n",hb[1].x, hb[1].y,
    hb[1].z); printf("tree coord_min: %f %f %f \n",tree->coord[0].x,
    tree->coord[0].y, tree->coord[0].z); printf("hb coord_min: %f %f %f
    \n",hb[0].x, hb[0].y, hb[0].z);*/
    if (!(hb[0].x <= tree->coord[1].x && hb[1].x >= tree->coord[0].x))
        return 0;
    if (!(hb[0].y <= tree->coord[1].y && hb[1].y >= tree->coord[0].y))
        return 0;
    if (!(hb[0].z <= tree->coord[1].z && hb[1].z >= tree->coord[0].z))
        return 0;
    return 1;
}

static void add_obj(struct bsptree *tree, struct object *obj)
{
    struct object_list *new = malloc(sizeof(*new));
    new->elm = obj;
    new->next = tree->list->next;
    tree->list->next = new;
    tree->size++;
}

void bsp_add(struct bsptree *tree, struct object *obj)
{
    if (!tree->left)
        add_obj(tree, obj);
    else
    {
        if (hitbox_collide(obj, tree->left))
        {
            bsp_add(tree->left, obj);
        }
        if (hitbox_collide(obj, tree->right))
        {
            bsp_add(tree->right, obj);
        }
    }
}

void bsp_cut(struct bsptree *tree)
{
    if (tree->left)
        return;
    double tab1[3] = { tree->coord[0].x, tree->coord[0].y, tree->coord[0].z };
    double tab2[3] = { tree->coord[1].x, tree->coord[1].y, tree->coord[1].z };
    if (tab1[tree->axe] == 0 && tab2[tree->axe] == 0)
        return;
    double tab3[3] = { tab1[0], tab1[1], tab1[2] };
    double tab4[3] = { tab2[0], tab2[1], tab2[2] };
    double dist = tab1[tree->axe] - tab2[tree->axe];
    dist = dist < 0 ? -dist : dist;
    tab3[tree->axe] += dist / 2;
    tab4[tree->axe] -= dist / 2;
    struct vec3 left[2] = { (struct vec3){ tab1[0], tab1[1], tab1[2] },
                            (struct vec3){ tab4[0], tab4[1], tab4[2] } };
    struct vec3 right[2] = { (struct vec3){ tab3[0], tab3[1], tab3[2] },
                             (struct vec3){ tab2[0], tab2[1], tab2[2] } };
    tree->left = bsp_init(left);
    tree->right = bsp_init(right);
    int new_axe = (tree->axe + 1) % 3;
    tree->left->axe = new_axe;
    tree->right->axe = new_axe;
}

static void bsp_build_rec(struct bsptree *tree, int cpt)
{
    if (!cpt || !tree)
        return;
    bsp_cut(tree);
    bsp_build_rec(tree->left, cpt - 1);
    bsp_build_rec(tree->right, cpt - 1);
}

void bsp_build(struct bsptree *tree)
{
    bsp_build_rec(tree, 4);
}

static struct bsptree *bsp_closest(struct bsptree *tree_left,
                                 struct bsptree *tree_right, struct ray ray)
{
    struct vec3 left = vec3_sub(&tree_left->coord[0], &tree_left->coord[1]);
    left.x = left.x < 0 ? -left.x : left.x;
    left.y = left.y < 0 ? -left.y : left.y;
    left.z = left.z < 0 ? -left.z : left.z;
    left = vec3_mul(&left, 0.5);
    left = vec3_add(&left, &tree_left->coord[0]);
    struct vec3 right = vec3_sub(&tree_right->coord[0], &tree_right->coord[1]);
    right.x = right.x < 0 ? -right.x : right.x;
    right.y = right.y < 0 ? -right.y : right.y;
    right.z = right.z < 0 ? -right.z : right.z;
    right = vec3_mul(&right, 0.5);
    right = vec3_add(&right, &tree_right->coord[0]);
    struct vec3 vec_left = vec3_sub(&left, &ray.source);
    struct vec3 vec_right = vec3_sub(&right, &ray.source);
    double dist_left = vec3_length(&vec_left);
    double dist_right = vec3_length(&vec_right);
    return (dist_left < dist_right) ? tree_left : tree_right;
}

static struct bsptree *find_box_rec(struct bsptree *tree, struct ray ray)
{
    if (!tree->left)
        return tree;
    struct bsptree *tree_left = NULL;
    struct bsptree *tree_right = NULL;
    if (bsp_intersec(ray, tree->left))
        tree_left = find_box_rec(tree->left, ray);
    if (bsp_intersec(ray, tree->right))
        tree_right = find_box_rec(tree->right, ray);
    if (!tree_left)
    {
        return tree_right;
    }
    if (!tree_right)
    {
        return tree_left;
    }
    return bsp_closest(tree_left, tree_right, ray);
}

struct bsptree *find_box(struct bsptree *tree, struct ray ray)
{
    return find_box_rec(tree, ray);
}
