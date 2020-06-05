## Билет 01
Автор: <Имя Фамилия>
## Билет 02
Автор: Носивской Влад

* Зачем: во избежание коллизий. Например, пишем клиент-сервер, класс User есть и у клиента, и у сервера. Для этого заведем namespace client и namespace server. Теперь обращение client::User и server::User соответственно.

* Еще немного про мотивацию:
    * Можно использовать вложенные классы:

        ```C++
        class Database {
        public:
            class User { ... };
        };
        ```

        ```C++
        class Application {
        public:
            class User { ... };
            static Database connectToDb() { ... }
        };
        ```

        Здесь Database::User и Application::User - разные классы. Есть минусы: нужно писать много static-ов и в разных файлах использовать разные классы, либо заводить один God object, который содержит все функции и переменные со всех файлов. 
        
        Зачем здесь написан static для функции connectToDb? static функции как члены класса - это функции, которые не требуют наличия экземпляра класса для вызова (в данном случае мы не обязаны иметь экземпляр класса Application для вызова функции connectToDb).

    * А можно использовать namespaces:
        
        ```C++
        namespace database {
        class Database {
        public:
            class User { ... };
        };
        } // namespace database

        namespace application {
        class User { ... };
        database::Database connectToDb() { ... }
        } namespace application
        ```
* Немного про стиль при использовании namespace:
    * Обычно в одном файле один namespace. Внутри пишется без отступов (см. пример выше)
    * Имя в snake_case
    * namespace можно переоткрывать (в отличие от классов), в том числе в разных файлах. При этом содержимое конкатенируется. 
    * Если пишете библиотеку - хороший тон обернуть ее в namespace, а дальше творить в ней что угодно.
    * Заглянуть внутрь это ::, как у классов.

* Вложенные пространства имен.
    ```C++
    namespace database {
        namespace internal {
            void foo();
            void bar();
            struct Foo {};
        } // namespace internal
        struct User {};
    } // namespace database
    ```

    Начиная с С++11 можно писать так:

    ```C++
    namespace database::internal {
        void foo();
        void bar();
    } // namespace database::internal
    ```

    Чтобы обратиться к глобальному пространству имен, обратитесь будто к безымянному пространству:

    ```C++
    int foo;
    namespace bar {
        int foo;
        void some() {
            ::foo = 0; // Изменилось значение переменной, которая объявлена в глобальном пространстве имен.
        }
    } 
    ```

* Анонимное пространство имен.
    * Вспомним для начала, что такое internal linkage.
        
        header.hpp

        ```C++
        static int var = 82; // Передаю привет Гимназии №82 города Краснодара
        ```

        file1.hpp

        ```C++
        #include "header.hpp"

        void foo() { var = 239; }
        ```

        main.cpp
        ```C++
        #include <iostream>
        
        #include "header.hpp"
        #include "file1.hpp"

        int main() {
            foo();
            std::cout << var << std::endl;
        }
        ```

        Такая программа на выходе выдаст 82, а не 239, так как переменная var объявлялась как static, поэтому в каждом файле, где ее подключили, она своя.

    * Выше был показан сишный подход, в плюсах решили, что для internal linkage будут использовать анонимные пространства имен. А именно, для реализации того, что показано выше, в плюсах мы напишем:

        header.cpp

        ```C++
        namespace {

        int var = 82;

        }
        ```

        Теперь в плюсах не принято писать static-функции, их принято оборачивать в анонимные пространства имен.

* Про typedef и using, а главное - про отличие между ними.

    typedef пришел к нам из Си и означает он синоним имени. using же протаскивает имя.

    Например, можно сказать typedef int score_t; чтобы использовать score_t в коде, это будет псевдоним для int.

    * Вне контекста пространств имен typedef проигрывает using-у, так как ему очень плохо дается работа с шаблонами. Пример:

        ```C++
        template<typename T>
        using myAllocList = std::list<T, myAlloc<T>>;

        myAllocList<Object> ml;
        ```

        С typedef-ом написать подобный код будет больно и сложно (но все же возможно, пусть и использование усложнится).

    * В контексте пространств имен using позволяет протащить имя в точку.

        ```C++
        using database::Database;
        ```

    * Еще можно протаскивать целиком все имена из пространства имен:

        ```C++
        using namespace database;
        ```

        Здесь нужно быть очень аккуратным и уверенным, что не будет коллизий с именами. Поэтому не стоит писать в коде using namespace std; (привет контестам), ведь мы не знаем все имена из стандартной библиотеки. Например, там лежат функции left и right, а назвать так переменную иногда может хотеться. 

        Стоит отметить, что использование using namespace внутри .h/.hpp файлов идея сомнительная. Пользователь может добавить хедер и надеяться, что коллизий не будет.


