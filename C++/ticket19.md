## Билет 19
Автор: Глеб Марьин

### Явные (explicit) и неявные (implicit) приведения типов

Как я понимаю, и как на консультации рассказал Егор: явные преобразования -
когда явно пишем круглые, или фигурные скобки, зависит от вызываемого конструктора,
то есть [direct initialization (cppreference)](https://en.cppreference.com/w/cpp/language/direct_initialization). Явным также является static_cast, например.
Если же пишем равно, или принимаем в функцию, или возвзращаем из функции, то
происходит неявное преобразование типа, то есть происходит [copy initialization (cppreference)](https://en.cppreference.com/w/cpp/language/copy_initialization#:~:text=The%20effects%20of%20copy%20initialization,destination%20object%3A%20see%20copy%20elision).

Определения direct и copy из конспекта, эти виды важны в этом билете.

Direct initialization: T t(foo, bar), T(foo, bar), static_cast\<T>(foo, bar), new T(foo, bar), : member(foo, bar).
Вызов конструктора T с параметрами foo и bar.
В C++11 добавили: T t{foo, bar};, T{foo, bar}, new T{foo, bar}, member{foo, bar}.
Copy initialization: T t = ..;, T t = {..}, f(t), return .., throw ...
Вызов не-explicit конструктора T с параметрами.
В C++11 можно делать несколько параметров.

Примеры:

```C++
struct Point {
    int x, y;
    Point() : x(), y() /* value initialization */ {}
    Point(int x_, int y_) : x(x_), y(y_) /* direct initialization */ {}
};
Point foo(Point p) { return {p.y, p.x} /* copy initialization */; }
int main() {
    foo(Point(10, 20));  // direct initialization
    foo({10, 20});       // copy initialization
    Point p1{10, 20};    // direct initialization
    Point p2 = {10, 20}; // copy initialization
}
```

### Оператор приведения типа, конструктор приведения

Можно приводить типы двумя способами:

- Сделать конструктор от любого другого типа

```C++
struct From { ... };
struct To {
    From(const From &) { ... }
};
```

В таком случае получится конструктор приведения типа.

- Сделать оператор приведения

```C++
struct To { ... };
struct From {
    operator To() { ... }
}
```

Получится оператор приведения типа.

Эти способы абсолютно симметричны, каждая команда выбирает способ приведения,
какой ей больше нравится (от Егора: лучше конструкторы).

### Модификатор explicit для конструкторов и операторов приведения

По умолчанию, если написан конструктор, или оператор приведения типа, то
могут выполняться неявные преобразования к этому классу, или из него соответственно.
Чтобы избежать нежелательных неявных преобразований, нужно использовать модификатор
explicit, который говорит, что конструктор или оператор приведения явный.

```C++
class A {};
class B {};
class C {
public:
    explicit C(const A &) { ... } // конструктор приведения из A -> C
    explicit operator B() { ... } // оператор приведения C -> B
};

int main() {
    A a;
    // C c = a; неявное преобразование, забанено explicit
    С с(a); // все еще можно, если определено - преобразование явное.
    // B b = c; тут уместно было бы использовать operator B(), если бы он не был explicit
    B b(c); // так можно, если определено - явное преобразование.
}
```

Без explicit неожиданные вещи могут происходить, поэтому 10 раз подумать, прежде чем
писать не explicit оператор.

### Особенности explicit operator bool() по сравнению с остальными explicit-операторами

С лекции:

Например, когда пишем свой unique_ptr, может захотеться проверять его на null, как с обычным указателем:

```C++
unique_ptr<Foo> f = ...;
if (f) { ...  }
// или
if (!f) { ... }
```

Можно сделать operator bool(). Но тогда неявные преобразования врубятся вообще везде, включая: 10 + f (преобразовали в bool, преобразовали в int, сложили). В C++03 для этого использовались костыли под названием "Safe bool idiom" (там возвращали хитрый тип, который нельзя в int, но можно в bool), в C++11 можно написать explicit operator bool(). Тогда можно будет преобразовать в bool, но только явно, как с explicit конструкторами (static_cast). Конкретно для bool захардкожено, что можно использовать в: if/while/for, !, ||, &&, ?: и ещё паре мест.

И добавить нечего, могу только пример привести

```C++
struct my_unique_ptr {
    explicit operator bool() { ... }
};

int main() {
    my_unique_ptr<int> a;
    if (a) {
        assert(false);
    }

    // bool a = p; explicit запрещает
    // int a = 10 + a; тоже неявное преобразование
    a.reset(new int(10));
    std::cout << (a ? *a : 0) << std::endl;
}
```
