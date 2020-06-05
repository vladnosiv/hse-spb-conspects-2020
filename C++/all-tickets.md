## Билет 01
Автор: <Имя Фамилия>
## Билет 02
Автор: <Имя Фамилия>
## Билет 03
Автор: <Имя Фамилия>
## Билет 04
Автор: <Имя Фамилия>
## Билет 05
Автор: <Имя Фамилия>
## Билет 06
Автор: <Имя Фамилия>
## Билет 07
Автор: <Имя Фамилия>
## Билет 08
Автор: <Имя Фамилия>
## Билет 09
Автор: <Имя Фамилия>
## Билет 10
Автор: <Имя Фамилия>
## Билет 11
Автор: <Имя Фамилия>
## Билет 12
Автор: <Имя Фамилия>
## Билет 13
Автор: <Имя Фамилия>
## Билет 14
Автор: <Имя Фамилия>
## Билет 15
Автор: <Имя Фамилия>
## Билет 16
Автор: <Имя Фамилия>
## Билет 17
Автор: <Имя Фамилия>
## Билет 18
Автор: <Имя Фамилия>
## Билет 19
Автор: <Имя Фамилия>
## Билет 20
Автор: <Имя Фамилия>
## Билет 21
Автор: <Имя Фамилия>
## Билет 22
Автор: <Имя Фамилия>
## Билет 23
Автор: <Имя Фамилия>
## Билет 24
Автор: Пётр Сурков

### STL

* Containers libary
* Iterators libary
* Algoritms libary

Этот билет — Containers libary. В следующих билетах есть подробнее про каждый из видов контейнеров в нём. 

А ещё тут спрашиваются отличия, например, stdio.h и cstdio. В C используем первый, в C++ — второй, он не загрязняет глобальное пространство имён, а кладёт всё в `std::`

### Containers libary
Хранит объекты и управляет их временем жизни.

Должен быть

* Деструктор
* Хотя бы некоторые из конструкторов (копирования, перемещения, создания с параметрами, в зависимости от вызываемых методов)
* Обычно должны быть операторы присванивания (копирования, перемещения)

Контейнеры инвариантны по типам: `vector<Base*>` и `vector<Derived*>` - независимые контейнеры. Скопировать второй в первый не получится.

#### Аллокатор

Памятью управляет аллокатор - шаблонный класс, умеющий выделять, удалять память, инициализировать и удалять объект. 

