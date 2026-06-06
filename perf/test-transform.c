#include "test-common.h"

void transform(const char* filePath);

gint main(gint argc, gchar **argv) {
    gegl_init(&argc, &argv);
    //GeglBuffer *buffer = NULL;
    //
    //
    // gint width = 1024;
    // gint height = 1024;
    // buffer = test_buffer(width, height, babl_format("R'G'B' u8"));

    //bench("transform", buffer, &transform);
    transform("/Users/christophekunzli/CLionProjects/gegl/test.png");
    gegl_exit();
    return 0;
}

void transform(const gchar* filePath) {
    GeglNode *gegl, *load, *translate, *rotate, *scale, *save;

    gegl = gegl_node_new();
    load = gegl_node_new_child(gegl,
                               "operation",
                               "gegl:png-load",
                               "path",
                               filePath,
                               NULL);
    //translate = gegl_node_new_child(gegl, "operation", "gegl:translate", "x", 200.0, "y", 100.0, NULL);
    rotate = gegl_node_new_child(gegl, "operation", "gegl:rotate", "degrees", 90.0, NULL);
    // scale = gegl_node_new_child(gegl,
    //                             "operation",
    //                             "gegl:scale-ratio",
    //                             "x",
    //                             2.0,
    //                             "y",
    //                             1.0,
    //                             "sampler",
    //                             GEGL_SAMPLER_LINEAR,
    //                             NULL);

    save = gegl_node_new_child(gegl,
                               "operation",
                               "gegl:png-save",
                               "path",
                               "/Users/christophekunzli/CLionProjects/gegl/out.png",
                               NULL);

    gegl_node_link_many(load, rotate, save, NULL);
    gegl_node_process(save);
    g_object_unref(gegl);
}
