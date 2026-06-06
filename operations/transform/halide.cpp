#include "Halide.h"
#include <string>

using namespace Halide;

int main(int argc, char **argv) {
    ImageParam input(UInt(8), 3, "input");
    ImageParam matrix(Float(32), 2, "matrix");
    Param<int> roi_x{"roi_x"};
    Param<int> roi_y{"roi_y"};

    Var x, y, color;

    Func transform;

    Expr a = matrix(0,0);
    Expr b = matrix(1,0);
    Expr c = matrix(2,0);
    Expr d = matrix(0,1);
    Expr e = matrix(1,1);
    Expr f = matrix(2,1);
    Expr det = a*e - b*d;
    Expr ia = e / det;
    Expr ib = -b / det;
    Expr ic = (b*f - e*c) / det;
    Expr id = -d / det;
    Expr ie = a / det;
    Expr if_ = (d*c - a*f) / det;
    //print(a,b,c,d,e,f);

    Expr xf = cast<float>(x + roi_x) + 0.5f; // 0.5 | 1.5 | 2.5
    Expr yf = cast<float>(y + roi_y) + 0.5f; // 0.5

    Expr u = // 0.5 | 0.5 | 0.5
        ia * xf + // 0 | 0 | 0
        ib * yf + // 0.5 | 0.5 | 0.5
        ic; // 0 | 0 | 0
    Expr v = // -0.5 (should be 0.5) | -1.5 (should be 1.5) | -2.5 (should be 2.5)
        id * xf + // -0.5 | -1.5 | -2.5
        ie * yf + // 0 | 0 | 0
        if_; // 0 | 0 | 0

    Expr sx = clamp(cast<int>(floor(u)), 0, input.dim(1).extent() - 1); // 0
    Expr sy = clamp(cast<int>(floor(v)), 0, input.dim(2).extent() - 1); // 0

    transform(color,x, y) = input(color, sx, sy);

    Target target = get_host_target();

    std::string out_dir = (argc > 1) ? argv[1] : ".";
    std::string filename = out_dir + std::string("/halide_transform");
    std::string function_name = "halide_transform";

    transform.compile_to_object(filename + ".o", {input, matrix, roi_x, roi_y}, function_name, target);

    transform.compile_to_header(filename + ".h", {input, matrix, roi_x, roi_y}, function_name, target);

    return 0;
}