## Билет 03 "Правила поиска имён (глобальных и внутри классов)"
Автор: Герман Тарабонда

### Где возникает qualified lookup, а где — unqualified

Если простым языком: то для  `foo()` происходит unqualified lookup (то есть просто поиск по имени), а для `::foo()` происходит qualified lookup.

Как вообще это происходит.

Unqulified lookup -- ищем в текущих `{}` где находится наш условный `foo()` и ищем объвление `foo()`. Подробнее об этом будет написано позже.

Qualified lookup -- ищем так как указано. Например: `model::animal::run()`. Сначала мы смотрим на `run()`. Вызывается qualified lookup. Для этого должны найти `animal`. Теперь нужно найти `animal`. Это тоже выполняется qualified lookup'ом. Чотбы найти `animal` нужно найти `model`. А вот поиск `model` происходит с помощью unqulified lookup.

### Явное обращение к глобальному пространству имён (`::foo`)

Все что не лежит явно в namespace'ах лежит в глобальном namespace. Чтобы обратиться к глобальному namespace можно явно указать `::foo`.

### Правила unqualified lookup в телах функций, методов, на уровне пространств имён ("глобально")

Правило такое: везде кроме классов ищем объвление до строчки использования. В классах мы изем во всем его объявлении. Например:

```C++
... // И в конце концов здесь (4)
namespace food {
... // Потом здесь (3)
class Meat {
... // Потом здесь (2)
void fry() {
... // Ищем здесь сначала (1)
slice();
... // Здесь не ищем
}
... // И здесь (2)
}
... // Здесь тоже не ищем
}
... // И тут не ищем
```

### Forward declaration для функций и классов, особенности неполных типов

В чем прикол forward declaration? На самом деле это довольно полезная штука. Оно позволяет программисту написать объявление чего-либо, а затем когда-нибудь написать определение этого чего-либо.

Пример из `lab_11` файл `bin_manip.h`:

```C++
#include <iosfwd>

std::ostream& hex(std::ostream&);
...
```

Здесь нужно только объявление `ostream`, поэтому достаточно подключения `iosfwd`. Это заметно ускоряет компиляцию программу, так как в `iosfwd` написаны только неполные объявления классов.

В `bin_manip.cpp` уже нужно подключать `iostream`, так как там есть вызов функций, которые не объявлены в `iosfwd`.

В `employee.cpp` тоже есть проблемы. Например:

```C++
#include "bin_manip.h"

...(ifstream& f) {
f >> read_le_int32(...);
}
```

Такого рода файл будет выдавать ошибку компиляции, так как не поймет, что `ifstream` - наследник `istream`, так как в `iosfwd` не указаны детали реализации. Поэтому дополнительно в этот файл нужно подключить `fstream`.

Кратко про неполные типы: объвлять можно, пользоваться нельзя. Пример:

```C++
class IncompleteClass; // forward declaration, неполный класс

void foo1(const IncompleteClass* p); // ОК
void foo2(const IncompleteClass& p); // ОК
void foo3(IncompleteClass p); // ОК
void foo4(IncompleteClass p); // не ОК, определение функции

IncompleteClass* p; // ОК
IncompleteClass p; // не ОК
```

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
Автор: Кирилл Карнаухов

### Объект-функтор
Функтором называется класс (или структура) с перегруженным `operator()`. Могут применяться также, как и функции, однако имеют преимущества. Например, хотим посортировать массив по компаратору `a[i] * x`, где `x` — константа. Для обычной функции-компаратора пришлось бы заводить отдельную глобальную константу, что не очень хорошо. Однако в функторе мы можем хранить `x`, как поле класса. Возможная реализация:
```cpp
struct Comparator {
    int x;
    bool operator()(int a, int b) {
        return a * x < b * x;
    }
};
```
Теперь можно использовать этот функтор, например, для сортировки такого вида:
```cpp
template<typename T, typename Cmp>
void sort(Array<T>& array, Cmp cmp) {...}
```
Плюсом ко всему является то, что нет виртуальных вызовов.

