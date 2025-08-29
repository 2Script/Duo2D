#include <numbers>

#include <Duo2D/arith/axis.hpp>
#include <Duo2D/arith/vector.hpp>
#include <Duo2D/arith/point.hpp>
#include <Duo2D/arith/matrix.hpp>


constexpr double sqrt_newton (double x, double curr, double prev) { return curr == prev ? curr : sqrt_newton(x, 0.5 * (curr + x / curr), curr); }


int main(){
    constexpr d2d::vector3<std::size_t> x{5, 13, 7};
    constexpr d2d::pt2<float> x2 = static_cast<d2d::pt2<float>>(x);


    //test vector transformations 
    constexpr static auto sin90 = [](double){ return 1.0; };
    constexpr static auto cos90 = [](double){ return 0.0; };
    constexpr static auto tan45 = [](double){ return 1.0; };
    constexpr static auto sinn90 = [](double){ return -1.0; };
    constexpr static auto cosn90 = [](double){ return 0.0; };
    constexpr static auto sqrt = [](float x){ return sqrt_newton(x, x, 0); };
    constexpr auto xr = d2d::rotate{std::numbers::pi / 2, sin90, cos90}(x2);
    constexpr auto xs = d2d::scale{{10.f,10.f}}(x2);
    constexpr auto xss = d2d::scale{10.f,10.f}(xs);    
    constexpr auto xsr = d2d::rotate{-std::numbers::pi / 2, sinn90, cosn90}(xs);
    constexpr auto xsrt = d2d::translate{{30.f,5.f}}(xsr);
    static_assert(xr[0] == -13 && xr[1] == 5);
    static_assert(xs[0] == 50 && xs[1] == 130);
    static_assert(xss[0] == 500 && xss[1] == 1300);
    static_assert(xsr[0] == 130 && xsr[1] == -50);
    static_assert(xsrt[0] == 160 && xsrt[1] == -45);
    //auto xre = d2d::rotate{std::numbers::pi / 2, sin90, cos90}(xsrt); //error
    //auto xse = d2d::scale{10.f}(xs); //error
    constexpr decltype(x2) y2 = static_cast<decltype(x2)>(xsrt);
    constexpr auto yt = d2d::translate{-160.f, 45.f}(y2);
    static_assert(y2[0] == 160 && y2[1] == -45);
    static_assert(yt[0] == 0 && yt[1] == 0);
    constexpr decltype(x2) zss = d2d::transform(x2, d2d::scale{10.f,10.f}, d2d::scale{10.f,10.f});
    constexpr decltype(x2) ztt = d2d::transform(x2, d2d::translate{10.f,10.f}, d2d::translate{10.f,10.f});
    constexpr decltype(x2) zsrt = d2d::transform(x2, d2d::translate{-160.f, 45.f}, d2d::translate{{30.f,5.f}}, d2d::scale{10.f,10.f}, d2d::rotate{-std::numbers::pi / 2, sinn90, cosn90});
    static_assert(ztt[0] == 25 && ztt[1] == 33);
    static_assert(zsrt[0] == 0 && zsrt[1] == 0);
    static_assert(zss[0] == 500 && zss[1] == 1300);
    //constexpr auto zt1 = d2d::transform(x2, d2d::translate{{30.f,5.f}});
    constexpr d2d::point<4, float> x4 = {10,10,10,1};
    constexpr d2d::point<4, float> x4st = d2d::transform(x4, d2d::translate{10.f,0.f,0.f}, d2d::scale{2.f,2.f,2.f});
    constexpr d2d::vk_mat4 psp = d2d::vk_mat4::perspective(90, 1280, 720, tan45, 0.1f, 10.f);
    constexpr d2d::vk_mat4 la = d2d::vk_mat4::looking_at({2,2,2}, {0,0,0}, d2d::axis::z, d2d::axis_direction::positive, sqrt);
    static_assert(x4st[0] == 30 && x4st[1] == 20 && x4st[2] == 20 && x4st[3] == 1);
    //static_assert(psp[0][0] == .5625f && psp[1][1] == 1 && psp[2][2] == 1.010108e-02f && psp[3][2] == 1.0101011e-01f && psp[2][3] == -1);
    //static_assert(la[0][0] == -707106.76e-6f && la[0][1] == -408248.27e-6f && la[0][2] == 57735.025e-5f && la[0][3] == 107600.95e-5f);
    //static_assert(la[1][0] == 707106.76e-6f  && la[1][1] == -408248.27e-6f && la[1][2] == 57735.025e-5f && la[1][3] == -175241.756e-5f);
    //static_assert(la[2][0] == 0              && la[2][1] == 816496.55e-6f  && la[2][2] == 57735.025e-5f && la[2][3] == -278769.35e-5f);
    //static_assert(la[3][0] == 0 && la[3][1] == 0 && la[3][2] == 0 && la[3][3] == 1);
    //static_assert(la[0][0] == -707106.76e-6f && la[0][1] == -408248.27e-6f && la[0][2] == 57735.025e-5f && la[0][3] == 0);
    //static_assert(la[1][0] == 707106.76e-6f  && la[1][1] == -408248.27e-6f && la[1][2] == 57735.025e-5f && la[1][3] == 0);
    //static_assert(la[2][0] == 0              && la[2][1] == 816496.55e-6f  && la[2][2] == 57735.025e-5f && la[2][3] == -34641.015e-4f);
    //static_assert(la[3][0] == 0              && la[3][1] == 0              && la[3][2] == 0             && la[3][3] == 1);



    //test vector operations
    constexpr d2d::vec3<float> v1 = {1,2,3};
    constexpr d2d::vec3<float> v2 = {4,5,6};
    constexpr d2d::vec3<float> vc = d2d::cross(v1, v2);
    static_assert(vc[0] == -3 && vc[1] == 6 && vc[2] == -3);
    constexpr d2d::vec3<float> v1n = d2d::normalized(v1, sqrt);
    constexpr d2d::vec3<float> v2n = d2d::normalized(v2, sqrt);
    constexpr float sqrt_14 = sqrt(14);
    constexpr float sqrt_77 = sqrt(77);
    static_assert(v1n[0] == 1/sqrt_14 && v1n[1] == 2/sqrt_14 && v1n[2] == 3/sqrt_14);
    static_assert(v2n[0] == 4/sqrt_77 && v2n[1] == 5/sqrt_77 && v2n[2] == 6/sqrt_77);
    constexpr float vd = d2d::dot(v1, v2);
    static_assert(vd == 32);



    return 0;
}