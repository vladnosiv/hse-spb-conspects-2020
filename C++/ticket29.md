## Билет 29
Автор: Никита Абрамов

В связи с некой специфичностью билета: я постараюсь дать общую характеристику каждому алгоритму, все что было дан на лекции. Если нужны какие-то специфичные детали, про которые нам не рассказывалось - обычно в начале билета есть ссылка на cppreference

#### Конвенции именований стандартной библиотеки, отличия stdio.h и cstdio

Язык С++ позволяет использовать всю стандартную библиотеку языка С, но появляется еще и собственная стандартная библиотека. Заметим, что STL (standard template library) - кусок стандартной библиотеки языка C++. Конвенция в стандартной библиотеке C++: заголовок пишется без разширений (например, без `.h`). Добавил что-то свое в `std` - за парой исключений получил UB. Для стандартных заголовков из C есть два варианта их подключения:

* `<stdio.h>` - попадает в глобальный namespace (`::fopen`), может положить что-то в `std`
* `<cstdio>` - попадает в `std` (`::std::fopen`), может положить что-то в глобальный

Можно использовать оба, но лучше придерживаться одного стиля и использовать один.  
`#include <bits/stdc++.h>` — нестандартный заголовок из GCC, зависит от версии, лучше не использовать.

#### Конвенции сравнения элементов: == и <

Всегда либо == (отношение эквивалентности: рефлексивность, симметричность, транзитивность), либо < (отношение линейного порядка: антирефлексивность; если ни один не меньше, то равны; транзитивность). Активно используются функторы, всегда можно выдать свой вместо стандартного (std::less) (скорее всего просто вызывает внутри себя <).

Никогда не управляют временем жизни объектов. Это к контейнерам. Только переприсваивают.

Если что-то не нашёл, то обычно возвращается end()/last.