### Применение в алгоритмах 
Функторы имеют огромное применение в `std::algorithm`. Например, имея функтор из предыдущего пункта, можно:
```cpp
std::vector<int> arr = {...};
std::sort(arr.begin(), arr.end(), Comparator{1}); //сортировка по возрастанию
std::sort(arr.begin(), arr.end(), Comparator{-1}); //сортировка по убыванию
```
Сюда можно передавать все, отчего можно вызвать `()` без аргументов. 

Примечание: в таких применениях функция сравнения должна быть транзитивной, иначе будет `Runtime Error`.

### Функторы, как компараторы в ассоциативных контейнерах
По умолчанию, `std::set` хранит значения по возрастанию. Хотим хранить, например, `int` по убыванию. Напишем компаратор:
```cpp
struct Comparator {
	bool operator()(int a, int b) {
		return a > b;
	}
};
```
Тогда, если написать `std::set<int, Comparator>`, то контейнер будет хранить ключи по убыванию. 

В общем, случае, нужно передавать функтор, который для `(a, b)` возвращает `true`, когда `a` должен идти *не позже* `b`. 

То же самое можно делать с `std::map` и другими ассоциативными контейнерами. 

Примечание: верно то же самое, что функция должна быть транзитивной. 

### Лямбды
Все это, конечно, очень круто, но иногда нам нужно использовать функтор всего в одном месте, и не как-то не хочется для этого создавать отдельный класс. Или нам хочется создать функцию внутри другой функции. Для этого придумали лямбды. Синтакис такой:
```cpp
auto func = [](int a, int b) {
	return a + b;
};
```
По сути, лямбды — это синтаксический сахар. На самом деле, это просто функтор с неизвестным типом. Применение:
```cpp
std::sort(arr.begin(), arr.end(), [](int a, int b) { return a > b; }); //сортировка по убыванию
```
По умолчанию, в лямбдах нельзя ничего использовать, помимо глобальных переменных. Чтобы была возможность брать переменные из области видимости, есть так называемые захваты:
```cpp
int x = 100;

auto cmp1 = [x](int a, int b) { return a * x < b * x; }; //захват по значению (то есть значение x скопируется)
auto cmp2 = [&x](int a, int b) { return a * x < b * x; }; //захват по ссылке
auto cmp3 = [=](int a, int b, int c, std::string d) { ... }; //захват всей области видимости по значению
auto cmp4 = [&](int a, int b, float f) { ... }; //захват всей области видимости по ссылке

struct smth {
	void func() {
		auto f = [this]() { ... }; //захват всего экземпляра класса
	}
};
``` 

### Подробности лямбды
Как уже говорилось, нельзя явным образом получить тип лямбды. Однако, если мы хотим указать тип, например, для шаблонов, то можно использовать `decltype`.

Лямбда превращается в анонимный класс с `operator()`. 

Также, комппилятор все оптимизирует на этапе компиляции.

### mutable для лямбды
По умолчанию, лямбда создает функтор с `operator()`, который является `const-qualified`. Но иногда нам все-таки хочется менять переменные внутри оператора. Для этого можно написать `mutable`, то есть:
```cpp
int x = 10;
auto func = [&x](int a, int b) mutable {
	x--;
	return a * x < b * x;
};
```

### Возвращаемое значение
Лямбда сама узнает, какое возвращаемое значение у `operator()`. Например, здесь это `int`:
```cpp
auto func = [](int x) { return x * x; };
```
Если функция возвращает несколько значений, то компилятор смотрит на первый `return`. То есть следующий код **не** скомпилируется:
```cpp
auto func = [](int x, double y) {
	if (x > 0) {
		return 0;
	} else {
		return y;
	}
};
```
Это происходит, потому что компилятор не понимает, что именно мы хотим вернуть. Однако, можно использовать касты. 

Также можно явно указать тип возвращаемого значения:
```cpp
auto func = [](int a, double y) -> double { ... };
```

### Шаблонные параметры
Пока полноценные шаблоны для лямбд в C++ не завезли, но есть `auto`.  Например, можно так:
```cpp
std::vector<std::string> array = { ... };
std::sort(array.begin(), array.end(), [](const auto &fir, const auto &sec) {
	return a.length() < b.length();
});
```
Компилятор сам поймет, какие типы у аргументов. 

