#include <gegl.h>

#include "halide_transform.h"

// returns allocated data, caller frees
uint8_t *alloc_u8_buffer(gint width, gint height, const Babl *fmt)
{
    gint bpp = babl_format_get_bytes_per_pixel(fmt); // bytes per pixel
    gsize alloc_size = (gsize) width * (gsize) bpp * (gsize) height;
    uint8_t *data = g_malloc0(alloc_size);

    return data;
}

// returns allocated data, caller frees
uint8_t *gegl_buffer_to_u8(GeglBuffer *buf)
{
    const Babl *fmt = gegl_buffer_get_format(buf);
    gint width = gegl_buffer_get_width(buf);
    gint height = gegl_buffer_get_height(buf);
    gint bpp = babl_format_get_bytes_per_pixel(fmt);
    gint rowstride = GEGL_AUTO_ROWSTRIDE;

    gsize alloc_size = (gsize) width * (gsize) bpp * (gsize) height;
    uint8_t *data = g_malloc0(alloc_size);

    GeglRectangle rect = {0, 0, width, height};

    gegl_buffer_get(buf, &rect, 1.0, fmt, data, rowstride, GEGL_ABYSS_CLAMP);

    return data;
}

void u8_buffer_to_gegl(uint8_t *pixels, gint width, gint height, GeglBuffer *buf)
{
    const Babl *fmt = gegl_buffer_get_format(buf); /* 3 channels, 8-bit interleaved */
    gint bpp = babl_format_get_bytes_per_pixel(fmt);
    gint rowstride = width * bpp; /* bytes per row */

    GeglRectangle rect = { 0, 0, width, height };

    /* Write the contiguous pixel block into the buffer.
       gegl_buffer_set will handle format conversion where necessary. */
    gegl_buffer_set(buf, &rect, 0, fmt, pixels, rowstride);
}

// returns buffer, caller frees dim
void alloc_halide_buffer(uint8_t *data, int32_t width, int32_t height, struct halide_buffer_t *buf) {
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
    buf->dim[1].stride = width;
}

void free_halide_buffer(struct halide_buffer_t *buf) {
    free(buf->dim);
}
