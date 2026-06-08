#include "test-common.h"

#include <time.h>
#include <string.h>

void transform(const char *in, const char *out);
void transform_for_benchmark(GeglBuffer *buffer);
void benchmark();

#define TIME_BENCHMARK // run time benchmark

#define OUTPUT_BENCHMARK // run output image benchmark, IMAGE_IN and IMAGE_OUT must be set
#define IMAGE_IN "" // input image path
#define IMAGE_OUT "" // output image path

gint main(gint argc, gchar **argv) {
    gegl_init(&argc, &argv);

    #ifdef TIME_BENCHMARK
    benchmark();
    #endif

    #ifdef OUTPUT_BENCHMARK
    if (strlen(IMAGE_IN) > 0 && strlen(IMAGE_OUT) > 0) {
        transform(IMAGE_IN, IMAGE_OUT);
    }
    #endif

    gegl_exit();
    return 0;
}

void transform(const gchar* in, const gchar* out) {
    GeglNode *gegl, *load, *translate, *rotate, *scale, *save;

    gegl = gegl_node_new();
    load = gegl_node_new_child(gegl, "operation", "gegl:png-load", "path", in, NULL);

    // transform operations
    translate = gegl_node_new_child(gegl, "operation", "gegl:translate", "x", 200.0, "y", 100.0, NULL);
    rotate = gegl_node_new_child(gegl, "operation", "gegl:rotate", "degrees", 90.0, NULL);
    //scale = gegl_node_new_child(gegl, "operation", "gegl:scale-ratio", "x", 2.0, "y", 1.0, NULL);

    save = gegl_node_new_child(gegl, "operation", "gegl:png-save", "path", out, NULL);

    gegl_node_link_many(load, translate, rotate, save, NULL);
    gegl_node_process(save);
    g_object_unref(gegl);
}

void transform_for_benchmark(GeglBuffer *buffer) {
    GeglBuffer *buffer_sink;
    GeglNode *gegl, *source, *rotate, *sink;

    gegl = gegl_node_new();
    source = gegl_node_new_child (gegl, "operation", "gegl:buffer-source", "buffer", buffer, NULL);

    // transform operations
    rotate = gegl_node_new_child(gegl, "operation", "gegl:rotate", "degrees", 90.0, NULL);

    sink = gegl_node_new_child (gegl, "operation", "gegl:buffer-sink", "buffer", &buffer_sink, NULL);

    gegl_node_link_many(source, rotate, sink, NULL);

    gegl_node_process(sink);// process the sink node to execute the operations

    g_object_unref(gegl);
    g_object_unref(buffer_sink);
}

void benchmark() {
    GeglBuffer *buffer = test_buffer(1024, 512, babl_format("R'G'B' u8"));

    int iterations = 1000;
    int warmup = 10;
    double minTime = DBL_MAX, maxTime = 0.0, totalTime = 0.0;

    // warm-up
    for (int i = 0; i < warmup; ++i) {
        transform_for_benchmark(buffer);
    }

    // benchmark
    for (int i = 0; i < iterations; ++i) {
        struct timespec start, end;

        clock_gettime(CLOCK_MONOTONIC, &start);
        transform_for_benchmark(buffer);
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Calculate time in seconds as a double
        double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        totalTime += time;
        if (time < minTime) minTime = time;
        if (time > maxTime) maxTime = time;
    }

    double averageTime = totalTime / iterations;

    printf("Benchmark results (%d iterations):\n", iterations);
    printf("  Min Time:     %.6f s\n", minTime);
    printf("  Max Time:     %.6f s\n", maxTime);
    printf("  Average Time: %.6f s\n", averageTime);

    g_object_unref(buffer);
}
