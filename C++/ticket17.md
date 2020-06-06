## Билет 17
Автор: Юра Худяков

Руководствуясь лекциями Скотта Майерса и Егора.

Есть разные виды вывода типа. 
1. Шаблонный
2. auto
3. Возвращаемое значение функций и лямбд
4. Захват переменных в лямбду
5. decltype

При этом они пересекаются и где-то похожи, где-то отличаются.

Начнём по порядку.

### Шаблоны И auto


```c++
template<typename T>
void f(PARAM param);

f(EXPR);
```

Авотматически выводится, что такое `T` и что такое `PARAM`.

Пример: `T` не всегда такой же, как `PARAM`

```c++
template<typename T> 
void printAll(const vector<T>&);

printAll(vector<int>{10, 20});
```

Здесь `PARAM=const vector<int>&`, `T=int`.

#### PARAM&, PARAM\*    -- ссылки и указатели, ссылки НЕ RVALUE

То есть функция принимает по ссылке или по указателю, притом не забываем, что они могут быть константными (или `volatile`)

Правила:
1. Отбрасываем у `EXPR` ссылку (если есть)
2. Делаем pattern matching, на основе которого выводим тип. 

Важно: здесь `T` никогда не выведется в ссылку или указатель.

Пример:

```c++
template<typename T> void f(T&);

      int    x = 10;   f(x);           // T =       int, PARAM =       int&
const int   cx = 20;  f(cx);           // T = const int, PARAM = const int&
      int  &rx =  x;  f(rx);           // T =       int, PARAM =       int&
const int &crx =  x; f(crx);           // T =       int, PARAM = const int&

template<typename T> void g(const T&);
g(x);                                  // T =       int, PARAM = const int&
```

Разберём этот пример. (очень подробно, можно пропускать, если выше понятно)

Первый вызов: `EXPR` - это `x`. Ссылки у `x` нет, отбрасывать нечего, тогда `T = int`. Получаем, `T& => int&`

Второй вызов: `EXPR` - это `cx`. Ссылки у `cx` нет, отбрасывать нечего, тогда `T = int`. Получаем, `T& => const int&`

Третий вызов: `EXPR` - это `rx`. Отбросили ссылку, получили `int& => int`, тогда `T = int`. Получаем, `T& => int&`

Четвёртый вызов: `EXPR` - это `rx`. Отбросили ссылку, получили `const int& => const int`,тогда `T = const int`. Получаем, `T& => const int&`

Аналогично с вызомом `g`: только теперь `PARAM` имеет ещё и `const`, и то, во что выводится `T`, не поменялось, зато поменяется тип аргумента (что логично, мы его поменяли)



С `auto` удобно: выводится так же, как и для `T` в шаблонах.

```c++
int x = 10;
const int cx = x;
int& rx = x;
const int& crx = x;

      auto&  rx_a1 =   x;  // auto =       int =>       auto& =       int&
      auto&  rx_a2 =  cx;  // auto = const int =>       auto& = const int&
const auto& crx_a3 =  rx;  // auto =       int => const auto& = const int&
const auto& crx_a4 = crx;  // auto =       int => const auto& = const int&

```

#### PARAM&& -- ссылка RVALUE

Здесь также про __Forwarding Reference__

Ведёт себя почти также, как если бы была PARAM&, но в некоторых местах отличается

Если у нас написано некоторое выражение от шаблонного параметра, например, `Foo<T>&&`, то оно себя вести будет точно также, как и в пункте выше: отбрасывать у `EXPR` ссылку, паттерн-матчить. Сюда же относится и `const T&&` !

Если же у нас написано просто `T&&`, происходят <s>костыли</s> другие вещи.

Если `EXPR` - это lvalue (просто переменная, например), то если в предыдущем пункте она вывелась в `E`, то здесь `T` выведется в `E&`. И `PARAM` выведется в `E&` тоже.

