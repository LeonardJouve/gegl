#include "test-common.h"

void transform(GeglBuffer *buffer);

gint
main (gint    argc,
      gchar **argv)
{
  GeglBuffer *buffer;

  gegl_init (&argc, &argv);

  buffer = test_buffer (4096, 4096, babl_format ("R'G'B' u8"));
  bench ("transform", buffer, &transform);

  return 0;
}

void transform(GeglBuffer *src_buffer)
{
  GeglBuffer *dst_buffer;
  GeglNode   *gegl, *source, *translate, *rotate, *scale, *shear, *border_align, *reflect, *sink;

  gegl = gegl_node_new ();
  source = gegl_node_new_child (gegl, "operation", "gegl:buffer-source", "buffer", src_buffer, NULL);  
  translate = gegl_node_new_child (gegl, "operation", "gegl:translate", "x", 200.0, "y", 100.0, NULL);
  rotate = gegl_node_new_child (gegl, "operation", "gegl:rotate", "degrees", 50.0, NULL);
  scale = gegl_node_new_child (gegl, "operation", "gegl:scale-ratio", "x", 2.0, "y", 1.0, NULL);
  // shear = gegl_node_new_child (gegl, "operation", "gegl:shear", "x", 2.0, "y", 1.0, NULL);
  // border_align = gegl_node_new_child (gegl, "operation", "gegl:border-align", "x", 2.0, "y", 1.0, "horizontal_margin", 1.0, "vertical_margin", 1.0, NULL);
  // reflect = gegl_node_new_child (gegl, "operation", "gegl:reflect", "x", 2.0, "y", 1.0, NULL);
  sink = gegl_node_new_child (gegl, "operation", "gegl:buffer-sink", "buffer", &dst_buffer, NULL);

  gegl_node_link_many (source, translate, rotate, scale, sink, NULL);
  // gegl_node_link_many (source, translate, rotate, scale, shear, border_align, reflect, sink, NULL);
  gegl_node_process (sink);
  g_object_unref (gegl);
  g_object_unref (dst_buffer);
}
