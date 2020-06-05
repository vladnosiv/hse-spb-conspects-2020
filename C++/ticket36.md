## Билет 36
Автор: Петя Сурков

### Свои расширяемые type traits
Допустим, написали такую библиотеку
```c++
template<typename T> struct serialization_traits {
    static void serialize(ostream &os, const T &x) {      x.serialize(os); }
    static T deserialize(ostream &is)              { T x; x.deserialize(is); return x; }
};
template<> struct serialization_traits<int> {
    static void serialize(ostream &os, int x) { os.write(...); }
    static int deserialize(istream &is)       { int x; is.read(...); return x; }
};
// А тут идут разные полезные функции, работающие через serialization_traits, к примеру
template<typename T>
void saveToFile(string fileName, T data) {
    ofstream os(fileName, std::ios::binary);
    serialization_traits<T>::serialize(os, std::move(data));
}
```

Тогда в пользовательском коде можем обернуть даже "не свой" тип, про который библиотека ничего не знает:
```c++
template<typename T>
struct serialization_traits<vector<T>> {
    static void serialize(ostream &os, const vector<T> &data) {
        serialization_traits<size_t>::serialize(os, data.size());
        for (const auto &item : data)
            serialization_traits<T>::serialize(os, item);
    }
    
    // тут аналогично deserialize, если захотим использовать "полезные функции" в библиотеке, которые его юзают.
    
};
saveToFile("foo.txt", std::vector<int>{1, 2, 3, 4}); // Работает!
```

Таким образом, позволили пользователям нашей библиотеки расширять её. Зачем нужно? Нам меньше писать, а пользователь библиотеки может работать с любыми типами.

### Оператор `noexcept`
Про типы всё можем узнавать, теперь давайте что-нибудь узнаем про выражения и функции.

Оператор `noexcept` позволяет узнать, может ли из выражения теоретически вылететь исключение (просто смотрит на спецификаторы `noexcept`!). Возвращает `true`/`false` _на этапе компиляции_, **не** вычисляя выражение.


```c++
int foo() noexcept { return 1; }
int bar()          { return 2; }
// ....
int a = 10;
vector<int> b;
static_assert(noexcept(a == 10));
static_assert(!noexcept(new int{}));   // Утечки не будет: не вычисляется.
static_assert(noexcept(a == foo()));
static_assert(!noexcept(a == bar()));  // bar() НЕ noexcept
static_assert(!noexcept(b == b));      // vector::operator== не noexcept по историческим причинам
bool x = noexcept(a = 10);
assert(x);
```

### Спецификатор `noexcept`
У _спецификатора_ (не оператора) `noexcept` есть две формы:

```c++
template<typename T> struct optional {
    optional() noexcept;  // Не кидает никогда. Если же кинула, то terminate
    optional(optional &&other) noexcept(  // noexcept только при условии...
        std::is_nothrow_move_constructible_v<T>
    );
    // В данном случае - если у T move конструктор не бросает исключений.
};
```

Если же мы забыли про `std::is_nothrow_move_constructible_v<T>` (ну или просто хотим что-то более сложное) и хотим сделать такую же логику сами, то можно вызвать оператор внутри спецификатора:

```c++
template<typename T> struct optional {
    optional() noexcept;
    optional(optional &&other) noexcept(noexcept(T(std::move(other))));
}
```
Пояснение:
```c++
template<typename T> struct optional {
    optional() noexcept;
    optional(optional &&other) noexcept(    // noexcept только при условии... (тут noexcept - спецификатор)
       noexcept(  // ...что следующее выражение noexcept...  (тут noexcept - оператор)
           T(std::move(other))  // вызов move-конструктора T
       )
   );
}
```

### Применение `noexcept`
Полезно для `vector` со строгой гарантией исключений.

Если у элементов `is_nothrow_move_constructible`, то можно перевыделять буфер без копирований:

```c++
void increase_buffer() {
    vector_holder new_data = allocate(2 * capacity);  // Может быть исключение.
    for (size_t i = 0; i < len; i++)
        new (new_data + i) T(std::move(data[i]));     // Портим data, боимся исключений.
    data.swap(new_data);                              // Исключений точно нет.
}
```

Иначе просто копируем.

Подобная ситуация встречается часто, поэтому есть уже готовая функция `std::move_if_noexcept`, тогда специализировать не надо:

```c++
void increase_buffer() {
    vector_holder new_data = allocate(2 * capacity);
    for (size_t i = 0; i < len; i++)
        new (new_data + i) T(std::move_if_noexcept(data[i]));
    data.swap(new_data);
}
```

`std::move_if_noexcept` проверяет, кидает ли исключения `T` при move (просто смотрит на noexcept, не более!), если не кидает, то возвращает rvalue (как обычный `move`), иначе lvalue.

### Функция `declval<>`
Внутри `noexcept` иногда не хватает переменных:
```c++
template<typename T>
constexpr bool is_nothrow_move_assignable_v = noexcept(
    /* переменная типа T */ = std::move(/*что-нибудь типа T???*/)
);
```
При этом конструкторов у `T` может вообще не быть:
```c++
template<typename T>
constexpr bool is_nothrow_move_assignable_v = noexcept(
    /* переменная типа T */ = T{} /* rvalue, но не всегда скомпилируется */
);
```