#### Примеры немодифицирующих, сравнивающих, модифицирующих алгоритмов
##### Немодифицирующие
Заметим, что немодифицирующие - это не означает, что с помощью этого алгоритма модифицировать нельзя. Мы можем модифицировать, но модифицировать будет **предикат**
Рассмотрим первую группу, которая проверяет выполнен ли предикат для каждого из элементов диапазона \[first, last)  
[Если нужно - cppreference](https://en.cppreference.com/w/cpp/algorithm/all_any_none_of)
* `bool all_of(first, last, pred)` - унарный предикат выполняется для всех элементов
* `bool any_of(first, last, pred)`- унарный предикат выполняется для какого-то из элементов
* `bool none_of(first, last, pred)` - унарный предикат не выполняется ни для какого из элементов  
```C++
struct DivisibleBy // проверяет делится ли число на d
{
   const int d;
   DivisibleBy(int n) : d(n) {}
   bool operator()(int n) const { return n % d == 0; }
};
 
if (std::any_of(v.cbegin(), v.cend(), DivisibleBy(7))) {
   std::cout << "At least one number is divisible by 7\n";
}
```  
Следующая группа - это такие алгоритмы, которые насчитывают что-то для всех элементов из диапазона \[first, last)  
* `UnaryFunction for_each( InputIt first, InputIt last, UnaryFunction f )`[ссылка](https://en.cppreference.com/w/cpp/algorithm/for_each)
* `InputIt for_each_n(InputIt first, Size n, UnaryFunction f)` [ссылка](https://en.cppreference.com/w/cpp/algorithm/for_each_n)

Первый из них для каждого элемента применяет предикат, а после его возвращает(`std::move`) (может и не возвращать ничего, если это нам не нужно). Это сделано, чтобы, например, можно было насчитать фунцию на всех элементах, а потом получить результат и работать с ним.
Пример для этого:
```C++
#include <vector>
#include <algorithm>
#include <iostream>
 
struct Sum
{
    void operator()(int n) { sum += n; }
    int sum{0};
};
 
int main()
{
    std::vector<int> nums{3, 4, 2, 8, 15, 267};
 
    auto print = [](const int& n) { std::cout << " " << n; };
 
    std::cout << "before:";
    std::for_each(nums.cbegin(), nums.cend(), print); // 3 4 2 8 15 267
    std::cout << '\n';
 
    std::for_each(nums.begin(), nums.end(), [](int &n){ n++; }); //увеличили все элементы на 1
 
    // calls Sum::operator() for each number
    Sum s = std::for_each(nums.begin(), nums.end(), Sum()); //насчитали сумму
 
    std::cout << "after: ";
    std::for_each(nums.cbegin(), nums.cend(), print); // 4 5 3 9 16 268
    std::cout << '\n';
    std::cout << "sum: " << s.sum << '\n'; //305
}
```
Второй - применяет для всех \[first, first + n) предикат f и возвращает итератор `first + n`. Если `n < 0` - UB. 
Хочет, чтобы у предиката была сигнатура: `void fun(const Type &a);` (`operator()` тоже подоходит) 
Пример использования:
```C++
#include <algorithm>
#include <iostream>
#include <vector>
 
int main()
{
    std::vector<int> ns{1, 2, 3, 4, 5};
    for (auto n: ns) std::cout << n << ", ";
    std::cout << '\n'; // 1, 2, 3, 4, 5, 
    std::for_each_n(ns.begin(), 3, [](auto& n){ n *= 2; }); // первые 3 элемента умножили не 2
    for (auto n: ns) std::cout << n << ", ";
    std::cout << '\n'; // 2, 4, 6, 4, 5,
}
```
Следующая группа - это посчитать количество элементов в диапазоне \[first, last). [ссылка] (https://en.cppreference.com/w/cpp/algorithm/count)
Есть две разновидности: 
* `difference_type count(first, last, value) // считает элемент, которые равны value(вызывает operator==)` 
* `difference_type count_if(first, last, pred) // считает элемент, которые удовлетворяют предикату` 
`difference_type` - то, что вернет разность итераторов у конкретного контейнера. 
Пример: 
```C++
#include <algorithm>
#include <iostream>
#include <vector>
 
int main()
{
    std::vector<int> v{ 1, 2, 3, 4, 4, 3, 7, 8, 9, 10 };
 
    // determine how many integers in a std::vector match a target value.
    int target1 = 3;
    int target2 = 5;
    int num_items1 = std::count(v.begin(), v.end(), target1);
    int num_items2 = std::count(v.begin(), v.end(), target2);
    std::cout << "number: " << target1 << " count: " << num_items1 << '\n';
    std::cout << "number: " << target2 << " count: " << num_items2 << '\n';
 
    // use a lambda expression to count elements divisible by 3.
    int if (result2 != std::end(v)) {
        std::cout << "v contains: " << n2 << '\n';
    } else {
        std::cout << "v does not contain: " << n2 << '\n';
    }
} num_items3 = std::count_if(v.begin(), v.end(), [](int i){return i % 3 == 0;});
    std::cout << "number divisible by three: " << num_items3 << '\n';
}
```
Следующая группа алгоритмов - это какие-то алгоритмы для поиска. Их тоже 3 вида: 
* `InputIt find( InputIt first, InputIt last, const T& value );`Ищет первый элемент, равный `value`
* `InputIt find_if( InputIt first, InputIt last, UnaryPredicate p );`Ищет первый элемент, удовлетворяющий предикату
* `InputIt find_if_not( InputIt first, InputIt last, UnaryPredicate q );`Ищет первый элемент, не удовлетворяющий предикату
Если такой элемент не найден, то вернется `last`. 
Пример
```C++
#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
 
int main()
{
    int n1 = 3;
    int n2 = 5;
 
    std::vector<int> v{0, 1, 2, 3, 4};
 
    auto result1 = std::find(std::begin(v), std::end(v), n1);
    auto result2 = std::find(std::begin(v), std::end(v), n2);
 
    if (result1 != std::end(v)) {
        std::cout << "v contains: " << n1 << '\n'; \\3
    } else {
        std::cout << "v does not contain: " << n1 << '\n'; 
    }
 
    if (result2 != std::end(v)) {
        std::cout << "v contains: " << n2 << '\n';
    } else {
        std::cout << "v does not contain: " << n2 << '\n'; \\5
    }
}
```
#### Erase-remove idiom, встроенный метод для list, своя реализация remove_if
#### Двоичный поиск: отличия lower_bound и upper_bound
#### Особенности использования двоичного поиска для не-RandomAccess итераторов, в том числе для set



