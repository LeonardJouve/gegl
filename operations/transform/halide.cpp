#include "Halide.h"
#include <string>

using namespace Halide;

int main(int argc, char **argv) {
    std::string out_dir = (argc > 1) ? argv[1] : ".";

    ImageParam input(UInt(8), 3, "input"); // x,y,c
    ImageParam matrix(Float(32), 2, "matrix"); // 3x3

    Var x, y, c;

    Func out;

    Expr r = cast<float>(input(x, y, 0));
    Expr g = cast<float>(input(x, y, 1));
    Expr b = cast<float>(input(x, y, 2));

    Expr result[3];

    result[0] =
        matrix(0, 0) * r +
        matrix(0, 1) * g +
        matrix(0, 2) * b;

    result[1] =
        matrix(1, 0) * r +
        matrix(1, 1) * g +
        matrix(1, 2) * b;

    result[2] =
        matrix(2, 0) * r +
        matrix(2, 1) * g +
        matrix(2, 2) * b;

    out(x, y, c) = cast<uint8_t>(clamp(
        select(
            c == 0,
            result[0],
            c == 1,
            result[1],
            result[2]
        ),
        0.0f,
        255.0f
    ));

    Target target = get_host_target();

    std::string filename = out_dir + std::string("/halide_transform");
    std::string function_name = "halide_transform";

    out.compile_to_object(
        filename + ".o",
        {input, matrix},
        function_name,
        target
    );

    out.compile_to_header(filename + ".h", {input, matrix}, function_name, target);

    return 0;
}
