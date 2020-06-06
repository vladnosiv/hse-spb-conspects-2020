## Билет 40
Автор: Кирилл Бриллиантов

### Variadic template и template parameter pack (c C++11)
#### Основы
**Дисклеймер**: ```...``` везде по делу - то есть если написан такой символ он так и используется в коде. 

**Variadic template** - объявление шаблона класса или функции, в которое входит так называемый *paramter pack*.

Выглядит это следующим образом
```cpp
template <typename U, typename... Ts>
struct VariadicHolder {
  static inline constexpr std::size_t M = sizeof...(Ts);
};
VariadicHolder<int> vHolder_1; // M = 0
VariadicHolder<int, std::vector<int>, double> vHolder_2; // M = 2
```
Чтобы объявить *parameter pack* надо написать ```typename/class ...Name```, где Name - имя параметр пака.
Также в коде выше применяется специальный оператор ```sizeof...```, с его помощью можно на этапе компиляции узнать размер пака


**Важно** - parameter pack может идти только последним в объявлении шаблона, так как компилятор для его вывода использует жадный алгоритм

#### Специализация структур и классов с помощью parameter pack
Для того чтобы специализировать структуру можно использовать несколько параметр паков
```cpp
template <tyename U, typename... Ts1, typename... Ts2>
struct VariadicHolder<U, std::tuple<Ts1>, std::tuple<Ts2>> {};
```
В данном случае мы специализировали стуркурку из предыдущего пункта с помощью какого-то типа U и двух туплов.
#### Pack expansion
Сейчас покажем несколько примеров как можно разворачивать parameter pack. Чтобы его развернуть нужно написать следующую конструкцию:

```pattern...```, где **pattern** содержит один или несколько parametr pack. Данный pattern для каждого элемента из пака повторится через запятую.

Использование вместе с наследованием (c C++17)
```cpp
template <typename T>
struct DataSerializer {
  void serialize(const T& data) {/*какая-то реализация*/}
};

template <typename... Ts>
struct GenericSerializer : DataSerializer<Ts>... {
  GenericSerializer() : DataSerializer<Ts>()... {}
  using DataSerializer<Ts>::serialize...;
};

```
При такой реализации можно специализировать DataSerializer-ы для каких-нибудь T, потом написать 
```cpp 
GenericSerializer<int, float /* и что-то еще */> BigSerializer;
BigSerializer.serialize(10), BigSerializer.serialize(2.5f) /* и еще что-нибудь */; 
```
Выглядит достаточно удобно))

##### Одновременное разворачивание 

Несколько паков можно разворачивать одновременно

```c++
template<typename, typename> struct ZipTuple;
template<typename ...As, typename ...Bs>
struct ZipTuple<tuple<As...>, tuple<Bs...>> {
    using type = tuple<pair<As, Bs>...>;
}
// ....
ZipTuple<tuple<int, char>, tuple<char, string>>::type =
    tuple<
        pair<int, char>,
        pair<char, string>
    >
```
Для такого разворачивания необходимо, чтобы размеры паков совпадали.
##### Возврат пака
Напрямую вернуть параметр пак нельзя, но если очень хочется его получить, то вот общее решение:
```cpp
// Заглушка
template<template<typename...> typename /*F*/, typename /*TypeList*/>
struct apply_type_list;
// Реализация
template<template<typename...> typename F, typename ...Ts>
struct apply_type_list<F, type_list<Ts...>> {
    using type = F<Ts...>;
};
typename apply_type_list<tuple, type_list<int, string>>::type == tuple<int, string>
```
Можно вместо ```F``` какую-нибудь другую структуру и как-то это все использовать в своих целях)
### Function parameter pack
Синтаксис
```cpp
template <typename... Ts>
void f(const Ts& ...args) {
  size_t M = sizeof...(Ts);
}
f(10, 20); // M = 2
f("abc", std::vector<int>({1, 2, 3}), 2.4); // M = 3
```
В фукнциях ситуация немного отличается, так как на каждый пак с типами приходится еще пак со значениями.
Также можно укзаывать сколько угодно параметр паков, главное, чтобы компилятор смог жадно вывести все типы.
```cpp
template <typename... Ts, typename... Us, template Y>
void f(Y y, std::tuple<Ts...> tup, Us& ... u) {
  size_t M = sizeof...(Ts);
}
f("a", std::tuple<int, int>(10, 20), std::tuple<int, int>(10, 20)); // Ts = int, int; Us = std::tuple<int, int>
```
### Perfect forwarding
Очень часто используется фишка с function parameter pack и std::forward для оборачивания других функций.
```cpp
template <typename T, typename... Args>
T emplace(Args&&... args) {
  return T(std::forward<Args>(args)...);  // одновременное разворачивание pack-ов 
}
```
Идея с таким оборачиванием конструктора активно используется в STL контейнерах.
### fold expression и лямбды

С C++17 добавили так называемые fold expressions - они позволяют записывать какие-то операции над элементами pack-а короче и может быть понятнее)
Fold expression бывают binary и unary. 
Вот общий вид всех fold expressions (скобки везде обязательны)

Unary:
- ```(pack op ...) == ((arg1 op arg2) op arg3 /* и так далее */)```
  + пример: ```((std::cout << args), ...) == (std::cout << args1), (std::cout << args2)```
    
    здесь ```pack == args && op == ","```
- ```(... op pack) == (arg1 op (arg2 op arg3) /* и так далее */)```
  + то же самое, что и в предыдущем пункте, просто будут выполнятся в другом порядке.
Unary fold expression требуют, чтобы pack был непустым, кроме случая когда 
```op == "&&" V op == "||" V op == ","```. 
- При ```op == "&&"``` и пустом паке результат выражения ```true```
- При ```op == "||"``` и пустом паке результат выражения ```false```
- При ```op == ","``` и пустом паке результат выражения ```void```(ничего не делает в общем)

Binary:
- ```(init_val op ... op pack) == ((init_val op args1) op args2)```
  + пример: ``` (std::cout << ... << args) == std::cout << args1 << args2```
  
    здесь ```pack == args && op == "<<"```
- ```(pack op ... op init_val) == (args1 op (args2 op init_val))```
  + то же самое, только другая ассоциативность

Условия на fold exprs:
- op это любой из 32 (кхм-кхм) бинарных операторов языка C++. В бинарных fold op должны совпадать.
- pack это выражение, содержащее нераскрытый пак, и, не содержащее операторов с меньшим приоритетом, чем у op.
- init это выражение, не содержащее нераскрытых паков и операторов с меньшим приоритетом, чем у op.

Вместе с fold expression удобно использовать лямбды или функторы, чтобы проэмулировать цикл по всем элементам.
```cpp
template <typename Fn, typename... Ts>
void for_each(Fn fn, Ts ...args) {
    (fn(args), ...);
}
int main() { 
    for_each([](auto a) {std::cout << a << " ";}, 10, "asd", 2.4);
    std::cout << '\n';
    for_each([cur = 0](auto a) mutable {std::cout << ++cur << ". " << a << "\n";}, 10, "asd", 2.4);  
}
// Вывод: 
// 10 asd 2.4
// 1. 10
// 2. asd
// 3. 2.4
```
Если попросят добавить всякие forwarding ссылки, то везде, где надо - дописать ```&&``` (в том числе ```auto&&```) и ```std::forward```, а чтобы внутри лямбды написать ```forward```, видимо надо напсиать 
```cpp
std::forward<decltype(a)>(a);
```
