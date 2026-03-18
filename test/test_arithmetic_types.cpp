#include <array>
#include <cstddef>

#include <vulkan/vulkan.h>

#include <sirius/arith/vector.hpp>
#include <sirius/arith/size.hpp>
#include <sirius/arith/matrix.hpp>


int main(){
    //test vector & matrix
    constexpr acma::vector3<std::size_t> x{5, 13, 7};
    static_assert(x[0] == 5);
    constexpr VkOffset3D y = static_cast<VkOffset3D>(x);
    constexpr acma::size2u64 z = static_cast<acma::size2u64>(x);
    static_assert(y.x == 5 && y.y == 13 && y.z == 7);
    static_assert(z.width() == 5 && z.height() == 13);
    constexpr acma::pt2<float> x2 = static_cast<acma::pt2<float>>(x);
    constexpr acma::matrix<2,2,float> mat4_1 = {{std::array<float, 2>{{1.0f, 2.0f}}, std::array<float, 2>{{3.0f, 4.0f}}}};
    constexpr acma::matrix<2,2,float> mat4_2 = {{std::array<float, 2>{{3.0f, 4.0f}}, std::array<float, 2>{{2.0f, 1.0f}}}};
    constexpr acma::vector2<float> mat_mult_vec = {{mat4_1 * x2}};
    constexpr acma::vector2<float> vec_mult_mat = {{x2 * mat4_1}};
    constexpr acma::matrix<2,2,float> mat_mult_mat = mat4_1 * mat4_2;
    static_assert(mat_mult_vec[0] == 31);
    static_assert(mat_mult_vec[1] == 67);
    static_assert(vec_mult_mat[0] == 44);
    static_assert(vec_mult_mat[1] == 62);
    static_assert(mat_mult_mat[0][0] == 7);
    static_assert(mat_mult_mat[0][1] == 6);
    static_assert(mat_mult_mat[1][0] == 17);
    static_assert(mat_mult_mat[1][1] == 16);
}