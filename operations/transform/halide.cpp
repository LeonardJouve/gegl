#include "Halide.h"
#include <string>

using namespace Halide;

int main(int argc, char **argv) {
    std::string out_dir = (argc > 1) ? argv[1] : ".";

    Var x, y;

    ImageParam input(UInt(8), 2);

    Func out;
    out(x, y) = input(x, y);

    Target target = get_host_target();

    std::string filename = out_dir + std::string("/halide_transform");
    std::string functionname = "halide_transform";

    out.compile_to_object(
        filename + ".o",
        {input},
        functionname,
        target
    );

    out.compile_to_header(filename + ".h", {input}, functionname, target);

    return 0;
}
