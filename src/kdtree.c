#include "kdtree.h"

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

struct kdtree *kd_init(struct vec3 *coord)
{
    struct kdtree *new = malloc(sizeof(*new));
    if (!new)
        errx(1, "Malloc failed !");
    new->list = malloc(sizeof(struct object_list));
    if (!new->list)
        errx(1, "Malloc failed !");
    new->list->elm = NULL;
    new->list->next = NULL;
    struct vec3 temp =
    {min(coord[0].x,coord[1].x),min(coord[0].y,coord[1].y),min(coord[0].z
    ,coord[1].z)};
    new->coord[0] = temp;
    struct vec3 temp2 =
    {max(coord[0].x,coord[1].x),max(coord[0].y,coord[1].y),max(coord[0].z
    ,coord[1].z)};
    new->coord[1] = temp2;
    new->size = 0;
    new->axe = X;
    new->left = NULL;
    new->right = NULL;
    return new;
}
void kd_print(struct kdtree *tree)
{
  if(!tree)
    return;
  printf("node { ");
  if(!tree->left)
    {
      printf("%i items",tree->size);
    }
  else
    {
      kd_print(tree->left);
      printf(" ; ");
      kd_print(tree->right);
    }
  printf(" }");
  
}
void kd_destroy(struct kdtree *tree)
{
    if (!tree)
        return;
    kd_destroy(tree->left);
    kd_destroy(tree->right);
    struct object_list *tmp = tree->list->next;
    while (tmp)
    {
	puts("no");
      tree->list->next = tmp->next;
      printf("%p\n",(void *)tmp->elm->free);
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

int kd_intersec(struct ray r, struct kdtree *tree)
{
/*    if (pose.x > tree->coord[1].x || pose.x < tree->coord[0].x)
        return 0;
    if (pose.y > tree->coord[1].y || pose.y < tree->coord[0].y)
        return 0;
     if (pose.z > tree->coord[1].z || pose.z < tree->coord[0].z)
        return 0;*/
    double tmin = (tree->coord[0].x - r.source.x) / r.direction.x; 
    double tmax = (tree->coord[1].x - r.source.x) / r.direction.x; 
 
    if (tmin > tmax) swap(&tmin, &tmax); 
 
    double tymin = (tree->coord[0].y - r.source.y) / r.direction.y; 
    double tymax = (tree->coord[1].y - r.source.y) / r.direction.y; 
 
    if (tymin > tymax) swap(&tymin, &tymax); 
 
    if ((tmin > tymax) || (tymin > tmax)) 
        return 0; 
 
    if (tymin > tmin) 
        tmin = tymin; 
 
    if (tymax < tmax) 
        tmax = tymax; 
 
    double tzmin = (tree->coord[0].z - r.source.z) / r.direction.z; 
    double tzmax = (tree->coord[1].z - r.source.z) / r.direction.z; 
 
    if (tzmin > tzmax) swap(&tzmin, &tzmax); 
 
    if ((tmin > tzmax) || (tzmin > tmax)) 
        return 0; 
 
    if (tzmin > tmin) 
        tmin = tzmin; 
 
    if (tzmax < tmax) 
        tmax = tzmax; 
 
    return 1; 
}

static int hitbox_collide(struct object *obj, struct kdtree *tree)
{
    struct vec3 *hb = obj->hitbox(obj);
    /*printf("tree coord_max: %f %f %f \n",tree->coord[1].x, tree->coord[1].y, tree->coord[1].z);
    printf("hb coord_max: %f %f %f \n",hb[1].x, hb[1].y, hb[1].z);
    printf("tree coord_min: %f %f %f \n",tree->coord[0].x, tree->coord[0].y, tree->coord[0].z);
    printf("hb coord_min: %f %f %f \n",hb[0].x, hb[0].y, hb[0].z);*/
    if (!(hb[0].x <= tree->coord[1].x && hb[1].x >= tree->coord[0].x))
        return 0;
    if (!(hb[0].y <= tree->coord[1].y && hb[1].y >= tree->coord[0].y))
        return 0;
    if (!(hb[0].z <= tree->coord[1].z && hb[1].z >= tree->coord[0].z))
        return 0;
    return 1;
}

/*
static void kdsplit(struct kdtree *tree)
{
    struct vec3 left_coord = {tree->coord[0]->x};
    struct vec3 right_coord = {};
    struct kd_left = 
}*/

static void add_obj(struct kdtree *tree, struct object *obj)
{
    struct object_list *new = malloc(sizeof(* new));
    new->elm = obj;
    new->next = tree->list->next;
    tree->list->next = new;
    tree->size++;
//    if (tree->size == 10)
//        kdsplit(tree);
}

void kd_add(struct kdtree *tree, struct object *obj)
{
    if (!tree->left)
        add_obj(tree, obj);
    else
    {
        if (hitbox_collide(obj, tree->left))
	  {
	    kd_add(tree->left, obj);
	  }
        if (hitbox_collide(obj, tree->right))
	  {
	    kd_add(tree->right, obj);
	  }
    }
}
void kd_cut(struct kdtree *tree)
{
    if (tree->left)
        return;
    double tab1[3] = {tree->coord[0].x, tree->coord[0].y, tree->coord[0].z};
    double tab2[3] = {tree->coord[1].x, tree->coord[1].y, tree->coord[1].z};
    if (tab1[tree->axe] == 0 && tab2[tree->axe] == 0)
        return;
    double tab3[3] = {tab1[0],tab1[1],tab1[2]};
    double tab4[3] = {tab2[0],tab2[1],tab2[2]};
    double dist = tab1[tree->axe] - tab2[tree->axe];
    dist = dist < 0 ? -dist : dist;
    tab3[tree->axe] += dist/2;
    tab4[tree->axe] -= dist/2;
    struct vec3 left[2] = {
      (struct vec3){tab1[0],tab1[1],tab1[2]}
      ,(struct vec3){tab4[0],tab4[1],tab4[2]}
    };
    struct vec3 right[2] = {
      (struct vec3){tab3[0],tab3[1],tab3[2]}
      ,(struct vec3){tab2[0],tab2[1],tab2[2]}
    };
    //struct vec3 right[2] = {(struct vec3)tab3, (struct vec3)tab2}
    tree->left = kd_init(left);
    tree->right = kd_init(right);
    int new_axe = (tree->axe + 1) % 3;
    tree->left->axe = new_axe;
    tree->right->axe = new_axe;
}

static void kd_build_rec(struct kdtree *tree, int cpt)
{
    if (!cpt || !tree)
        return;
    kd_cut(tree);
    kd_build_rec(tree->left, cpt - 1);
    kd_build_rec(tree->right, cpt - 1);
}

void kd_build(struct kdtree *tree)
{
    kd_build_rec(tree, 3);
}

static struct kdtree *kd_closest(struct kdtree *tree_left, struct kdtree *tree_right,
struct ray ray)
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

static struct kdtree *find_box_rec(struct kdtree *tree, struct ray ray)
{
    if (!tree->left)
        return tree;
    struct kdtree *tree_left = NULL;
    struct kdtree *tree_right = NULL;
    if (kd_intersec(ray, tree->left))
        tree_left = find_box_rec(tree->left, ray);
    else if (kd_intersec(ray, tree->left))
        tree_right = find_box_rec(tree->left, ray);
    if (!tree_left)
    {
        return tree_right;
    }
    if (!tree_right)
    {
        return tree_left;
    }
    return kd_closest(tree_left, tree_right, ray);
}

struct kdtree *find_box(struct kdtree *tree, struct ray ray)
{
    return find_box_rec(tree, ray);
}
