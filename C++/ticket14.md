## Билет 14
Автор: Валера Головин
### Шаблонные классы
```C
// my_array.h
class IntArray {
private: 
    int *array;
    size_t size;
public:
    ...
}
```
Хотим сделать IntArray не только для int
#### Решение в Си
```C
// my_array.h
#define MyArray(TYPE) class MyArray_##Type { \
private: \
    TYPE *array; \
    size_t size; \
public: \
    ... \
}
// \ - перевод строки, а ## - конкатенация токенов.

// Применение
#include "my_array.h"
MyArray(int);
MyArray(double);

MyArray_int a;
```
##### Проблемы
* среда разработки никак не поможет
* очень странные ошибки могут быть, при опечатке в названии
* не работает с namespace: `MyArray(details::Foo)` не скомпилируется.

#### Решение в C++
```cpp
template<typename T>
class MyArray {
private:
    T *data;
    size_t size;
public:
    Array();
    Array(const Array& other);
    T& get(size_t i) { return data[i]; }
    T& operator[](size_t i);
};

<T>
T& MyArray<T>::operator[](size_t i) { return data[i]; }
<T>
T& MyArray<T>::Array() { ... }
```
* Здесь `MyArray` — шаблон, а `MyArray<int>` — класс.
* Все `MyArray<???>` абсолютно независимые.
* Компилируется методом "скопировали токены, заменили `T` на указанный тип".
  * Нет проблем с именами.
* Шаблон проверяется только на базовую синтаксическую корректность
  (скобочки сошлись), но не компилируется, пока не потребуется.
* В момент первого использования шаблон **инстанцируется** в конкретный класс и происходит компиляция. (_Инстанцирование (англ. instantiation) — создание экземпляра класса_)
  Тут уже можно получить несоответствия типов, не найденные перегрузки функций...
  * Инстанцируется отдельно: все поля класса (когда пытаемся создать объект), методы (когда вызываем конкретный метод; то есть какие-то методы могут для некоторых типов не работать). Пока методы в классе не вызываются они могут вообще не иметь смысла ("утиная типизация").
* Так как работает потенциально с любым типом,
  все реализации должны быть в `.h`.
 Компилятор должен понимать, какой для какого типа он должен компилить.
* До C++11 нельзя было писать `vector<vector<int>>`, потому что `>>` похоже на битовый сдвиг.

### Шаблонные функции
```cpp
template<typename T>
void print(const vector<T>& a) {
    for (const auto& i : a) {
        std::cout << i << '\n';
    }
}

// использование
vector<int> a;
print<int>(a);
print(a); // он решит уравнение и поймет, что T = int
```
Проверка наличия << у типа `T` происходит только в момент инстанцирования.
Если есть несколько перегруженных функций, то компилятор в каждой перегрузке решит
уравнение, если не получилось - выкинет из кандидатов, а среди выживших
проведёт конкурс "кто лучше подоходит под параметры".
```cpp
// объявление функций
template<typename T, typename V>
void foo(V x);

template<typename T, typename V>
void goo(T x);

template<typename T = void, typename V>
void hoo(V x);


// вызов этих функций
foo<int>(10.0); // T = int, V = double
foo(10.0); // не хватает информации для T

goo<int>(10.0); // не хватает информации для V
goo(10.0); // не хватает информации для V

hoo<int>(10.0); // T = int, V = double
hoo(10.0); // T = void, V = double
```
Еще один пример с сортировкой
```cpp
template<typename T, typename Cmp>
void sort(Array<T> &arr, Cmp fn);

// вызов
struct Comparator {
    bool operator()(int a, int b) {
        return a < b;
    }
};
Comporator c;
Array<int> arr;
sort<int, Comparator>(arr, c); // явное указание типов
sort(arr, c);
```
Работает быстрее, т.к. нет виртуальных вызывов.
Код можно сократить, используя лямбды.
```cpp
sort(arr, [](int a, int b) { return a < b; });
// Тип лямбды неизвестен, его явно не указать, только автовыводом.
```
### Строгая гарантия исключений
### Неявный конструктор присваивания
В стандарте есть:
```cpp
class Foo {
    Foo(const Foo&)  // Copy constructor
    Foo(Foo&&)       // Move constructor
    Foo& operator=(Foo&&)       // Move assignment
    Foo& operator=(const Foo&)  // Copy assignment (1)
    Foo& operator=(Foo)         // Copy assignment (2) для copy-and-swap.
};
```
* Если не объявлен копирующий конструктор, то его сгенерирует компилятор.
* Даже если есть другой подходящий конструктор

```cpp
class Foo {
    template<typename T>
    Foo(const T&) {}    // (1) не copy constructor.
    // Foo(const Foo&)  // (2) copy constructor.
    // (2) неявно объявлен и определён, приоритет выше (1).
};
```
При вызове `Foo new_foo( foo )` вызовится `Foo(const Foo&)`

