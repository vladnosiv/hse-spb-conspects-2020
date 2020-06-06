#include <utility>
#include <iostream>

//#define C1
//#define C2
//#define C3
//#define C4
//#define C5
//#define C6
//#define C7
//#define C8
//#define C9

template <typename T> struct TD;

template <typename T> struct Foo{};

template <typename T>
T&& forward(T&& t) {
    return static_cast<T&&>(t);
}

template<typename T> void f(T&& t) {
        TD<T> s1;
        TD<decltype(t)> s2;
#ifdef C5
        TD<decltype(forward<T>(t))> s3;
#endif
};

template <typename T> void fFoo(Foo<T> t) {
    TD<T> s1;
    TD<Foo<T>> s2;
    TD<decltype(t)> s3;
}

template<typename T> void f2(const T* v) {
    TD<T> s;
    TD<decltype(v)> s2;
};
        
template <typename T>
auto foofoo(T&&) -> T&& {}
       
#ifdef C8
auto ret_brace() { return {1,2,3}; }
#endif

#ifdef C9
    int ar[10];
    decltype(auto) declauto_foo1() {
        int x = 1;
        return x;
    }
    decltype(auto) declauto_foo2() {
        return ar[0];
    }
    decltype(auto) declauto_foo3() {
        int x = 1;
        return (x);
    }
#endif

template<typename T> void g(const T&) {};

int main() {
#ifdef C1
    {
            int    x = 10;//   f(x);           // T =       int, PARAM=      int&
      const int   cx = 20;  f(cx);           // T = const int, PARAM=const int&
            int  &rx =  x;  f(rx);           // T =       int, PARAM=      int&
      const int &crx =  x; f(crx);           // T =       int, PARAM=const int&
            const int * const ccx = &x; f2(ccx);
      
            g(x);                               // T = int, PARAM=const int&
    }
#endif


#ifdef C2
    {
            int x = 10;
            const int cx = x;
            int& rx = x;
            const int& crx = x;
            
                  auto&  rx_a1 =   x;  // auto =       int =>       auto& =       int&
                  auto&  rx_a2 =  cx;  // auto = const int =>       auto& = const int&
            const auto& crx_a3 =  rx;  // auto =       int => const auto& = const int&
            const auto& crx_a4 = crx; // auto  =       int => const auto& = const int&

//            TD<decltype(crx_a4)> s;
    }
#endif
#ifdef C3
    {
            int    x = 10;//   f(x);           // T =       int, PARAM=      int&
            f(x);
            f(std::move(x));
            f(std::move(Foo<decltype(std::move(x))>{}));
            TD<decltype(std::move(x))> s;
    }
#endif
#ifdef C4
    {
            int xx = 10;
            Foo<int>    x;//   f(x);           // T =       int, PARAM=      int&
            fFoo(x);
            fFoo(std::move(x));
            fFoo(std::move(Foo<decltype(std::move(xx))>{}));
            TD<decltype(std::move(x))> s;
    }
#endif
#ifdef C5
    {
            int x = 10;
            int& rx = x;
            const int cx = x;
            const int& crx = cx;
#ifdef C5_
            f(x);
            f(rx);
            f(std::move(x));
            f(std::move(rx));
#endif
//            f(cx);
//            f(std::move(cx)); // wtf move from const int
            f(crx);
            f(std::move(crx)); // wtf move from const int&
//            TD<decltype(std::move(x))> s;
    }
#endif

#ifdef C6
    {
        int xx = 10;
        auto lam = [&x = xx] () {x++;};
        lam();
        std::cout << xx << "\n";

        auto lam2 = [] () -> int { return 0.5; };
        std::cout << lam2() << "\n";

#ifdef C6_
        TD<decltype(foofoo(xx))> s1; // TD<int&>
        TD<decltype(foofoo(std::move(xx)))> s2; //TD<int&&>
#endif
    }
#endif

#ifdef C7
    {
        auto x1 {1,2,3};
        auto x2 = {1,2,3};
        auto x3 {1};
        auto x4 = {1};

        TD<decltype(x1)> s1;
        TD<decltype(x2)> s2;
        TD<decltype(x3)> s3;
        TD<decltype(x4)> s4;
    }
#endif

#ifdef C8
    {
        auto lam = [](){return {1,2,3}};
    }
#endif

#ifdef C9
    {
         TD<decltype(declauto_foo1())> s1; // TD<int>
         TD<decltype(declauto_foo2())> s2; // TD<int&>
         TD<decltype(declauto_foo3())> s3; // TD<int&>
    }
#endif
}

