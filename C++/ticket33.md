## Билет 33
Автор: Кирилл Карнаухов

### `std::tuple` и его друзья
У нас есть `std::pair<T, U>`, но работает только для двух параметров. Хотим больше. На помощь приходит `std::tuple<Args...>`, так называемый гетерогенный список (кортеж). Пример использования:
```cpp
std::tuple<int, vector<int>, string> t1(10, vector<int>(2), "foo");
auto t2 = std::make_tuple(10, vector<int>(2), "foo");  // t1 и t2 совпадают
int a = std::get<0>(t1);     // получить нулевой элемент
string c = std::get<2>(t1);  // Цикла по tuple<> нет
```

Теперь подробнее про функции, связанные с `std::tuple`:

* `std::get<index>(tuple)`. Получает элемент с номером `index` из `tuple`. Важно: `index` должен быть `constexpr` и находиться в полуинтервале `[0, std::tuple_size(tuple))`.
* `std::tuple_size(tuple)` возращает размер кортежа.
* `std::tuple_element<size_t I, template Tuple>::type` возвращает тип `Tuple` на позиции `I`. Возможное применение:
```cpp
auto mytuple = std::make_tuple (10, 'a');
std::tuple_element<0, decltype(mytuple)>::type first = std::get<0>(mytuple);
std::tuple_element<1, decltype(mytuple)>::type second = std::get<1>(mytuple);
```
* `std::make_tuple(Args...)` создает `std::tuple` из переданных аргументов. Пример выше.
* `std::tuple_cat` объединяет два кортежа:
```cpp
auto tuple1 = std::make_tuple(10, 10.0, 'c');
auto tuple2 = std::tuple_cat(t, t); //получится (10, 10.0, 'c', 10, 10.0, 'c')
```
* `std::apply(Func, Tuple)` вызывает `Func` с аргументами, взятыми из `Tuple`.

Полезный факт: есть каст из `std::pair<T, U>` в `std::tuple<T, U>`.
    
### Ссылки внутри `std::tuple`
Внутри можно хранить ссылки, пример:
```cpp
int a = 10; string b = "foo";
std::tuple<int&, string&> t(a, b);
t = std::make_tuple(20, "bar"); 
assert(a == 20);
assert(b == "bar");
```

### `std::tie` 
Пусть есть функция, которая возвращает `std::tuple<>` (или `std::pair<>`, так как есть каст), и мы хотим сопоставить каждому элементу из кортежа свою переменную. С C++11 можно делать так:
```cpp
std::tuple<int, int, char> func() { return { 1, 2, 'a' }; }
//здесь какой-то код

int first;
int second;
char third;

std::tie(first, second, third) = func();
assert(first == 1);
assert(second == 2);
assert(third == 'a');
```

### Structured binding (базовое)
Это более современная замена `std::tie`. Синтаксис:
```cpp
std::tuple<int, int, std::string> tuple(10, 15, "hello");
auto [first, second, third] = tuple; //происходит копирование
assert(first == 10);
assert(second == 15);
assert(third == "hello");
```
Можно навешивать `const/&/static`.

Полезные факты:

* Есть direct initialization (`auto [a, b, c](tuple)`) и list initialization(`auto [a, b, c]{tuple}`).
* Нельзя указать типы отдельных элементов.
* Нельзя вкладывать друг в друга (т.е. нельзя `[[a, b], c]`).
* Происходит на этапе компиляции, поэтому массивы можно (дальше будет подробнее), а вектора — нет.
* Может разворачивать простые структуры.

### Structured binding (применение)
* Получить результат `insert`:
```cpp
map<int, string> m = ...;
if (auto [it, inserted] = m.emplace(10, "foo"); inserted) {
	cout << "Inserted, value is " << it->second << '\n';
} else {
    cout << "Already exists, value is " << it->second << '\n';
}
```
* Итерирование по `map`:
```cpp
for (const auto &[key, value] : m) {
	cout << key << ": " << value << '\n';
}
```

### Structured binding (как работает и раскрывается)
Пусть есть выражение:
```cpp
map<int, string> m = { ... };
//здесь какой-то код
auto [key, value] = *m.begin();
```
Тогда произойдет следующее:

* Создается невидимая переменная:
```cpp
auto e = *m.begin();
using E = pair<const int, string>;
``` 
* Проверяется, что количество аргументов совпадает:
```cpp
static_assert(std::tuple_size_v<E> == 2);
```
* Объявляются новые переменные:
```cpp
std::tuple_element_t<0, E> &key   = get<0>(e);  // Или e.get<0>()
std::tuple_element_t<1, E> &value = get<1>(e);  // Или e.get<1>()
```

Важная особенность состоит в том, что возвращаемое значение не должно быть обязательно `std::tuple`. Можно туда сунуть все, что умеет делать `std::get<>` и `std::tuple_size<>` (метод или функция), которое можно найти с помощью `ADL`. 

### Structured binding (массивы, кортежи и структуры)
Есть три формы привязки:

* Массив фиксированного размера:
```cpp
Foo arr[3];
auto [a, b, c] = arr;
// превращается в
auto e[3] = { arr[0], arr[1], arr[2] };
Foo &a = e[0], &b = e[1], &c = e[2];
```

* Кортежоподобная структура (умеет `std::get<>` и `std::tuple_size<>`). Тогда будет включен `ADL` (упоминалось выше).
* Простые структуры данных:
```cpp
struct Point {
	int x, y;
}; // Привяжется.

struct Good { int a, b; }
struct GoodDerived : Good {};

struct BadPrivate { int a; private: int b; }  // Не привяжется: приватные запрещены.
struct BadDerived : Good { int c; }  // Не привяжется: все поля должны быть в одном классе.
```

### Structured binding (дополнительно)
В зависимости от `auto`/`auto&`/`const auto&` и инициализатора у нас получаются немного разные типы:

* `auto&` попробует привязать ссылку.
* `const auto&` продлит жизнь временному объекту.
* `auto` всегда скопирует объект целиком, а не просто его кусочки.

Если внутри объекта лежали ссылки, то может сломаться время жизни:
```cpp
namespace std {
    std::pair<const T&, const T&> minmax(const T&, const T&);
}
auto [min, max] = minmax(10, 20);  // Только копирование значений?
// перешло в
const pair<const int&, const int&> e = {10, 20};
// Сам `e` — не временный, поэтому продления жизни нет.
// e.first и e.second ссылаются на уже умершие 10 и 20.
const int &min = e.first;   // Oops.
const int &max = e.second;  // Oops.
```

Рекомендация: осторожно с функциями, которые возвращают ссылки.
С ними лучше `std::tie`.