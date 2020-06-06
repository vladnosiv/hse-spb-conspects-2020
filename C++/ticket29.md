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
Следующая группа - это посчитать количество элементов в диапазоне \[first, last). 
[ссылка](https://en.cppreference.com/w/cpp/algorithm/count)
Есть две разновидности: 
* `difference_type count(first, last, value) // считает элемент, которые равны value(вызывает operator==)` 
* `difference_type count_if(first, last, pred) // считает элемент, которые удовлетворяют предикату` 
`difference_type` -
 то, что вернет разность итераторов у конкретного контейнера. 
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
    int num_items3 = std::count_if(v.begin(), v.end(), [](int i){return i % 3 == 0;});
    std::cout << "number divisible by three: " << num_items3 << '\n';
}
```
Следующая группа алгоритмов - это какие-то алгоритмы для поиска. [ссылка](https://en.cppreference.com/w/cpp/algorithm/find) 
Их тоже 3 вида: 
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
##### Сравнивающие
Тоже немодифицирующие, но есть тонкость, поэтому разберем их отдельно
`min`[ссылка](https://en.cppreference.com/w/cpp/algorithm/min)/`max`
[ссылка](https://en.cppreference.com/w/cpp/algorithm/max)/
`minmax`[ссылка](https://en.cppreference.com/w/cpp/algorithm/minmax) 
* `constexpr const T& min( const T& a, const T& b );`
* `constexpr const T& min( const T& a, const T& b, Compare comp );`
Возвращает ссылку/пару ссылок для `minmax` на минимум/максимум 
Важная тонкость: 
```C++
const Foo &x = foo();  // Работает. Продлили время жизни временного объекта
const Foo &x = std::min(foo1(), foo2());  /* Не работает, потому что lifetime extension 
 только с самым внешним значением. 
A то, что вернулось(аргумент min) - у них время жизни уже не продлится.
```
* ` const T& clamp( const T& v, const T& lo, const T& hi)` — обрезаем значение. Возвращает ссылку. Есть еще версия с компаратором. Нельзя ставить, что `lo > hi`. Алгоритм берет и выставляет элемент между двумя этими границами включительно.  
Возможная реализация для понимания с [cppref](https://en.cppreference.com/w/cpp/algorithm/clamp)
```C++
template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    assert( !(hi < lo) );
    return (v < lo) ? lo : (hi < v) ? hi : v;
}
```
* `equal(f1, l1, f2[, pred])` Проверяет, что на данном предикате(по умолчанию ==) все элементы "равны". При этом мы должны сами проследить, что второй range такой же. Вообще, это достаточно частое явление в std, что конец второго range отсутствует. 
Ещё есть лексикографическое меньше (для него нужен конец второго, потому что сравнению строк это важно). 
Красивый пример проверки на палиндром: 
```C++
#include <algorithm>
#include <iostream>
#include <string>
 
bool is_palindrome(const std::string& s)
{
    return std::equal(s.begin(), s.begin() + s.size()/2, s.rbegin());
}
 
void test(const std::string& s)
{
    std::cout << "\"" << s << "\" "
        << (is_palindrome(s) ? "is" : "is not")
        << " a palindrome\n";
}
 
int main()
{
    test("radar"); //"radar" is a palindrome
    test("hello"); // "hello" is not a palindrome
}
```
* `std::pair<> mismatch(first1, last1, first2)`, первое место с отличием. Возвращает пару итераторов, в которых раличаются 2 range 
```C++
// mismatch algorithm example
#include <iostream>     // std::cout
#include <algorithm>    // std::mismatch
#include <vector>       // std::vector
#include <utility>      // std::pair

bool mypredicate (int i, int j) {
  return (i==j);
}

int main () {
  std::vector<int> myvector;
  for (int i=1; i<6; i++) myvector.push_back (i*10); // myvector: 10 20 30 40 50

  int myints[] = {10,20,80,320,1024};                //   myints: 10 20 80 320 1024

  std::pair<std::vector<int>::iterator,int*> mypair;

  // using default comparison:
  mypair = std::mismatch (myvector.begin(), myvector.end(), myints);
  std::cout << "First mismatching elements: " << *mypair.first;
  std::cout << " and " << *mypair.second << '\n'; // First mismatching elements: 30 and 80

  ++mypair.first; ++mypair.second;

  // using predicate comparison:
  mypair = std::mismatch (mypair.first, myvector.end(), mypair.second, mypredicate);
  std::cout << "Second mismatching elements: " << *mypair.first;
  std::cout << " and " << *mypair.second << '\n'; // Second mismatching elements: 40 and 320
  return 0;
}
```
* `min_element, max_element, minmax_element` — первый минимум/максимум. Возвращают итератор(или пару из итераторов). Пример, когда вектор непустой: `vec.erase(min_element(vec.begin(), vec.end()));` Заметим, что в случае пустого вектора - плохо, так как `min_element(vec.begin(), vec.end())` отработает корректно и вернет итератор на `vec.end()`, но вот извлечь у нас уже не получится  
* Из numeric library:
`accumulate(first, last, init = 0)` для operator+ - возвращает сумму всех элементов, можем задать начальное значение и, например, сконкатенировать все строки в диапазоне
`T accumulate( InputIt first, InputIt last, T init, BinaryOperation op );` - можно задавать свой бинарный оператор. 
C C++20 в обоих случаях будет добавляться std::move() к тому, что у нас суммируется. Например, сама сумма в конкретный момент времени будет перемещаться, чтобы избежать копирований, то есть, предполагаемый вид: 
```C++
template<class InputIt, class T, class BinaryOperation>
constexpr // since C++20
T accumulate(InputIt first, InputIt last, T init, 
             BinaryOperation op)
{
    for (; first != last; ++first) {
        init = op(std::move(init), *first); // std::move since C++20
    }
    return init;
}
```
##### Модифицирующие
Осторожно с пересечениями входа и выхода! 

* `copy(f, l, out)/move{,_backward}` Если куда-то копия, то конечный итератор не даём, он выводится. 
Первая версия - копирует в первый итератор второго диапазона все элементы, начиная с f. Если добавить `copy_backward`, то он будет копировать, но начиная с l. Это нужно, чтобы, например, если у нас `out` лежит между `f` и `l`, то мы могли бы скопировать. В то же время, это не просто будет зацикливание, а UB, потому что, например, для чего-то компилятор сможет использовать `memset` и это сработают. Есть аналогичная версия с `move`, отличие в том, что теперь происходит не копирование, а перемещение   
Рассмотрим пример:   
```C++
#include <algorithm>
#include <iostream>
#include <vector>
#include <iterator>
#include <numeric>
 
int main()
{
    std::vector<int> from_vector(10);
    std::iota(from_vector.begin(), from_vector.end(), 0); //функция из numericlibrary, 
   //которая заполняет числами от 0 до 9 в нашем случа
 
    std::vector<int> to_vector;
    std::copy(from_vector.begin(), from_vector.end(),
              std::back_inserter(to_vector));
// or, alternatively,
//  std::vector<int> to_vector(from_vector.size());
//  std::copy(from_vector.begin(), from_vector.end(), to_vector.begin());
// either way is equivalent to
//  std::vector<int> to_vector = from_vector;
 
    std::cout << "to_vector contains: ";
 //скопировали все в поток вывода
    std::copy(to_vector.begin(), to_vector.end(),
              std::ostream_iterator<int>(std::cout, " "));
    std::cout << '\n'; //to_vector contains: 0 1 2 3 4 5 6 7 8 9
}
```
* `fill(f, l, value)`, `OutputIt fill_n(first, count, value)`  
Первая версия устанавливает значение `value` для всего диапазона, вторая делает это `count` раз и возврашает итератор на первый элемент который мы еще не изменили.  
* `generate(f, l, g)`, `OutputIt generate_n(first, count, gen)`  
Аналогично предыдущему, но теперь вызывается генератор, ожидаемая сигнатура от него: `Ret fun();`, то есть, чтобы он возвращал результат. Может быть, как функтором, так и функцией  
Пример:  
```C++
#include <algorithm>
#include <iostream>
#include <vector>
 
int f()
{ 
    static int i = 1;
    return i++;
}
 
int main()
{
    std::vector<int> v(5);
    std::generate(v.begin(), v.end(), f);
 
    std::cout << "v: ";
    for (auto iv: v) {
        std::cout << iv << " ";
    }
    std::cout << "\n"; // v: 1 2 3 4 5
 
    // Initialize with default values 0,1,2,3,4 from a lambda function
    // Equivalent to std::iota(v.begin(), v.end(), 0);
    std::generate(v.begin(), v.end(), [n = 0] () mutable { return n++; });
 
    std::cout << "v: ";
    for (auto iv: v) {
        std::cout << iv << " ";
    }
    std::cout << "\n"; // v: 0 1 2 3 4
}
```
* `OutIt transform(f, l, out, unary_op)` [ссылка](https://en.cppreference.com/w/cpp/algorithm/transform) 
Как map из Haskell: трансформирует последовательность и записывает ее в итератор out  
Существует версия для бинарного оператора: 
```C++
template<class InputIt1, class InputIt2, 
         class OutputIt, class BinaryOperation>
OutputIt transform(InputIt1 first1, InputIt1 last1, InputIt2 first2, 
                   OutputIt d_first, BinaryOperation binary_op)
{
    while (first1 != last1) {
        *d_first++ = binary_op(*first1++, *first2++);
    }
    return d_first;
}
//first2 0 - второй входной итератор
``` 
* `sort` (необязательно стабильный, гарантированный NlogN, если долго работает quicksort, то запускается mergesort)/`stable_sort`(может выделять себе память, работает дольше, чаще всего представляет из себя mergesort)/`nth_element` (k-ая порядковая статистика)

Из numeric library:  
* `OutputIt partial_sum( InputIt first, InputIt last, OutputIt d_first );` - считает частичные суммы
Пример: 
```C++
#include <numeric>
#include <vector>
#include <iostream>
#include <iterator>
#include <functional>
 
int main()
{
    std::vector<int> v = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2}; // or std::vector<int>v(10, 2);
 
    std::cout << "The first 10 even numbers are: ";
    std::partial_sum(v.begin(), v.end(), 
                     std::ostream_iterator<int>(std::cout, " "));
    std::cout << '\n'; //The first 10 even numbers are: 2 4 6 8 10 12 14 16 18 20 

    std::partial_sum(v.begin(), v.end(), v.begin(), std::multiplies<int>()); //добавили свой бинарный оператор
    std::cout << "The first 10 powers of 2 are: ";
    for (auto n : v) {
        std::cout << n << " ";
    }
    std::cout << '\n'; // The first 10 powers of 2 are: 2 4 8 16 32 64 128 256 512 1024
}
```
* `inclusive_scan` [ссылка](https://en.cppreference.com/w/cpp/algorithm/inclusive_scan)
* `exclusive_scan`  
Данные две функции похожи на предыдущую, вычисляют какую-то префиксную функцию на range. Разница лишь в том, что в первом случае учитывается последний элемент в текущем range, а во втором - нет 
```C++
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
 
int main()
{
  std::vector data {3, 1, 4, 1, 5, 9, 2, 6};
 
  std::cout << "exclusive sum: ";
  std::exclusive_scan(data.begin(), data.end(),
		      std::ostream_iterator<int>(std::cout, " "),
		      0); // добавили начальное значение: 0 3 4 8 9 14 23 25 
  std::cout << "\ninclusive sum: "; 
  std::inclusive_scan(data.begin(), data.end(),
		      std::ostream_iterator<int>(std::cout, " ")); // 3 4 8 9 14 23 25 31
 
  std::cout << "\nexclusive product: ";  
  std::exclusive_scan(data.begin(), data.end(),
		      std::ostream_iterator<int>(std::cout, " "),
		      1, std::multiplies<>{}); //добавили начальное значение и свою функцию 1 3 3 12 12 60 540 1080 		      
  std::cout << "\ninclusive product: ";
  std::inclusive_scan(data.begin(), data.end(),
		      std::ostream_iterator<int>(std::cout, " "),
		      std::multiplies<>{}); // 3 3 12 12 60 540 1080 6480		      
}
```
#### Erase-remove idiom, встроенный метод для list, своя реализация remove_if
##### Erase-remove idiom
Существуют функции, которые как-то "сужают" наш контейнер, наприме, remove. 
Пусть мы хотим удалить из вектора все `10`, тогда воспользуемся:  
`remove(v.begin(), v.end(), 10)`. Однако, на самом деле он не удаляет элементы, а просто группирует те, которые хочет оставить в начало контейнера. А в конце остается какой-то мусор. Специально для этого такие алгоритмы возвращают итераторы на новый последний элемент. Тогда в нашем случае мы сможем просто сделать `erase(remove(v.begin(), v.end(), 10), v.end())`
Опишем теперь общую схему подобных алгоритмов:  
* `{remove}{,_copy}{,_if}: (first, last, [out], value|pred)`, возвращает итератор на конец.
* `{replace}{,_copy}{,_if}: (first, last, [out], old_value|pred, new_value)`, возвращает итератор на конец.
* `unique{,_copy}(f, l, [pred])`, удаляет подряд идущие одинаковые.  
Опишем пару примеров: 
```C++ 
#include <algorithm>
#include <iterator>
#include <string>
#include <iostream>
int main()
{
    std::string str = "Text with some   spaces";
    std::cout << "before: " << str << "\n"; // before: Text with some   spaces
 
    std::cout << "after:  ";
    std::remove_copy(str.begin(), str.end(),
                     std::ostream_iterator<char>(std::cout), ' ');
    std::cout << '\n'; // after:  Textwithsomespaces

}
```
Здесь у нас просто скопировались все нужные элементы(то есть не пробел)
Примеры на erase-remove idiom 
```C++ 
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
 
int main() 
{
    // a vector containing several duplicate elements
    std::vector<int> v{1,2,1,1,3,3,3,4,5,4};
 
    // remove consecutive (adjacent) duplicates
    auto last = std::unique(v.begin(), v.end());
    // v now holds {1 2 1 3 4 5 4 x x x}, where 'x' is indeterminate
    v.erase(last, v.end()); 
    for (int i : v)
      std::cout << i << " ";
    std::cout << "\n";
 
    // sort followed by unique, to remove all duplicates
    std::sort(v.begin(), v.end()); // {1 1 2 3 4 4 5}
    last = std::unique(v.begin(), v.end());
    // v now holds {1 2 3 4 5 x x}, where 'x' is indeterminate
    v.erase(last, v.end()); 
    for (int i : v)
      std::cout << i << " ";
    std::cout << "\n";
 
}
```
```C++
vector<int> v = { 1, 3, 2, 3 };
v.erase(remove_if(v.begin(), v.end(), [](int x) { return x % 2 == 0; }), v.end());
v.erase(unique(v.begin(), v.end()), v.end());
```
Аккуратнее с вектором при `remove_copy` в него: или сделайте `resize` вектора, а потом в конце `erase` или пушбекайте. 

Поговорим теперь про `list`:  
* Двусвязный список, удаление всегда за линию от количества элементов.
* Нельзя random access, итераторы и указатели не инвалидируются. Можно идти с начала, можно идти с конца за константу.
* Можно перемещать элементы из одного списка в другой:
  splice(pos, x) (если x != this): утащили все элементы из x (это контейнер или итератор из одного элемента) в позицию pos. Итераторы и ссылки тоже переехали. Всё за константу.
* splice(pos, begin, end): перетащили внутри себя элементы за константу на другое место. А если это был другой список, то линия (потому что надо пересчитать size; до C++11 было по-другому(там размер считался за линию, поэтому могло возникать: `for (size_t i = 0; i < l.size(); i++)` - работает за квадрат :) ).
* Есть методы: remove, remove_if, unique, merge, reverse, стабильный sort: аналогично алгоритмам, но не инвалидируют(потому что в связи с особенностью `list` мы можем не переставлять все элементы, потом выкинуть, а можно просто честно переставить указатели. Но так как нам тогда требуется доступ ко внутренностям, то это уже методы `list`). 

#### Двоичный поиск: отличия lower_bound и upper_bound
Требуется, чтобы элементы были отсортированны с точки зрения предиката меньше, или с точки зрения своего собственного предиката, который мы можем передать в качестве доп параметра.   
```C++
bool binary_search( ForwardIt first, ForwardIt last, const T& value );
//проверяет, если value в range
std::pair<ForwardIt,ForwardIt>
    equal_range( ForwardIt first, ForwardIt last,
                 const T& value );
//укажет диапазон, в котором лежит value, соответственно,
 //правая граница будет невключительно(элемент, следующий за value)
```
Выведем теперь `lower_bound` и `upper_bound`  
```C++
lower_bound = equal_range.first //first i : a_i >= v
upper_bound = equal_range.second //first i : a_i > v
```
Соответственно, если все элементы меньше, чем искомый, то вернет просто `end()/last` 
#### Особенности использования двоичного поиска для не-RandomAccess итераторов, в том числе для set
Поскольку у нас не RandomAccess итератор, то мы не можем использовать стандартный бинпоиск - он отработает не за логарифм, а за линейную сложность. (то есть в обычном мы бы позвали  `it += k`, а в нашем случае много раз прибавим 1). Поэтому существуют специальные методы, например, у `set` есть свои методы: `iterator lower_bound (const value_type& val);` 