### Отличия от `std::function`
Внутри `std::function` хранится указатель на выделенный объект на куче, у которого нужно вызвать `()`. Можно ли хранить лямбды в `std::function`? Да, можно. Нужно ли? Нет, нужно пытаться этого избегать.

Проблема в том, что вызов метода у `std::function` — это виртуальный вызов и он не всегда будет соптимизирован. В то же время, лямбда не имеет виртульаных вызовов. Из-за этого `std::function` может работать сильно медленнее обычной лямбды.

Если хотите сохранить лямбду, можете сохранить ее в шаблон.

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

## Билет 15
Автор: <Имя Фамилия>
## Билет 16
Автор: <Имя Фамилия>
## Билет 17
Автор: <Имя Фамилия>
## Билет 18
Автор: Тарасов Денис

* ### `static_cast<Target>(expr)`
  Почти всегда, когда нам необходим какой-то cast следует использовать `static_cast`. Позволяет делать все неявные преобразования из `expr` в `Target`, а так же все обратные к ним.
  * Преобразования между числовыми типами, в том числе с потерей точности.
  ```C++
  char c = 1;
  int i = 2;
  i = c; // Неявное расширяющее преобразование
  i = static_cast<int>(c); // То же самое
  c = i; // Обратное к неявному
  c = static_cast<char>(i); // То же самое
  ```
  * Знает о конструкторах типа.
  ```C++
  struct Foo1 {};
  struct Foo2 {};
  struct Bar {
      Bar() : val(1) {}
      Bar(Foo1) : val(2) {}
      explicit Bar(Foo2) : val(3) {}
      int val;
  };  
  Bar bar;
  bar = static_cast<Bar>(Foo1());
  assert(bar.val == 2);
  bar = static_cast<Bar>(Foo2());
  assert(bar.val == 3);
  ```
  * Перобразование от базового типа к наследнику. `static_cast` следит за положением базовых классов в памяти и корректно пересчитывает указатели.
  ```C++
  struct Base1 {int x};
  struct Base2 {int y};
  struct Derived : Base1, Base2 {int x};
  Derived *d = new Derived;
  Base1 *b1 = d; // Можно неявно
  Base2 *b2 = d; // Можно неявно
  assert(b1 != b2);
  Derived *d1 = static_cast<Derived*>(b1); // Нужен static_cast
  assert(b1 == d1);
  Derived *d2 = static_cast<Derived*>(b2); // Нужен static_cast
  assert(b2 != d2);
  ```
  * Используется для преобразований от/к `void*` (может понадобиться например при вызове C-функций), при этом значение указателя не меняется. Нельзя таким образом делать преобразование между произвольными указателями.
  ```C++
  char *pc;
  int *pi;
  pc = static_cast<char *>(pi); // CE
  pi = static_cast<int *>(pc); // CE
  void *pv = static_cast<void*>(pi);
  assert(pv == pi);
  int *pi1 = static_cast<int*>(pv);
  assert(pi == pi1);
  ```
  * Можно глушить предупреждение о том, что переменная не используется.
  ```C++
  Foo foo;
  static_cast<void>(foo); // Убирает warning
  ```

* ### `reinterpret_cast<Target>(expr)`
Нужен очень редко, небезопасен. Говорит компилятору интерпретировать `expr` как `Target`. Не компилируется в процессорные инструкции.
  * Не дает никаких гарантий на промежуточные преобразования,
  ```C++
  int *a = new int();
  void *b = reinterpret_cast<void*>(a); // Нет гарантии, что a == b
  int *c = reinterpret_cast<int*>(b);
  assert(a == c); // Лишь здесь есть гарантия
  ```
  * Может преобразовывать любые указатели/ссылки в любые другие, но часто ведет к UB из-за strict aliasing (кажется, на лекциях о нем не было).
  ```C++
  float *d = reinterpret_cast<float*>(a);
  cout << *d; // UB: нарушение strict aliasing: доступ к int через несовместимый float*
  ```
  * Преобразование из пункта выше разрешаются для `char*` и `unsigned char*` (но не `signed char*`, который является отдельным типом). Является одним из немногих разумных использований.
  ```C++
  Foo *f = new Foo();
  char *c = reinterpret_cast<char*>(foo);
  unsigned char *uc = reinterpret_cast<char*>(foo);
  cout << *c << *uc; // Не UB
  ```
  * Может конвертировать указатели в числа и обратно, если хватает места.
  * Не следит за корректностью указателей при работе с наследованием, как это делает `static_cast`.