Для решения этой проблемы есть `std::declval<T>()`. Она создаёт значение любого типа:
```c++
template<typename T>
static constexpr is_nothrow_move_assignable_v = noexcept(
    std::declval<T&>() = std::declval<T/*&&*/>()
);
```

Это легально только внутри _невычислимых контекстов_.

`int x = std::declval<int>();  // Ошибка компиляции`

### Оператор `sizeof`
Ещё невычислимый контекст: оператор `sizeof`

* Можно вызвать от типа:
  ```c++
  static_assert(sizeof(char) == 1);
  ```
* Можно вызывать от выражения, оно не будет вычислено:
  ```c++
  char& foo();
  static_assert(sizeof(foo()) == 1);
  static_assert(sizeof(&foo()) == sizeof(char*));
  ```

Самый первый оператор такого вида, единственный в C++03,
из-за этого часто применяется в костылях.

### Оператор `decltype`
Есть с C++11: `decltype(expr)` — это некоторый тип.

Если в скобках имя сущности, то её тип из объявления:
```c++
int a = 10; int &b = a;
struct { int& field; } c{a};
decltype(a) x = a;              // int x = a;
decltype(b) y = b;              // int& y = b;
decltype(s.field) z = c.field;  // int& z = c.field;
```

Если в скобках стоит выражение, то 
1) Берём чистый тип выражения: с константностью, но без ссылок.
2) Смотрим на категорию и добавляем соответствующую ссылку.

Поясняющий пример:
```c++
Foo func1();
Foo& func2();
Foo&& func3();
Foo a;
// ....
decltype(func1()) x = func1();  // prvalue, Foo x = func1();
decltype(func2()) y = func2();  // lvalue,  Foo &y = func2();
decltype(func3()) z = func3();  // xvalue,  Foo &&z = func3();
decltype(std::move(a))  v2 = std::move(a);  // xvalue,  Foo &&v2 = std::move(a);
decltype(Foo{})         v3 = Foo{};         // prvalue, Foo v3 = Foo{};
```
В билете просят пример, когда `declval`, может использоваться в `decltype`. Например, если нужно узнать тип не `static` функции класса: `decltype(std::declval<T>().foo)`
### Тонкости оператора `decltype`
Дополнительные скобки делают из имени выражение, а по правилам `decltype` для выражений:

```c++
Foo a;
const Foo &b = a;
decltype( a ) v1 = a; //  a  — имя.               Foo v1 = a;
decltype((a)) v2 = a; // (a) — выражение. lvalue, Foo &v2 = a;
decltype( b ) v3 = b; //  b  — имя.               const Foo &v3 = b;
decltype((b)) v4 = b; // (b) — выражение. lvalue, const Foo &v4 = b;
```

Сильно отличается от `auto`: второй снимает ссылки и константность.
```c++
auto x = a;  // Foo v1 = a;
auto y = b;  // Foo v3 = b;
```

 `decltype` используется:

* Когда нужно отличать типы друг от друга (до С++11 использовались костыли с `sizeof`)

* Для _очень_ правильного вывода типов параметров и
  возвращаемого значения функций:
    ```c++
    decltype(auto) v1 = a;    // Foo v1 = a;
    decltype(auto) v2 = (a);  // Foo &v2 = a;
    decltype(auto) v3 = b;    // const Foo &v3 = b;
    decltype(auto) v4 = (b);  // const Foo &v4 = b;
    ```

### `if constexpr`
* Хотим сделать метод `void printAll(const T &value)` для `int` и `vector<T>`, но без перегрузок.
* Разберём случай `int`: сделаем `if(is_same_v<T, int>)`. Не сработает, потому что компилируется то обе ветки. Поэтому для `int` мы всё равно будем пытаться скомпилить функции в `else`, т.е. пробовать вызвать `operator[]` и тд. 
* До C++17 приходилось писать шаблонный класс, специализировать, вызывать.
* В C++17 можно писать `if constexpr`. И даже работает автовывод возвращаемого `auto` типа функции.
* Даже внутри `if constexpr (false)` всё равно должен быть корректный синтаксис.

### `static_assert`
* Проверяет условие на этапе компиляции. Если не удалось — ошибка компиляции.
* Если мы хотим сделать `static_assert(false)` внутри `if constexpr`, то будет ошибка
  компиляции всегда.
  * Причины технические. Надо обмануть компилятор.
  * Надо дать ему выражение, которое как-то теоретически может зависеть от `T`, делаем 
  ```cpp
  template<typename> struct AlwaysFalse : std::false_type{};
  ```
  и уже `static_assert(AlwaysFalse<T>::value)`

### `std::conditional`
* Для выражений у нас есть тернарный оператор `cond ? t : f`, для `constexpr` тоже работает.
* Для типов такого нет. Надо писать свой if: `std::conditional<cond, T, F>`.
  * `typename std::conditional<true, T, F>::type` — это `T`.
  * `typename std::conditional<false, T, F>::type` — это `F`.
* Помогает объявлять поля.
 
Реализация (спрашивают в билете)
```c++
template<bool B, typename T, typename F>
struct conditional { typedef T type; };
 
template<typename T, typename F>
struct conditional<false, T, F> { typedef F type; };
```
