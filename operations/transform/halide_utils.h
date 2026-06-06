#ifndef HALIDE_TRANSFORM_HALIDE_UTILS_H
#define HALIDE_TRANSFORM_HALIDE_UTILS_H

#include <gegl.h>
#include "halide_transform.h"

uint8_t *alloc_u8_buffer(gint width, gint height, const Babl *fmt);

uint8_t *gegl_buffer_to_u8(GeglBuffer *buf);

void u8_buffer_to_gegl(uint8_t *pixels, gint width, gint height, GeglBuffer *buf);

void alloc_halide_buffer(uint8_t *data, int32_t width, int32_t height, struct halide_buffer_t *buf);

void free_halide_buffer(struct halide_buffer_t *buf);

void alloc_halide_matrix(GeglMatrix3 *matrix, struct halide_buffer_t *buf);

#endif
