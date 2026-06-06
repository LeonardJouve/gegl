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

# Analysis

Our first step was to analyze the code of the `gegl_operation_transform` operation, and identify the parts that are
the most computationally expensive. We wrote a simple benchmark in order to measure the time of the operation and also
generate a flame graph to find any bottlenecks.

## Flame graph

![Flame graph](flamegraph.png)

As we can see, ... (TODO)

## Benchmark

Run the benchmark :`meson test -C buildDir --benchmark 'Perf Test transform'`

# Proposed optimization

## Halide

[Halide](https://halide-lang.org/) is a domain-specific language for image processing and computational photography. It
is designed to make it easier to write high-performance image processing code, and we will use to see if it performs
better than the current implementation of `gegl_operation_transform`.

## Implementation

## Comparison with current implementation

# Difficulties encountered

- Understanding and building the gegl project.
- Understanding affine transformations and how they are implemented in the current codebase.
- Learning how to use Halide and how to integrate it into the existing codebase.

# Conclusion