Притом если `EXPR` - это rvalue, то она выведется "по-нормальному": `T = E`, `PARAM = E&&`

```c++
template<typename T> void f(T&&);

      int    x = 10;   f(x);           // T =       int&, PARAM = int&
      int  &rx =  x;  f(rx);           // T =       int&, PARAM = int&
                      f(std::move(x)); // T =       int,  PARAM = int&&

const int   cx = 20;  f(cx);           // T =  const int&, PARAM = const int&
const int &crx = cx; f(crx);           // T =  const int,  PARAM = const int&&
// последняя строчка: const int& - это rvalue, так что вывелось "по-нормальному"
```

Возможно, поможет следующее:

```c++

f(T&), зовём f(EXPR&)   => T&
f(T&), зовём f(EXPR&&)  => T&
f(T&&), зовём f(EXPR&)  => T&
f(T&&), зовём f(EXPR&&) => T&&

// "Минимум" по количеству амперсандов :)

```

Но по факту, первые 2 работают вполне стандартно - убирают ссылку, припиливают свою.
Последний работает тоже стандартно - убрал ссылку, припилил свою.
А вот третий работает необычно.

На лекции Скотта Мейерса говорилось примерно следующее: 
Мы пишем в объявлении параметра функции T&&, но при этом при инстантации шаблона он может стать как T&, так и T&&, в зависимости от принимаемого аргумента. 

Поэтому он называет их Universal reference. В стандарте их зовут Forwarding reference.

Итого, суть: `f(T&& t)` сохранит категорию значения `t`: была lvalue, останется lvalue, была rvalue, останется rvalue. При этом она сохраняет и все CV-qualifiers, то есть const и volatile, они не отбрасываются при выводе для __ссылок__ (или указателей, но здесь они нафиг не нужны)

Отсюда вырастает прекрасная возможность делать Perfect Forwarding: мы можем сделать функцию, которая полностью сохраняет категорию значения и все cv-qualifier-ы. Это уже залезает в Perfect Forwarding и здесь вроде не нужно.

`auto&&` тоже можно, например, range-based for
```c++
    for (auto&& x: ar) {
        ...
    }
```

#### PARAM - не ссылка или указатель

То есть функция принимает по значению.

Правила: (очень похожи)
1. Отбрасываем у `EXPR` ссылку (если есть)
2. __NEW!__ Отбрасываем `const` и `volatile` у `EXPR` (если есть)
3. Делаем pattern matching, на основе которого выводим тип. 

Пример:

```c++
template<typename T> void f(T); // ЗДЕСЬ ПОМЕНЯЛОСЬ T& -> T

      int    x = 10;   f(x);           // T = int, PARAM = int
const int   cx = 20;  f(cx);           // T = int, PARAM = int
      int  &rx =  x;  f(rx);           // T = int, PARAM = int
const int &crx =  x; f(crx);           // T = int, PARAM = int



template <typename T>
struct Foo {}

template<typename T> void g(Foo<T>);

const Foo<const int> foo;
g(foo);                                // T = const int, PARAM = Foo<const int>
```

В `auto` поэтому всегда копируем: на это можно напороться (лично я несколько раз точно)

```c++
vector<int> vec = {1, 2, 3, 4};
const vector<int> cvec = vec;
const vector<int> &rvec = vec;
auto avec = vec;    // auto = vector<int>
auto acvec = cvec;  // auto = vector<int>
auto arvec = rvec;  // auto = vector<int>
```

#### braced-initializer list

__Нет типа__

А выводится в `std::initializer_list`

```c++
auto x2 {1,2,3};     // не компилируется
auto x1 = {1, 2, 3}; // auto = std::initializer_list<int>
auto x3 {1};         // auto = int
auto x4 = {1};       // auto = std::initializer_list<int>
```


### Что там в лямбдах и auto func()?

#### Тип возвращаемого значения

Возвращаемый тип лямбды - тот же, как если бы мы сделали `auto func()`
Но он выводится не как обычная auto, он выводится как шаблоны выше. 

