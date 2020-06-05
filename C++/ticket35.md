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
