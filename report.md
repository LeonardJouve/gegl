Members of the group:

- Christophe Künzli
- Léonard Jouve

# Introduction

During this project, we are requested to analyze the performances of part of an open source project, and propose some
optimizations. We chose the [gegl](https://gegl.org/) project ([repo](https://gitlab.gnome.org/GNOME/gegl/)), which is
an image processing library used by GIMP. We focused on its transformation operations (see `operations/transform`),
and more precisely on the`gegl_operation_transform` operation, which is used to perform one or multiple affine
transformations on images, such as scaling, rotation, and shearing. We chose this operation because it is widely used in
image processing, and it is a good candidate for optimization, as it involves a lot of computations.

# Build project

Gegl uses meson to build the project and manage dependencies.

All listed dependencies can be found in root `meson.build` file.
Most of them can be installed via your platform package manager.

One of the dependencies is `babl` ([repo](https://gitlab.gnome.org/GNOME/babl)). It is another GNOME project used as a
pixel encoding and color space conversion engine.

It can be installed by following the `INSTALL.in` instructions.

Each dependency must include a `.pc` file in `PKG_CONFIG_PATH` to be found by meson.

```
meson setup buildDir
meson compile -C buildDir -j6
```

# Benchmark

We have written a simple benchmark that runs a transform operation 1000 times and computes average time spent on the
operation, along with min and max times.

<details>
  <summary>Click here to display benchmark code</summary>

```c++
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
```

</details>

How to run benchmark :

```
# Run all tests
meson test -C buildDir

# Run all benchmarks
meson test -C buildDir --benchmark

# Run only our benchmark :
meson test -C buildDir --benchmark 'Perf Test transform'     
```

**Important notes** :

- You can switch between the original implementation and our implementation by opening the `
  operations/transform/transform-core.c` file and commenting/uncommenting `#define TRANSFORM_HALIDE` and rebuilding the
  project. If `TRANSFORM_HALIDE` is defined, the benchmark will use our implementation, otherwise it will use the
  original implementation.
- The benchmark can also be used to perform a predefined transformation on an actual image and save it to disk. To
  use this feature, you need to open the `perf/test-transform.c` file and set IMAGE_IN and IMAGE_OUT to the path of the
  input image and the output image respectively. Then, rebuild the project and run the benchmark as shown above. The
  output image will be saved to the specified path.

# Analysis

Our first step was to analyze the code of the `gegl_operation_transform` operation, and identify the parts that are
the most computationally expensive. We wrote a simple benchmark in order to measure the time of the operation and also
generate a flame graph to find any bottlenecks using VTune.

## Flame graph

![Flame graph](flamegraph.png)

As we can see, ... (TODO)

# Proposed optimization

## Halide

[Halide](https://halide-lang.org/) is a domain-specific language for image processing and computational photography. It
is designed to make it easier to write high-performance image processing code, and we will use to see if it performs
better than the current implementation of `gegl_operation_transform`.

## Implementation

see `operations/transform/halide.cpp` and `operations/transform/transform-core.c`

## Comparison with current implementation

# Difficulties encountered

- Understanding and building the gegl project.
- Understanding affine transformations and how they are implemented in the current codebase.
- Learning how to use Halide and how to integrate it into the existing codebase.

# Conclusion

