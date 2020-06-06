## Билет 37
Автор: Глеб Марьин

### std::addressof

`operator&` можно перегрузить, чтобы точно взять адрес объкта нужно взять `std::addressof`

```C++
struct S { int operator&() const { return 7; } };
void some_c_api(const void*);
template<typename T>
void CallSomeCApi(const T &value) {
    some_c_api(static_cast<const void*>(&value));
}
// ....
S s;
CallSomeCApi(10);  // Компилируется.
CallSomeCApi(s);   // Почему ошибка компиляции внутри CallSomeCApi?
```

К счастью, там был static_cast, иначе бы оно передало число 7 как значение указателя.

Решение:

```C++
// Так в обобщённом коде правильно.
some_c_api(static_cast<const void*>(std::addressof(value)));
```

Упрощённая реализация:

```C++
template<typename T> T* addressof(T& value) {
    return reinterpret_cast<T*>(&reinterpret_cast<const char&>(value));
}
```

Вроде все, вот [ссылка на cppreference](https://en.cppreference.com/w/cpp/memory/addressof)


### Тип "функция"

Да, функции тоже имеют типы.

```C++
void func1();
void func2(int x);
void func3(int y);
int func4(int x, char y);
// ....
TD<decltype(func1)>{}; // Ошибка компиляции: TD<void()> is incomplete
TD<decltype(func2)>{}; // Ошибка компиляции: TD<void(int)> is incomplete
TD<decltype(func3)>{}; // Ошибка компиляции: TD<void(int)> is incomplete
TD<decltype(func4)>{}; // Ошибка компиляции: TD<int(int, char)> is incomplete
```

Особенности

- Объявить переменную такого типа нельзя.
- В метапрограммировании обычно применяется для упрощения синтаксиса: `function<void(int)>`.
- Неявно приводится к указателю на функцию.

### Тип "указатель на функцию"

Просто указатель на функцию, что сказать.
Добавили между типом и аргументами `(*)`, получили `void(*)(int)`.

```C++
char foo(int);
// ....
auto x = [](int val) -> char { return val; };
auto y = [&](int val) -> char { return val; };
char (*func1)(int) = &foo;  // Ок.
char (*func2)(int) = foo;   // Ок, функция неявно преобразуется в указатель на себя.
char (*func3)(int) = x;     // Ок: лямбда без захвата — почти свободная функция.
char (*func4)(int) = y;     // Не ок: лямбда с захватом должна знать своё состояние.
```

А ещё функции и данные могут вообще в совсем разной памяти лежать:

```C++
void *x = static_cast<void*>(&foo);  // Ошибка компиляции.
```

*Но Linux пофиг: там какие-то функции возвращают void\* вместо указателей на функции.*

Между собой указатели на функции несовместимы:

```C++
void (*func5)() = func4; // из примера выше
```

Вообще дальше вам скорее всего не нужно, но может быть в качестве
классного примера использования.

Можно написать `function_traits` для простых функций, для лямбд это не работает,
потому что они не матчатся под тип функций.

```C++
std::string f1(char c, int u) { return "Hello"; }
int f2(long long) { return 0; }
template <typename R, typename... Args>
struct function_traits {};
template <typename R, typename... Args>
struct function_traits<R(Args...)> { // вот тут матчим функцию
    using return_type = R;
    using argument_types = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
    template <std::size_t i>
    using arg = std::tuple_element_t<i, argument_types>;
};

template <typename T> struct TD {};

int main() {
    std::cout << function_traits<decltype(f1)>::arity << std::endl; // 2
    std::cout << function_traits<decltype(f2)>::arity << std::endl; // 1
    // TD<function_traits<decltype(f1)>::arg<0>>::me_please;           // char
    // TD<function_traits<decltype(f1)>::arg<1>>::me_please;           // int
}

```

С лямбдами сложнее, вот пример, но он точно не нужен: [stackoverflow](https://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda).

### Как взять указатель на перегруженную функцию и static_cast

Если у функции более одной перегрузки, то возникают пробелемы.

```C++
void foo(int x);
char foo(char x);
// ....
TD<decltype(&foo)>{};  // Ошибка компиляции: какую перегрузку выбрать decltype?
```

Вывод: у &foo нет типа (примерно как у {1, 2, 3}).

```C++
void (*func1)(int)  = &foo;  // Если в точности указать тип, то перегрузка разрешится.
char (*func2)(char) = &foo;
```

Вызывает проблемы в шаблонных функциях или с auto:

```C++
template<typename Fn> void bar(Fn fn) { fn(10); }
// ....
bar(&foo);  // no matching function for call to 'bar(<unresolved overloaded function>)'
bar(static_cast<void(*)(int)>(&foo));  // Ок, потому что указатели на разные функции разных типов не кастуются.
bar([](int x) { return foo(x); });     // Ок, у лямбды фиксированный параметр.
```

### Указатель на поле класса

Нам когда-то в интрузивном списке нужно было узнавать o том, на сколько
байт определенный член класса выпирает относительно начала структуры,
тогда мы использовали `offsetof`, в плюсах есть типобезопасный и более
красивый способ это делать. На самом деле не очень красивый.

```C++
struct S { int x = 1, y = 2; };
S s;
S *sptr = &s;
int (S::*a) = &S::x;  // Можно даже без скобок слева.
int (S::*b) = &S::y;  // А вот &(S::y) уже нельзя.
assert(s.*a == 1);
assert(sptr->*a == 1);

a = b;  // Теперь `a` "указывает" на поле `y`.
assert(s.*a == 2);
assert(sptr->*a == 2);

// Но вообще рекомендуют typedef или using:
using IntMemberOfS = int S::*;
IntMemberOfS c = &S::x;

// С наследниками тоже работает.
struct S2 : S { int z = 3; };
S2 s2;
assert(s2.*a == 2);  // Можно пользоваться указателем из родителя.
int (S2::*d) = a;  // Можно неявно преобразовать типы.
assert(s2.*d == 2);
IntMemberOfS e = static_cast<IntMemberOfS>(d);  // Можно явно.
```

### Указатель на метод класса

Синтаксис аналогичный:

```C++
struct S {
    int field = 100;
    int foo1(int x) { return x + 10 + field; }
    int foo2(int x) { return x + 20 + field; }
    int bar1(int x) const { return x + 30 + field; }
    int bar2(int x) const { return x + 40 + field; }
};
S s;
int (S::*a)(int) = &S::foo1;
assert((s.*a)(9) == 119);  // Сначала скобки вокруг .*!
a = &S::foo2;
assert((s.*a)(8) == 128);
```

Важно сохранить const-qualifier:

```C++
int (S::*b)(int) const = &S::bar1;  
assert((s.*b)(7) == 137);
b = &S::bar2;
assert((s.*b)(6) == 146);
```

Есть все стандартные проблемы с выбором перегрузки.

Можно добиться этой проблемы например так:

```C++
struct S {
    int foo1(char x) { return x + 15; }
    int foo1(int x) { return x + 15; }
};

int main() {
    S s;
    // auto b = &S::foo1; error: unable to deduce ‘auto’ from ‘& S::foo1’
    int (S::*a)(int) = &S::foo1;  // OK
    int (S::*b)(char) = &S::foo1; // OK
}
```
