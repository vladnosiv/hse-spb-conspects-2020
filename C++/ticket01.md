## Билет 01 "Детали классов"
Автор: Герман Тарабонда

### Корректное объявление глобальные констант

Круче пользоваться `constexpr`, нежели `const`, потому что лучше производительность, никаких race condition. Не ускоряет вычисления в compile-time, но требует возможность. Например:

```C++
constexpr b = 3; // ОК
constexpr c = f(1); // ОК, если f -- может быть вычислена во время компиляции
```

Будет больно, со сложными объектами, потому что нельзя инициализировать на этапе компиляции: будут беды с порядком инициализации разных единиц трансляций.

Если `const` вне класса, то он будет виден только в своей единице трансляции. При подключении хедера с константой в другие файлы произойдет копирование константы. Если мы хотим сохранить ссылки, то лучше `constexpr`. Например:

```C++
// inc.h
const std::string Flag = "--file"
// fst.cpp
#include "inc.h"
    ...&Flag... // (1) своя копия
// snd.cpp
#include "inc.h"
    ...&Flag... // (2) своя копия
// ссылка (1) != ссылке (2),
// чтобы были одинаковые пишем: constexpr char [] Flag = "--file"
```

### Корректное объявление статических констант-членов

Если у нас статические константы-члены классов, то константы видны везде и могут произойти беды, если мы захотим взять ссылку на константы:

```C++
// foo.h
struct Foo {
    static const int N = 60; // объявление
    static constexpr char Name[] = "NAME"; // объявление (аналогичное поведение до C++17)
}
// fst.cpp
... Foo::N ... // ОК
... Foo::Name[0] ... // ОК
... &Foo::N ... // может быть undefined reference на этапе линковки
... &Foo::Name[0] ... // может быть undefined reference на этапе линковки
// snd.cpp
... Foo::N ... // ОК
... Foo::Name[0] ... // ОК
... &Foo::N ... // может быть undefined reference на этапе линковки
... &Foo::Name[0] ... // может быть undefined reference на этапе линковки
```

В чем проблема? У константы должно быть и объявление, и должно быть определение ровно в одной единицы трансляции. Для решения проблемы добавляем определение констант.

```C++
// foo.h
struct Foo {
static const int N = 60; // объявление
static constexpr char Name[] = "NAME"; // объявление (аналогичное поведение до C++17)
}
// fst.cpp
const int Foo::N; // инициализация и слово `statuc` не нужны
constexpr char Name[]; // в заголовочном файле не определяем, иначе будет UB на этапе компиляции
... Foo::N ... // ОК
... Foo::Name[0] ... // ОК
... &Foo::N ... // ОК
... &Foo::Name[0] ... // ОК
// snd.cpp
... Foo::N ... // ОК
... Foo::Name[0] ... // ОК
... &Foo::N ... // ОК
... &Foo::Name[0] ... // ОК
```

Самое важное: **если можем, то ставим constexpr, а в заголовках еще пишем inline**.

```C++
struct Foo {
    static inline const std::string NO = "NO"
    static inline constexpr char YES[] = "YES"
}
inline const std::string NO = "NO"
inline constexpr char YES[] = "YES"
```

*C C++17 inline можно писать у констант и у переменных*

### Перегрузка `operator->`

Мы можем ее перегружать (вау). Это унарный оператор, а не бинарный (важно). Для перегрузки мы должны возвращать то, к чему мы можем применить стрелочку. Например:

```C++
template<typename T>
struct unique_ptr {
T* data;
// ....

// Унарный оператор *
      T& operator*()       { return *data; }
const T& operator*() const { return *data; }

// "Бинарный" оператор ->, второй аргумент - метод.
// Так что на самом деле унарный.
      T* operator->()       { return data; }
const T* operator->() const { return data; }
};
// ....
unique_ptr<std::string> s = ....;
s->size();
// На самом деле у нас выходит цепочка из вызовов operator->
// auto x = s.operator->(); x->size();
```

Получается, мы можем сделать и такую перегрузку:

```C++
struct Foo {
    auto operator->() {
        return std::make_unique<std::string>("hello");
    }
} f;
f->size();  // размер "hello" = 5; время жизни unique_ptr — до конца full expression (;)
// auto x = f.operator->(); x->size();
```

### Функции-друзья

Функция-друг позволяет пользоваться закрытой частью другого класса. Например:

```C++
class matrix;
class vector {
    int v[4];
    ...
    friend vector multiply(const matrix&, const vector&); // имеет доступ к vector и массиву v
}; 

class matrix { 
    vector v[4]; 
    ... 
    friend vector multiply(const matrix&, const vector&); // имеет доступ к matrix и массиву v 
}; 

### friend class

Если мы хотим в классе воспользоваться приватной функцией другого класса, то она должна стать его другом. При этом мы не можем отдельно обозвать функцию другом. Синтаксис такой:

```C++
class Bar;
class Foo {
    friend class Bar; // не можем сказать, что конкретный метод класса Bar -- друг
    void privateFunc();
};
class Bar {
    void bar() {
        Foo f;
        f.privateFunc(); // ОК
    }
};
class Baz {
    void bar() {
        Foo f;
        f.privateFunc(); // не ОК
    }
};
```

### `using` для методов и полей: method hiding, изменение видимости

Правило такое: если в наследнике есть хотя бы один метод с именем `foo`, то в родителей мы уже не смотрим (даже если у нас override), то есть по умолчанию у нас method hiding.

```C++
struct Foo {
virtual int foo(int) { return 1; }
virtual int foo(double) { return 2; }
}
struct Bar: Foo {
void foo(int) override { return 3; }
}

Foo f;
std::cout << f.foo(1) << std::endl; // 1, смотрим на функции Foo
std::cout << f.foo(1.2) << std::endl; // 2, смотрим на функции Foo
Bar b;
std::cout << b.foo(1) << std::endl; // 3, не смотрим в родителя
std::cout << b.foo(1.2) << std::endl; // 3, не смотрим в родителя
Foo& b2 = b;
std::cout << b2.foo(1) << std::endl; // 3, смотрим на функции Foo,
// но так как foo(int) виртуальная и ссылается на Bar, то выводим 3
std::cout << b2.foo(1.2) << std::endl; // 2, смотрим на функции Foo
```

Если добавить `using`, то мы подключим все имена методов в текущий класс:

```C++
struct Foo {
    void foo(int);
};
struct Bar : Foo {
    // using Foo::foo;
    void foo(double);
};
Foo f;
f.foo(1.2); // int
f.foo(1); // int
Bar b;
b.foo(1.2); // double
b.foo(1); // double, после using - int.
b.Foo::foo(1); // int
Foo &b2 = b;
b2.foo(1.2); // int
b2.foo(1); // int
```

### `using` для методов и полей: использование для упрощения конструкторов

Зачем писать так:

```C++
struct my_exception : std::runtime_error {
    my_exception(const std::string& what_arg) : std::runtime_error(what_arg) {}
    my_exception(const char*        what_arg) : std::runtime_error(what_arg) {}
};
// ...
throw my_exception("Foo");
// ...
```

Если можно так:

```C++
struct my_exception : std::runtime_error {
    using std::runtime_error::runtime_error; // вместо нескольких конструкторов 1 строчка
};
// ...
throw my_exception("Foo");
// ...
```

С помощью `using` мы указываем компилятору, что мы собираемся наследовать конструкторы базового класса. И компилятор тогда сгенерирует код выполнения наследования и пересылки производного класса в базовый класс.