Обычно используется `std::allocator`, он вызывает `new/delete`, которые можно [перегружать](https://habr.com/ru/post/490640/).

Аллокатор - сложный объект. должен уметь выделять не только `T`. Например, в `list<>` нужно уметь выделять внутренний тип (структуру с `T` и указателями на следующий элемент).

К тому же, до C++11 аллокаторы не могли иметь глобального состояния.

Из-за этого есть специфичные требования к написанию собственных аллокаторов.

#### Сложность и гарантии исключения
Стандарт обещает какое-то время работы. Например, копирование `vector<T>` происходит за `O(n)` копирований `T`. 

Базовая гарантия есть всегда, про строгую каждой операции надо читать в стандарте. Но если move T кидает исключения, то многие строгие гарантии рушатся. 

#### Виды контейнеров

* Последовательные (хранят элементы в фиксированном пользователем порядке): `vector`, `deque`, `list`, `forward_list`.
* Ассоциативные (сортируют элементы по ключу): `map`, `set`, `multimap`, `multiset`.
* Неупорядоченные ассоциативные (хэшируют элементы по ключу): `unordered_*`.
* Адаптеры (надстраиваются над другими контейнерами): `queue`, `priority_queue`, `stack`.

#### Стандартные операции, есть всегда:

* Конструктор по умолчанию `O(1)`
* Конструктор перемещения `O(n)` (при копировании `T` за `O(1)`)
* `a.swap(b)`, `swap(a,b)` за константу для всех, кроме `std::array`
* `.empty()` (константа)
* `.size()` (константа, даже у списка)
* Конструктор копирования, если `CopyInsertable` (есть конструктор копирования у `T`):
* `==`/`!=`, если объекты можно сравнивать на `==`/`!=`:    
* Для обобщённого программирования:
    * `vector<T>::value_type`
    * `vector<T>::reference_type`
    * `vector<T>::const_reference`
    * `vector<T>::size_type`
    * `vector<T>::difference_type`

#### Некоторые из гарантий исключений
##### noexcept
* `erase` (если только не бросит исключение move/copy оператор/конструктор в `vector`/`deque`)/`clear`/`swap`
* `pop_back`/`pop_front` (если есть)

##### Строгая гарантия
* `insert`/`emplace` (если вставляет один элемент, а не много сразу; и то только если с нужного конца, см. `vector`/`deque`)
* `push_back`/`push_front`/`emplace_back`/`emplace_front` (если есть)


#### Range-based for
Синтаксический сахар:
```cpp
for (DECL : EXPR) BODY
```
разворачивается как
```cpp
{
    auto range = EXPR;
    auto begin = std::begin(range);
    auto end = std::end(range);
    for (auto it = begin; it != end; ++it) {
        DECL = *it;
        BODY
    }
}
```
Функции `std::begin`/`std::end` вызывают внутренние `begin`/`end`, но перегружены для обычных массивов.

Есть тонкость. В `for (int &x : foo().bar()) { ... }` временное значение `foo()` не живёт, но `bar()` живёт. 

[Пример](https://cppinsights.io/) развёртки сахара.

Как видно из раскрытия, не стоит удалять/добавлять элементы в контейнер проходясь range-based for по нему.

В билете №28 можно прочитать про ranged-based ещё раз и найти информацию про связь с ADL, но в данном билете это не требуется.

#### Удаление, Инвалидация
Функция `.erase` возвращает итератор на элемент, который теперь занимает место первого удалённого элемента, либо `end()` при его отсутствии. 

Если идём по контейнеру, удаляя элементы по текущему итератору, то пишем `it=v.erase(it)`, вместо просто `v.erase(it)`. Иначе сделав `it++` на следующем шаге можем получить инвалидированный итератор, так как `it` указывал на уже удалённый элемент.

Про другие варианты инвалидации можно почитать в билете №28. 
## Билет 25
Автор: <Имя Фамилия>
## Билет 26
Автор: <Имя Фамилия>
## Билет 27
Автор: <Имя Фамилия>
## Билет 28
Автор: <Имя Фамилия>
## Билет 29
Автор: <Имя Фамилия>
## Билет 30
Автор: <Имя Фамилия>
## Билет 31
Автор: Александр Морозов
## Билет 32
Автор: <Имя Фамилия>
## Билет 33
Автор: <Имя Фамилия>
## Билет 34
Автор: Александр Морозов
## Билет 35
Автор: Сурков Пётр

### Мотивация метапрограммирования

#### Обобщения
* `std::distance` должен работать для разных итераторов по-разному:
    * Для random access iterator: `last - first`
    * Для остальных: `while (first != last) ++first;`
* `vector<T>::resize` может вести себя по-разному:
    * Если есть nothrow move constructor, то можно сразу перемещать элементы из буфера в буфер
    * Если нет, то надо всегда копировать, чтобы была strong exception safety.
* `std::any`/`std::function` — хотим small object optimization (если объект маленький, то выделим его на стеке, а не в куче). Т.е. поведение объекта зависит от типа, который скормили конструктору.

#### Компилятор делает рутину за нас
##### Пример 1:
```c++
int dijkstra(const Graph &g, int start, int end);
int floyd(const Graph &g, int start, int end);
int random_shuffle(const Graph &g, int start, int end);
// Хотим, чтобы можно было писать так:
check_answer_same(dijkstra, floyd, random_shuffle)(some_graph, 0, n - 1);
```
##### Пример 2:
Хотим сделать Serialize, аргументы которого представляют собой названия параметров и откуда их брать.
```c++
using PersonSerializer =
    Serialize<"name", &Person::getName, // &Person::getName - указатель на функцию
              "age", &Person::getAge>;
PersonSerializer s(std::cin); 
s << getPerson();  // name=Ivan, age=23
```
##### Пример 3:
Пишем калькулятор, хотим добавлять в него функции.
```c++
CalculatorFunctions functions = {
    {"sin", std::sin},
    {"func", [](double a, double b, double c) { return a + b * c; }}
};
```
##### Пример 4:
DFS в compile time (dependency injection): есть куча классов с конструкторами, надо собрать объект.

### Constexpr-вычисления

* Можно использовать многие конструкции языка: нельзя `goto` и не-`constexpr` функции.
* Можно использовать скалярные типы и типы с `constexpr`-конструктором и тривиальным деструктором (вроде `optional` от простых типов).
* С C++14 можно менять переменные внутри функций!
* До C++20 нельзя выделять память => нельзя `vector`

Пример:

```c++
constexpr int factorial(int n) {
    int res = 1;
    optional<int> unused1;
    // vector<int> unused2(n); нельзя до C++20 
    for (int i = 1; i <= n; i++)
        res *= i; // Можно с C++14
    return res;
}
static_assert(factorial(5) == 120);
// ...
assert(factorial(5) == 120);
std::vector<int> vec(factorial(5));
std::array<int, factorial(5)> arr;
```

### "Функции" из типов в типы
Ещё с C++98 бывают функции из типов в типы:
```c++
template<typename T>
struct iterator_traits {  // Общий случай
    using value_type = typename T::value_type;
    using difference_type = typename T::difference_type;
};
template<typename T>
struct iterator_traits<T*> {  // Частный случай
    using value_type = T;
    using difference_type = std::ssize_t;
};
```

Можно несколько входов, можно несколько выходов, можно
простые типы в качестве параметров шаблона.

Вызов:
```c++
typename iterator_traits<Iterator>::value_type x = *it;
```
Важно указать `typename`, иначе компилятор не поймёт смысл `<`, распарсит как знак меньше.

### "Функция" из типов в один тип
Если такая "функция" возвращает ровно один тип, есть конвенция:
```c++
template<typename T> struct remove_const {
    using type = T;
};
template<typename T> struct remove_const<const T> {
    using type = T;
};
```
В C++11 появились псевдонимы шаблонов (alias template), конвенция:
```c++
// В библиотеке делаем псевдоним с суффиксом _t
template<typename T> using remove_const_t = typename remove_const<T>::type;
// В коде
remove_const_t<const int> x = 10;
```
Получаем вызов функции, только `<>` вместо `()`, и в качестве параметров — и значения, и типы.

### "Функция" из типов в число
Напишем функцию, вычисляющую кол-во размерностей переданного массива
```c++
template<typename T> struct rank { // общий случай
    static constexpr size_t value = 0;
};
template<typename T> struct rank<T[]> { // передали массив, мы не знаем его размер
    static constexpr size_t value = rank<T>::value + 1;
};
template<typename T, size_t N> struct rank<T[N]> { // передали массив, мы знаем его размер
    static constexpr size_t value = rank<T>::value + 1;
};
static_assert(rank<int[][10]>::value == 2);
```
Раньше, такое [писали](https://stackoverflow.com/a/205000/767632) так `struct rank { enum { value = 10 }; }` из-за багов компилятора. Сейчас так не надо.

Заметим, что `constexpr`-функции тут не хватит. Она не сможет заглянуть внутрь типа.

С C++17 есть конвенция создавать template variable:
```c++
// В библиотеке делаем псевдоним с суффиксом _v
template<typename T> constexpr size_t rank_v = rank<T>::value;
// Тогда в коде сможем писать чуть короче
static_assert(rank_v<int[][10]> == 2);
```

Снова получаем вызов функции с `<>` вместо `()`.

### Стандартные вспомогательные классы
```c++
template<typename T> struct rank
    : std::integral_constant<size_t, 0> {}; // разобрали общий случай
template<typename T> constexpr size_t rank_v = rank<T>::value; // завели вспомогательный using
template<typename T> struct rank<T[]>
    : std::integral_constant<size_t, rank_v<T> + 1> {}; // частный случай
template<typename T, size_t N> struct rank<T[N]>
    : std::integral_constant<size_t, rank_v<T> + 1> {}; // частный случай
static_assert(rank<int[][10]>::value == 2);
```
Обычно пишут через них, так короче.

В библиотеке реализованы примерно так:
```c++
template<typename T, T v>
struct integral_constant {
    static constexpr T value = v;
    using value_type = T;
    constexpr operator T() const noexcept { return v; }
    ....
};
template<bool v>
struct bool_constant : integral_constant<bool, v> {};
using true_type = bool_constant<true>;
using false_type = bool_constant<false>;
```

### Тонкости с числами в параметрах
Бывают ограничения. В специализации нельзя фиксировать значение, не фиксируя тип значения:
```c++
template<typename T, T N> struct fac { .... };
template<typename T>      struct fac<T, 0> { .... };  // Нельзя( Ну вдруг 0 не типа T на самом деле
```

Но их можно обходить, если специализировать и тип тоже, в этом поможет `integral_constant`:
```c++
template<typename /*N*/> struct fac {};  // Базовый случай, вызовется только если не integral_constant
// Не будет value, поэтому если нам дадут какую-то фигню а не число, то, скорее всего, будет ошибка компиляции
// это хорошо
template<typename T, T N> constexpr auto fac_v =
    fac<std::integral_constant<T, N>>::value;
template<typename T, T N> struct fac<std::integral_constant<T, N>>
    : std::integral_constant<T, N * fac_v<T, N - 1>> {};
template<typename T>      struct fac<std::integral_constant<T, 0>>
    : std::integral_constant<T, 1> {};
static_assert(fac_v<int, 5> == 120);
```
Если передать отрицательный `N`, то компилятор зациклится и на глубине примерно 1000 выдаст ошибку компиляции.
Так как это дело "открывали", могут быть ещё сюрпризы...

### Простые type traits
Классы или структуры, которые только таскают информацию в типах, называются "типажи" (traits).

Есть встроенные в язык "кирпичики" из `type_traits`, которые можно сделать самому:

* `std::remove_const` (уже видели)
* `is_same<A, B>` совпадают ли типы? 
* `is_convertible<From, To>` — можно ли сконвертировать `From` в `To` неявно? Это позволит узнать, например, будет ли компилиться `To foo() { return From{}; }`
* `is_nothrow_convertible<T, U>` — то, что выше, но и `noexcept`? Важно: просто смотрит на пометку `noexcept`, никак не смотрит на реализацию.
* `is_constructible<T, A, B>` — есть ли конструктор `T` из `A` и `B`?

### Можно написать свой
```c++
template<typename T> struct is_reference { constexpr static bool value = false; };
template<typename T> struct is_reference<T&> { constexpr static bool value = true; };
template<typename T> struct is_reference<T&&> { constexpr static bool value = true; };
```
### Сложные type traits
Некоторые принципиально требуют поддержки компилятора:

```c++
// Цитата из libc++ Егора: вызываем кусок компилятора  __is_polymorphic
template<typename _Tp> struct is_polymorphic
  : public integral_constant<bool, __is_polymorphic(_Tp)> { };
```

Какие-то можно писать самому, но лучше не надо:
```c++
template<typename> struct is_integral       : false_type {};
template<>         struct is_integral<int>  : true_type {};
template<>         struct is_integral<char> : true_type {};
// И так далее; зависит от компилятора и библиотеки.
```

Некоторые есть в библиотеках (например, `boost::function_traits`. Позволяет узнать кол-во аргументов, их типы, тип возвращаемого значения). 

Для функций одного аргумента легко реализовать самим:
```c++
template<typename> struct function_traits {};
template<typename Ret, typename Arg0>  // Можно написать обобщённо для N аргументов.
struct function_traits<Ret(*)(Arg0)> {
    static constexpr std::size_t arity = 1;
    using return_value = Ret;
    using arg_0 = Arg0;
};
```

### Расширяемые type traits
Некоторые traits предполагаются для расширения пользователем.

Пример: `iterator_traits`. Это такое хранилище свойств (как и любой traits) итераторов. Например,  чтобы мы могли одинаковым кодом спросить `value_type` и у итератора на вектор, и у указателя на массив. По умолчанию берёт `typename` изнутри типа, но можно сделать что-нибудь своё для своего итератора:
```c++
namespace std {
    template<typename T> struct iterator_traits<MyMagicIterator<T>> {
        using value_type = typename MyMagicIterator<T>::magic_value_type;
        // ....
    };
}
```
в примере выше мы захотели, чтобы у нашего итератора в качестве `value_type` брался не просто `value_type` нашего итератора, а его `magic_value_type`.

Ещё пример: `char_traits` задаёт логику работы с "символами", [пример с SO](https://stackoverflow.com/a/5319855/767632):

```c++
  struct CaseInsensitiveTraits : std::char_traits<char> {
      static bool lt(char a, char b) { return tolower(a) < tolower(b); } 
      static bool eq(char a, char b) { return tolower(a) == tolower(b); }
      static int compare (const char *a, const char *b, size_t length) {
          ....
      }
  };
  using CaseInsensitiveString = std::basic_string<char, CaseInsensitiveTraits>;
```

* Получили строчки `CaseInsensitiveString`, которые сравнивается без учёта регистра.
* И заодно дают UB на не-латинице из-за `tolower`.
* Говорят, что из-за своих `char_traits` получается неудобный ужас :(
## Прочие мелочи, не в билетах

### Строковые литералы
Тип строковых литералов - массив `const char` фиксированной (на этапе компиляции) длины. Это позволяет писать такие шаблонные функции:
```c++
template<size_t N>
constexpr auto parse_format(const char (&s)[N]) {
    int specifiers = 0;
    array<char, N> found{};
    for (size_t i = 0; i < N; i++) {  // Вот в этом месте так как strlen не constexpr, то если бы приняли как char*, то не смогли узнать до куда идти циклом.
        if (s[i] == '%') {
            if (i + 1 >= N)
                throw std::logic_error("Expected specifier after %");
            i++;
            found[specifiers++] = s[i];
            if (!(s[i] == 'd' || s[i] == 'c' || s[i] == 's'))
                throw std::logic_error("Unknown specifier");
        }
    }
    return pair{specifiers, found};
}
static_assert(parse_format("hello%d=%s").first == 2);
static_assert(parse_format("hello%d=%s").second[0] == 'd');
static_assert(parse_format("hello%d=%s").second[1] == 's');
```
Таким образом, вот эта [библиотека](https://github.com/hanickadot/compile-time-regular-expressions) позволяет работать с регулярками в стиле `ctre::match<"[a-z]+([0-9]+)">(s)`