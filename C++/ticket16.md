## Билет 16
Автор: Илья Онофрийчук

## Псевдонимы шаблонов
Инстанцированный шаблон -- это обычный тип, поэтому для него можно написать `typedef`
```cpp
typedef vector<int> vi;
``` 
Для неинстанцированных шаблонов так писать уже нельзя, будет ошибка компиляции
```cpp
typedef vector v; //compilation error
``` 
В C++11 появился Alias template(псевдоним шаблонов). Конкретные примеры:
```cpp
template<typename T>
using v = vector<T>;

template<typename T>
using enable_if_t = typename enable_if<T>::type;
```
## Шаблонные методы в шаблонных классах
Рассмотрим пример:
```cpp
template<typename T>
struct shared_ptr {
    Storage *s;
    shared_ptr(T *data);
};

struct Base {};
struct Derived : Base {};

int main() {
    shared_ptr<Derived> d = ...;
    shared_ptr<Base> b = d;  // Упс, несовместимые типы.
}
```
Возникает проблема, что мы не можем записать указатель на наследника в указатель на базовый класс. 

Причина: `shared_ptr<Derived>` и `shared_ptr<Base>` -- разные типы, и они друг с другом не связаны. 

Решение: добавить шаблонный конструктор.
```cpp
template<typename T>
struct shared_ptr {
    ... 
    template<typename U>
    shared_ptr(const shared_ptr<U> &p);
};
```
Несмотря на то, что данный конструктор сработает даже если `U*` нельзя будет привести к `T*`, например `U = int` и `T = Derived`, произойдёт ошибка компиляции, то есть всё безопасно.

Аналогично любой метод у класса может быть шаблонным, при этом автовывод шаблонных параметров будет работать также как у шаблонных функций. Также, ***не конструктор*** можно вызвать, явно указав параметры шаблонного метода. 

## Необходимость слов `typename` и `template` в некоторых контекстах

```cpp
template<typename T> struct A { using x = int; };
template<typename T> struct B {
    void foo() {
        A<T>::x * y; // (1)
    }
};
```
Неочевидно, что делает строчка `(1)`, это либо умножение, либо объявление переменной типа `A<T>::x`. Поэтому обычно компиляторы требуют явно писать слово `typename` перед зависимым типом, иначе они будут трактовать `x` как переменную. Строку `(1)` следует исправить следующим образом:
```cpp
typename A<T>::x * y;
```
Пример про `template`:
```cpp
template<typename T> struct A { ... };
template<typename T> struct B {
    template<typename U> using C = A<U>;
};
template<typename T> struct D {
    B<T>::C<T>::x ... // (1)
};
```
В строке `(1)` компилятору неясно `C` -- это поле или шаблон. Поэтому нужно добавить перед `C` слово `template`. То есть строка `(1)` должна иметь следующий вид:
* Если `x` -- это переменная: 
```cpp
B<T>::template C<T>::x ...
```
* Если `x` -- это тип: 
```cpp
typename B<T>::template C<T>::x ...
```
Аналогичная проблема может возникнуть при вызове шаблонных функций, которым хотим явно задать параметры:
```cpp
template<typename T> struct Foo {
    template<typename U> 
    void foo();
};

template<typename T>
void bar() {
    Foo<T> x;
    x.template foo<char>();
}
```
Компилятору может потребоваться явное указание того, что `foo` -- это метод, так как он не знает метод ли это, поле или что-нибудь ещё.
## Специализации шаблонов функций
Частичной специализации функций не существует, есть только полная.

Пример специализации функции:
```cpp
template<typename T> 
void my_swap(T &a, T &b) { ... }

template<> 
void my_swap<std::string>(std::string &a, std::string &b) { ... }

template<typename T> 
void my_swap<std::vector<T>>(std::vector<T> &a, std::vector<T> &b) { ... } // compilation error, так как частичной специализации для функций не существует
```
Вместо частичной специализации функций предполагается использовать перегрузки.
```cpp
template<typename T> 
void my_swap(std::vector<T> &a, std::vector<T> &b) { ... } // Ok. Это перегрузка, а не специализация
```
Однако функцию `std::swap` можно по стандарту специализировать, но не перегружать. Стандарт запрещает добавлять свои перегрузки в пространство имён `std` .

## Отличия специализации шаблонов функций и перегрузок
```cpp
template<typename T>
void print(const T &) { } //(1)

template<>
void print<std::vector<int>>(const std::vector<int> &) { } //(2)

template<typename T>
void print(const std::vector<T> &) { } //(3)

int main() {
    std::vector<int> vi;
    std::vector<float> vf;

    print<std::vector<int>>(vi); //вызовется (2), T = std::vector<int>
    print<int>(vi); //вызовется (3), T = int

    print<std::vector<float>>(vf); //вызовется (1), T = std::vector<float>
    print<float>(vf); //вызовется (3), T = float
    return 0;
}
```