* ### `const_cast<Target>(expr)`
Отбрасывает константность у `expr`. UB, если `expr` был константным. Используется редко, не безопасен.
  * Пример с UB и без.
  ```C++
  vector<int> vec;
  const auto &rvec = vec;
  const_cast<vector<int>&>(rvec).clear();  // Окей, т.к. vec не const
  const vector<int> cvec;
  const_cast<vector<int>&>(cvec).clear();  // UB
  ```
  * Разумное использование при написании `operator[]`. Уже реализована константная перегрузка, при этом в неконстантной нужны те же действия. В обратную сторону может быть UB.
  ```C++
  const T& operator[](size_t i) const { .. }
  T& operator[](size_t i) {
      return const_cast<T&>(static_cast<const vector<T>&>(*this)[i]); // Нет UB
  }
  ```
  * Второе разумное использование: совместимость со старым C.
  ```C++
  void println(char *s); // Мы уверены, что s не меняется внутри функции
  println(const_cast<char*>("foo"));
  ```

* ### `dynamic_cast<Target>(expr)`
    Единственный каст, позволяющий работать с полиморфными объектами (те у кого есит виртуальные функции). Позволяет узнать тип объекта во время выполнения программы. В случае неверного преобразования возвращает `nullptr` для указателей и кидает исключение для ссылок. Безопасен.

    ```C++
    struct Base { virtual ~Base(); };
    struct Foo : Base {};
    struct Bar : Base {};
    struct Derived : Foo, Bar {};

    Foo *f = ..;
    Base *b = f;  // Неявное преобразование (basecast).
    Derived *d1 = static_cast<Derived*>(f);  // Derivedcast. UB, если f не был Derived.
    Derived *d2 = dynamic_cast<Derived*>(f);  // Derivedcast. nullptr, если f не был Derived.
    Bar *bar = dynamic_cast<Bar*>(f);  // Crosscast. static_cast не умеет. Сначала преобрзует
        // к Derived (derivedcast), а затем к Bar (basecast). В случае неудачи nullptr.

    Foo &rf = f;
    Derived &rd1 = static_cast<Derived&>(rf);  // Возможно UB.
    Derived &rd2 = dynamic_cast<Derived&>(rf);  // Возможно исключение std::bad_cast.
    ```

* ### ` C-style-cast`
  Пытается применить какую-то цепочку преобразований, чтобы код скомпилировался.
  * Синтаксис (есть и другой).
  ```C++
  int i = (int)10.5;
  T foo = (T)expr;
  ```
  * Пробует следующую цепочку преобразований
    * const_cast
    * static_cast и разрешает ещё пару преобразований
    * reinterpret_cast
    * На каждый вариант пытается ещё навесить const_cast
  * Не надо использовать совсем. Лучше попробовать `static_cast`, вызов конструктора или расписать цепочку явно.

* ### `dynamic_cast`, виртуальное наследование и еще примеры
```C++
struct Base { virtual ~Base() {} };
struct Foo : virtual Base {};
struct Bar : virtual Base {};
struct Derived : Foo, Bar {};
struct Derived2 : Derived {};
int main() {
    Derived *d = new Derived2; // Неявно от наследника к базе
    Derived2 *d2 = static_cast<Derived2*>(d); // Обратно нужен static_cast
    Derived *dd = d2; // Снова неявно как в первый раз
    const Base *a = d; // Можно неявно, т.к. Base только один (виртуальное наследование)
#if 0
    // Код внутри этого #if не компилируется из-за виртуального наследования.
    // Т.к. не знаем на этапе компиляции расположения Foo и Base.
    Foo *f = static_cast<Foo*>(a);
#endif
    const Foo *f = dynamic_cast<const Foo*>a; // Единственный выход,
        // т.к. он сможет изучить строение объекта во время выполнения.
    const Bar *b = dynamic_cast<const Bar*>(f); // Кажется, здесь crosscast.
    const Derived *ddd = static_cast<const Derived*>(f); // Foo не виртуальная база Derived, 
        // поэтому static_cast достаточно, но можно и dynamic_cast
    Derived *cddd = const_cast<Derived*>(static_cast<const Derived*>(f)); // Один нельзя, 
        // необходимо быть увереным в наших действиях
}
```
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