Потому мы не можем вернуть braced-initializer list - у него нет типа, хотя он и выводится auto

```c++
auto  lambda = [](int x) { return x; }   // T=int, retval=int
auto  foo(int x) { return x; }           // auto=int
auto  foo(int &x) { return x; }          // auto=int
auto& foo(int &x) { return x; }          // auto=int, auto&=int&
auto  foo(const int &x) { return x; }    // auto=int, retval=int
auto& foo(const int &x) { return x; }    // auto=const int, auto&=const int&
```

Притом оно должно полностью сойтись с первым встреченным `return`-ом:

```c++
auto fac1(int n) { // Окей
    if (n == 1) return 1;
    else        return n * fac1(n - 1);
}
auto fac2(int n) { // Ошибка компиляции: пытаемся вывести тип из выражения `n * fac2(n-1)`, которое зависит от `T`, не выходит, компилятор не может решить такое уравнение.
    if (n > 1) return n * fac2(n - 1);
    else       return 1;
}
auto foo() {
    return 1;
    return 1.0;  // Не сошлось, ошибка компиляции.
}
```

#### Захват аргументов лямбдой

Зависит от того, как захватывать

* По ссылке - выводим как в шаблонах для ссылок
* init captute
    То есть `[x = vector<int>(10)] () {}` - как для auto, здесь по значению
    Можно делать auto по ссылке: `[&x = vector<int>(10)] () {}`
* По значению - выводим как для шаблонов, НО СОХРАНЯЕМ CV-QUALIFIERS
    Не уверен, кажется, этого нет в плане Егора, но есть у Мейерса. __Спросить__ на консультации.
    То есть, мы сохраним `const` и т.д.
    ```c++
        const int cx = 0;
        auto lam = [cx] { ... };

        // внутри лямбды равзернулось
        class lambda_name // зависит от компилятора, кажется
        {
            private:
                const int cx;
        }
    ```
    Довольно логично сохранять, если мы хотим внутри константную переменную.

#### Можно явно указать тип

И у лямбд, и у функций с auto, поддерживается следующий синтаксис:
```c++
    // лямбды
    auto lam = [] () -> int { return 1.0; };
    
    // функции
    auto foo() -> int { return 1.0; }

    // Вполне можно и темплейтные функции
    template <typename T>
    auto foo() -> T { }

    // например, с forwarding-reference-ом (можно не говорить, чтобы не попутать всё)
    template <typename T>
    auto foo(T&&) -> T&& { }
```

### А ещё есть decltype

Выводит тип выражения, это оператор из выражения в тип.

Даёт `declared type` выражения, то есть то, как он был объявлен.

В отличие от auto, никогда никуда не девает const, volatile, references.

```c++
int x = 10;             // decltype(x) = int
const auto& rx = x;     // decltype(rx) = const int&
```

Притом важно, что если брать decltype от lvalue, то будет T&, не T.
`decltype(lvalue expr of type T) = T&`

Например,
```c++
int arr[10];

// decltype(arr[0]) = int&
```

Есть какие-то мелкие особенности, они не очень важны, их Скотт Мейерс упомянул и пропустил.

#### decltype(auto)

Можно это рассказать немного раньше, это относится к предыдущему пункту.

Можно выводить тип не с помощью шаблонного вывода, а с помощью decltype-а: смотреть на decltype того, что мы пытаемся вернуть, и от этого определять тип
```c++
    int ar[10];
    decltype(auto) declauto_foo1() { // int
        int x = 1;
        return x; 
    }
    
    decltype(auto) declauto_foo2() { // int&
        return ar[0];
    }
    decltype(auto) declauto_foo3() { // int&, потому что declval((x)) - это int&, так как x - lvalue => (x) - lvalue, и она так объявлена
        int x = 1;
        return (x);
    }
```

decltype(auto) вполне может и void вывести, если ничего не возвращается.


