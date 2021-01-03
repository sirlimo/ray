#include <err.h>
#include <libgen.h>

#include "color.h"
#include "normal_material.h"
#include "phong_material.h"
#include "scene.h"
#include "triangle.h"
#include "utils/alloc.h"
#include "utils/evect.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

static char *read_file(size_t *file_size, const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        warn("file not found while loading obj: %s", path);
        return NULL;
    }

    struct evect res_buf;
    evect_init(&res_buf, 4000);

    int c;
    while ((c = getc(fp)) != EOF)
        evect_push(&res_buf, c);

    fclose(fp);
    evect_finalize(&res_buf);
    *file_size = evect_size(&res_buf);
    return evect_data(&res_buf);
}

static void get_file_data(const char *filename, const int is_mtl,
                          const char *obj_filename, char **data,
                          size_t *data_len)
{
    if (!filename)
    {
        fprintf(stderr, "null filename\n");
        *data = NULL;
        *data_len = 0;
        return;
    }

    const char *ext = strrchr(filename, '.');

    if (strcmp(ext, ".gz") == 0)
        abort();

    char *basedirname_buf = NULL;
    char *basedirname = NULL;
    char tmp[1024];
    tmp[0] = '\0';

    /* For .mtl, extract base directory path from .obj filename and append .mtl
     * filename */
    if (is_mtl && obj_filename)
    {
        basedirname_buf = strdup(obj_filename);
        basedirname = dirname(basedirname_buf);
    }

    if (basedirname)
        snprintf(tmp, sizeof(tmp), "%s/%s", basedirname, filename);
    else
        strncpy(tmp, filename, strlen(filename) + 1);

    tmp[sizeof(tmp) - 1] = '\0';

    if (basedirname_buf)
        free(basedirname_buf);

    *data = read_file(data_len, tmp);
}

static void minimal(struct vec3 *mini, struct vec3 pts)
{
    mini->x = min(mini->x, pts.x);
    mini->y = min(mini->y, pts.y);
    mini->z = min(mini->z, pts.z);
}

static void maximal(struct vec3 *mini, struct vec3 pts)
{
    mini->x = max(mini->x, pts.x);
    mini->y = max(mini->y, pts.y);
    mini->z = max(mini->z, pts.z);
}

#include "utils/pvect.h"

#define GVECT_NAME phong_material_vect
#define GVECT_TYPE struct phong_material *
#include "utils/pvect_wrap.h"
#undef GVECT_NAME
#undef GVECT_TYPE

int load_obj(struct scene *scene, const char *filename,
             struct object_vect *list)
{
    struct vec3 mini = { 0, 0, 0 };
    struct vec3 maxi = { 0, 0, 0 };
    tinyobj_attrib_t attrib;
    tinyobj_shape_t *shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t *materials = NULL;
    size_t num_materials;

    int rc;
    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;

    rc = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
                           &num_materials, filename, get_file_data, flags);
    if (rc != TINYOBJ_SUCCESS)
        return -1;

    struct phong_material_vect conv_materials;
    phong_material_vect_init(&conv_materials, num_materials);

    // convert materials
    for (size_t i = 0; i < num_materials; i++)
    {
        struct phong_material *shape_material = zalloc(sizeof(*shape_material));
        phong_material_init(shape_material);
        shape_material->diffuse_Kn = 0.2;
        shape_material->spec_n = 10;
        shape_material->spec_Ks = 0.2;
        shape_material->ambient_intensity = 0.01;
        float *diff_color = materials[i].diffuse;
        shape_material->surface_color = light_from_rgb_color(
            diff_color[0] * 255, diff_color[1] * 255, diff_color[2] * 255);
        phong_material_vect_push(&conv_materials, shape_material);
    }

    // convert faces
    for (size_t face_i = 0; face_i < attrib.num_face_num_verts; face_i++)
    {
        assert(attrib.face_num_verts[face_i] == 3);
        int mat_id = attrib.material_ids[face_i];
        struct phong_material *mat =
            phong_material_vect_get(&conv_materials, mat_id);

        size_t face_off = face_i * 3;
        struct vec3 points[3];

        for (size_t node_i = 0; node_i < 3; node_i++)
        {
            tinyobj_vertex_index_t node_idx = attrib.faces[face_off + node_i];
            size_t node_vertex_offset = 3 * node_idx.v_idx;
            points[node_i].x = attrib.vertices[node_vertex_offset + 0];
            points[node_i].y = attrib.vertices[node_vertex_offset + 1];
            points[node_i].z = attrib.vertices[node_vertex_offset + 2];
            minimal(&mini, points[node_i]);
            maximal(&maxi, points[node_i]);
        }

        struct triangle *trian = triangle_create(points, &mat->base);

        object_vect_push(list, &trian->base);
    }

    // release the reference counter of materials
    for (size_t i = 0; i < num_materials; i++)
    {
        struct phong_material *mat =
            phong_material_vect_get(&conv_materials, i);
        material_put(&mat->base);
    }

    phong_material_vect_destroy(&conv_materials);

    // free tinyobjloader internal structures
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);
    printf("mini box: %f, %f, %f\n", mini.x, mini.y, mini.z);
    printf("maxi box: %f, %f, %f\n", maxi.x, maxi.y, maxi.z);
    scene->root->coord[0] = mini;
    scene->root->coord[1] = maxi;
    return 0;
}
