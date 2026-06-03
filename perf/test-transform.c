#include <gegl.h>
#include "test-common.h"
#include "halide_transform.h"

void transform(GeglBuffer *buffer);
uint8_t *alloc_buffer(gint width, gint height, const Babl *fmt);
uint8_t *read_buffer_as_rgb_u8(GeglBuffer *buf);
GeglBuffer *linear_wrap_pixels(uint8_t *pixels, int width, int height);
float *read_as_rgba_float(GeglBuffer *buf, int *w, int *h, int *rowstride_bytes);

uint8_t *alloc_buffer(gint width, gint height, const Babl *fmt) // returns allocated data, caller frees
{
    gint bpp = babl_format_get_bytes_per_pixel(fmt); // bytes per pixel
    gsize alloc_size = (gsize) width * (gsize) bpp * (gsize) height;
    uint8_t *data = g_malloc0(alloc_size);

    return data;
}

uint8_t *read_buffer_as_rgb_u8(GeglBuffer *buf) // returns allocated data, caller frees
{
    const Babl *fmt = babl_format("R'G'B' u8"); // 3 channels, 8-bit
    gint width = gegl_buffer_get_width(buf);
    gint height = gegl_buffer_get_height(buf);
    gint bpp = babl_format_get_bytes_per_pixel(fmt); // bytes per pixel
    gint rowstride = GEGL_AUTO_ROWSTRIDE; // let GEGL choose

    // Compute size if GEGL_AUTO_ROWSTRIDE is replaced; we'll pass AUTO and trust GEGL
    // Allocate a large enough buffer: (width * bpp * height) is safe upper bound
    gsize alloc_size = (gsize) width * (gsize) bpp * (gsize) height;
    uint8_t *data = g_malloc0(alloc_size);

    GeglRectangle rect = {0, 0, width, height};

    // GEGL_ABYSS_CLAMP / GEGL_ABYSS_NONE etc depend on how you want out-of-bounds handled
    gegl_buffer_get(buf, &rect, 1.0, fmt, data, rowstride, GEGL_ABYSS_CLAMP);

    return data;
}

GeglBuffer *make_gegl_buffer(uint8_t *pixels, gint width, gint height) {
    const Babl *fmt = babl_format("R'G'B' u8");
    GeglRectangle rect = {0, 0, width, height};
    int rowstride = width * babl_format_get_bytes_per_pixel(fmt);

    // Create a linear buffer that wraps the provided pixel memory (no copy).
    // The last two args are free_func and free_func_data (NULL means GEGL won't free your memory).
    GeglBuffer *linear = gegl_buffer_linear_new_from_data(pixels, fmt, &rect, rowstride, NULL, NULL);

    return linear;
}

float *read_as_rgba_float(GeglBuffer *buf, int *w, int *h, int *rowstride_bytes) {
    const Babl *fmt = babl_format("RGBA float");
    int width = gegl_buffer_get_width(buf);
    int height = gegl_buffer_get_height(buf);
    int bpp = babl_format_get_bytes_per_pixel(fmt); // should be 16
    int rs = width * bpp; // bytes per row
    float *data = g_malloc0((size_t) rs * height);
    GeglRectangle rect = {0, 0, width, height};
    gegl_buffer_get(buf, &rect, 1.0, fmt, data, rs, GEGL_ABYSS_CLAMP);
    if (w) *w = width;
    if (h) *h = height;
    if (rowstride_bytes) *rowstride_bytes = rs;
    return data;
}

static void make_buffer(uint8_t *data, int32_t width, int32_t height, struct halide_buffer_t *buf) {
    *buf = (struct halide_buffer_t){0};
    buf->host = data;

    buf->type.code = halide_type_uint;
    buf->type.bits = 8;
    buf->type.lanes = 1;

    buf->dimensions = 2;

    buf->dim = malloc(sizeof(halide_dimension_t) * buf->dimensions);

    buf->dim[0].extent = width;
    buf->dim[0].stride = 1;
    buf->dim[1].extent = height;
    buf->dim[1].stride = 1;
}

static void make_rgb_buffer(uint8_t *data, int32_t width, int32_t height, struct halide_buffer_t *buf) {
    *buf = (struct halide_buffer_t){0};

    buf->host = data;
    buf->dimensions = 3;

    buf->type.code = halide_type_uint;
    buf->type.bits = 8;
    buf->type.lanes = 1;

    buf->dim = malloc(sizeof(struct halide_dimension_t) * 3);

    // x
    buf->dim[0].min = 0;
    buf->dim[0].extent = width;
    buf->dim[0].stride = 3;

    // y
    buf->dim[1].min = 0;
    buf->dim[1].extent = height;
    buf->dim[1].stride = width * 3;

    // c (interleaved RGB)
    buf->dim[2].min = 0;
    buf->dim[2].extent = 3;
    buf->dim[2].stride = 1;
}

gint main(gint argc, gchar **argv) {
    GeglBuffer *buffer;

    gegl_init(&argc, &argv);

    gint width = 1024;
    gint height = 1024;
    buffer = test_buffer(width, height, babl_format("R'G'B' u8"));

    struct halide_buffer_t raw_input, raw_output;

    uint8_t *raw_image_input = read_buffer_as_rgb_u8(buffer);
    make_buffer(raw_image_input, width, height, &raw_input);
    uint8_t *raw_image_output = alloc_buffer(width, height, babl_format("R'G'B' u8"));
    make_buffer(raw_image_output, width, height, &raw_output);

    int ret = halide_transform(&raw_input, &raw_output);
    g_assert_cmpint(ret, ==, 0);

    GeglBuffer *output = make_gegl_buffer(raw_image_output, width, height);

    bench("transform", buffer, &transform);

    free(raw_input.dim);
    free(raw_output.dim);
    g_free(raw_image_input);
    g_free(raw_image_output);

    return 0;
}

void transform(GeglBuffer *src_buffer) {
    GeglBuffer *dst_buffer;
    GeglNode *gegl, *source, *translate, *rotate, *scale, *shear, *border_align, *reflect, *sink;

    gegl = gegl_node_new();
    source = gegl_node_new_child(gegl, "operation", "gegl:buffer-source", "buffer", src_buffer, NULL);
    translate = gegl_node_new_child(gegl, "operation", "gegl:translate", "x", 200.0, "y", 100.0, NULL);
    rotate = gegl_node_new_child(gegl, "operation", "gegl:rotate", "degrees", 50.0, NULL);
    scale = gegl_node_new_child(gegl, "operation", "gegl:scale-ratio", "x", 2.0, "y", 1.0, NULL);
    // shear = gegl_node_new_child (gegl, "operation", "gegl:shear", "x", 2.0, "y", 1.0, NULL);
    // border_align = gegl_node_new_child (gegl, "operation", "gegl:border-align", "x", 2.0, "y", 1.0, "horizontal_margin", 1.0, "vertical_margin", 1.0, NULL);
    // reflect = gegl_node_new_child (gegl, "operation", "gegl:reflect", "x", 2.0, "y", 1.0, NULL);
    sink = gegl_node_new_child(gegl, "operation", "gegl:buffer-sink", "buffer", &dst_buffer, NULL);

    gegl_node_link_many(source, translate, rotate, scale, sink, NULL);
    // gegl_node_link_many (source, translate, rotate, scale, shear, border_align, reflect, sink, NULL);
    gegl_node_process(sink);
    g_object_unref(gegl);
    g_object_unref(dst_buffer);
}
