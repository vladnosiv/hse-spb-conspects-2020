## Билет 41 "Parameter pack — детали"
Автор: Герман Тарабонда

### Возврат parameter pack невозможен, что с этим делать

Иногда хочется вернуть parameter pack или использовать его с `using`  или `typedef`, но просто так не получится, так как это все требует конкретных типов. То есть в таком коде будет ошибка:

```C++
template<typename .../*Ts*/> struct tail {};
template<typename Head, typename ...Tail>
struct tail<Head, Tail...> {
    using types = Tail...;  // не ОК: внутри структуры могут быть только типы
};
```

Единственное решение: можем создать вспомогательный тип

```C++
template<typename...> struct type_list {}; // тип, который только хранит в себе parameter pack
template<typename .../*Ts*/> struct tail {};
template<typename Head, typename ...Tail>
struct tail<Head, Tail...> {
    using type = type_list<Tail...>;
};
template<typename ...Ts> using tail_t = typename tail<Ts...>::type; // можем создать псевдоним
```

Но появляются новые проблемы: мы не можем взять `tuple` от одного листа. Для этого создаем еще одну структуру (но можно задолбаться все это писать)

```C++
template<typename> struct type_list_to_tuple {};
template<typename ...Ts> struct type_list_to_tuple<type_list<Ts...>> {
    using type = tuple<Ts...>;
};
typename type_list_to_tuple<tail_t<int, string>>::type = tuple<string>
// == type_list_to_tuple<tail<int, string>::type>::type
// == type_list_to_tuple<type_list<string>>::type
// == tuple<string>
```

В общем случае можно передать `tuple` как параметр:

```C++
// Заглушка
template<template<typename...> typename /*F*/, typename /*TypeList*/>
// передаем шаблонную функцию F
struct apply_type_list;
// Реализация
template<template<typename...> typename F, typename ...Ts>
struct apply_type_list<F, type_list<Ts...>> {
    using type = F<Ts...>;
};
typename apply_type_list<tuple, type_list<int, string>>::type == tuple<int, string> 
```

### Pattern matching + специализации для обработки элементов пака у классов

Рассмотрим пример реализации `std::tuple_element`:

```C++
template<std::size_t, typename /*Tuple*/> struct tuple_element;
// Базовый случай.
template<typename Head, typename ...Tail>
struct tuple_element<0, tuple<Head, Tail...>> {
    using type = Head;
};
// Основной случай.
template<std::size_t I, typename Head, typename ...Tail>
struct tuple_element<I, tuple<Head, Tail...>> : tuple_element<I - 1, tuple<Tail...>> {};
// ....
tuple_element<0, tuple<int, string>> = int
tuple_element<1, tuple<int, string>> = tuple_element<0, tuple<string>> = string
```

Здесь pattern matching для поиска нужной перегрузки.

### Pattern matching + автовывод параметров + перегрузки для обработки элементов пака у функций

Иногда чтобы работать с элементами, очень классно бывает перегружать функции:

```C++
// getInt("x", 15.0, 10, "x") == 10
template<typename ...Ts> int getInt(int x, const Ts &...) {
    return x;
}
template<typename T, typename ...Ts> int getInt(const T &, const Ts &...args) {
    return getInt(args...); // вот здесь происходит автовывод параметров
    // так как мы явно не указываем какие типы мы передаем
}
```

Здесь происходит выбор между двумя функциями по принципу pattern matching. Если вдруг мы в качестве аргументов функции не передадим `int`, то мы можем дойти до пустого вызова функции и будет ошибка компиляции (sad).

### Реализация `std::tuple`

`std::tuple` - класс, который хранит фиксированное число различных по типу объектов.

Для написания сначала нужно указать общий тип, затем писать специализацию:

```C++
template<typename ...Args> struct tuple {}; // общий тип
template<typename T, typename ...Args> // специализация
struct tuple<T, Args...> {
    T head;
    tuple<Args...> tail;
};
// ....
tuple<int, std::string> x;
x.head            // int
x.tail            // tuple<std::string>
x.tail.head       // std::string
x.tail.tail       // tuple<>
x.tail.tail.head  // compilation error
```

### Реализация `std::get`

`std::get` помогает получить i-ый элемент `std::tuple`

```C++
template<std::size_t I, typename ...Ts>
auto get(const tuple<Ts...>& tuple) {  // можно не константные auto& / tuple&
    if constexpr (I == 0) return tuple.head; // constexpr нужен,
    // чтобы правильно вывелся тип функции
    else                  return get<I - 1>(tuple.tail);
}
// ....
tuple<int, string> x;
get<0>(x) == x.head  // int
get<1>(x) == get<0>(x.tail) == x.tail.head  // string
```

### Реализация `std::tuple_size`

`std::tuple_size` - размер `std::tuple`

```C++
template<typename> struct tuple_size {}; // для не tuple - хрень
template<typename ...Args>
struct tuple_size<tuple<Args...>> // для tuple - не хрень
    : std::integral_constant<std::size_t, sizeof...(Args)> {};
```

### Реализация `std::tuple_element`

`std::tuple_element` - по индексу и `std::tuple` получаем тип элемента

```C++
template<std::size_t, typename /*Tuple*/> struct tuple_element;
// Базовый случай.
template<typename Head, typename ...Tail>
struct tuple_element<0, tuple<Head, Tail...>> {
    using type = Head;
};
// Основной случай.
template<std::size_t I, typename Head, typename ...Tail>
struct tuple_element<I, tuple<Head, Tail...>> : tuple_element<I - 1, tuple<Tail...>> {};
// ....
tuple_element<0, tuple<int, string>> = int
tuple_element<1, tuple<int, string>> = tuple_element<0, tuple<string>> = string
```

### Реализация `std::make_tuple`

`std::make_tuple` создает `std::tuple` из принятых аргументов

```C++
template <typename... Ts>
auto make_tuple(Ts&&... args) {
    return tuple<Ts...>(std::forward<Ts>(args)...);
}
// ...
auto t = std::make_tuple(239, "help"); // tuple<int, string>
```

### Трюки с pack expansion: инициализацию массива (в том числе фиктивного до fold expression C++17), `operator,`

Хотим мы сделать что-то с аргументами, но могут вылезать всякие "приятные" вещи:

```C++
std::tuple t( readInt<Args>()... );
std::tuple t( readInt<Int>(), readInt<int>() ); // не понятен порядок вывода
//
std::tuple t{ readInt<Args>()... };
std::tuple t{ readInt<Int>(), readInt<int>() ]; // понятен порядок вывода,
// но выводится warning (это просто бага)
```

Или мы хотим записать все в массив или очень круто вывести элементы (зачем?)

```C++
int res[] = { args... }; // инциализируем массив константного размера элементами args
int dummy[] = { (std::cout << args, 0)... }; // выводим элемент, а затем инциализируем массив нулями
```
