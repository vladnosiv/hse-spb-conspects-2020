## Билет 28
Автор: Саврасов Михаил

Этот билет содержит: 

* Итераторы (константные и неконстантные)
* Использование итераторов


>Тут спрашиваются отличия, например, stdio.h и cstdio. В C используем первый, в C++ — второй, он не загрязняет глобальное пространство имён, а кладёт всё в std::

>Автор: Сурков Пётр, билет 24


## Итераторы и операции с ними

У каждого контейнера также есть __итератор__ `vector<T>::iterator` (похож на `T*`) и `vector<T>::const_iterator` (похож на `const T*`).
Это некоторый небольшой объект, который умеет ходить по контейнеру (копируется и присваивается без исключений).
В общем случае это что угодно (например, у `map` надо обходить двоичное сбалансированное дерево).

* Итератор можно получить у любого контейнера:
  * Картинка с `begin`/`end`
  * `.begin()`/`.end()` — возвращает __итераторы__ на начало и элемент-после-конца `[begin; end)`. Типа указателей, но более общее.
  * `.cbegin()`/`.cend()` — аналогично, но итераторы константные (`const T*`).
* Итераторы можно разыменовывать:
  * `*i` и `i->foo()` у любого итератора: mutable/constant.
  * У mutable даже `*i = foo`.
* У любого контейнера есть специтератор, указывающий на элемент "после последнего".
  Его нельзя разыменовывать никогда и никак, это UB.
* Пара итераторов задаёт полуинтервал range.
  Если задали кривой, то UB.
* Все операции с итераторами работают за константу (амортизированно) или не работают вообще.


## Инвалидация итераторов

Инвалидация может произойти после каких-либо изменений контейнера. Например, при добавлении в `unordered_set`, может произойти rehash, в результате чего инвалидируются все итераторы.


## Иерархия итераторов
* Input/output iterator: можно сравнивать `==` и `!=`, можно `++r` и `r++`.
  При этом нельзя предполагать, что если `a == b`, то `++a == ++b`.
  Ещё есть алгоритмы `std::prev` и `std::next`.
  Считывать можно много раз, а вот записывать через `*r = foo` только один раз.
  Нельзя писать в input, нельзя читать из output.
  Пример: `istream_iterator`/`ostream_iterator`/`back_insert_iterator`/`front_insert_iterator`/`insert_iterator` для вставки в контейнер.
  Следствие: single pass algorithm.
* Forward iterator.
  Гарантируется, что итераторы ведут себя нормально: если `a == b`, то `++a == ++b`
  и если поменять копию итератора, то сам итератор не поменяется.
  Следствие: бывают multi pass algorithm.
* Bidirectional iterator. Добавили ещё и `--`.
* Random access iterator.
  Добавили `+=`, `+`, `-`, `<` и `>`, как в арифметике указателей.
  Вычитание даёт `difference_type`, но так можно только для связанных итераторов.

## Алгоритмы с итераторами

### Немодифицирующие
* `bool all_of(first, last, pred)` и друзья
* `Pred for_each(first, last, pred)`, `for_each_n` (только первые `n` элементов)
* `difference_type count(first, last, value)`, `count_if(...., pred)`
* `find(first, last, pred)`, `find_if`, `find_if_not`



### Сравнения
* `min`/`max`/`minmax` из двух значений или из фигурных скобок. Осторожно: возвращают ссылку:
  https://www.youtube.com/watch?v=s9vBk5CxFyY
  ```
  const Foo &x = foo();  // Работает.
  const Foo &x = std::min(foo1(), foo2());  // Не работает, потому что lifetime extension только с самым внешним значением.
  ```
* `clamp(value, lo, hi)` — обрезаем значение. Возвращает ссылку.
* `equal(f1, l1, f2[, pred])`, ещё есть лексикографическое `<`
* `std::pair<> mismatch(first1, last1, first2)`, первое место с отличием
* `min_element`, `max_element`, `minmax_element` — первый минимум/максимум. Возвращают итератор.
  Пример, когда вектор непустой: `vec.erase(min_element(vec.begin(), vec.end()));` 

Из numeric library:

* `accumulate(first, last, init = 0)` для `operator+`

### Модифицирующие
Осторожно с пересечениями входа и выхода!

* `copy(f, l, out)`/`move`{,`_backward`}
  Если куда-то копия, то конечный итератор не даём, он выводится.
  Этот и дальше стараются работать даже для `ForwardIt`, по этому поводу могут активно возвращать указатель на "конец".
* `fill(f, l, value)`, `fill_n(first, count, value)`
* `generate(f, l, g)`, `OutputIt generate_n(first, count, gen)`, есть частные случаи в Numerics library вроде 
* `OutIt transform(f, l, out, unary_op)` как map из Haskell
* `sort` (необязательно стабильный, гарантированный NlogN)/`stable_sort`/`nth_element`,
  есть частичные сортировки...

Из numeric library:

* `partial_sum`, `inclusive_scan`, `exclusive_scan`

### Сужающие
* `{remove}{,_copy}{,_if}`: `(first, last, [out], value|pred)`, возвращает итератор на конец.
* `{replace}{,_copy}{,_if}`: `(first, last, [out], old_value|pred, new_value)`, возвращает итератор на конец.
* `unique{,_copy}(f, l, [pred])`, удаляет __подряд идущие__ одинаковые.

Пример:
```
vector<int> v = { 1, 3, 2, 3 };
v.erase(remove_if(v.begin(), v.end(), [](int x) { return x % 2 == 0; }), v.end());
v.erase(unique(v.begin(), v.end()), v.end());
```

### Binary search
* `bool binary_search(f, l, [pred])`
* `pair<> equal_range(f, l, val, [pred])`
* `lower_bound` и `upper_bound` как крайние случаи.

### Прочие операции
* `swap(a, b)` для значений
* `swap_ranges(f1, l1, f2) --> l2`
* `iter_swap(a, b)` для обмена значений по итераторам
* `reverse{_,copy}` — угадайте, какие параметры?
* `rotate{_,copy}` (сделай элемент первым)
* `partition(f, l, pred) --> mid` за линию, есть модификации и проверка `is_partitioned`
* Merge отсортированных с дополнительной памятью и без
* Операции с отсортированными множествами в векторах: объединение, симметрическая разность, проверка принадлежности...
* Операции с кучей на массиве
* Операции с перестановками на массиве (проверить, получить следующую)

### Свои алгоритмы
#### `value_type`
* Хотим написать `min` по range:
  ```
  template<typename It>
  auto min_element(It first, It last) {
      auto found = *first;
      for (++first; first != last; ++last) {
          if (*first < found) {
              found = *first;
          }
      }
      return found
  }
  ```
* До C++11 нельзя было написать `auto`.
  Поэтому у итераторов и контейнеров есть `value_type`:
  ```
  typename It::value_type min_element(....) { .... }
  ```

#### Tag dispatching
* Хотим сделать свой `std::next`:
  ```
  template<typename It>
  It my_next(/*std::random_access_iterator_tag, */It x, typename It::difference_type count) {
      return x += count;
  }
  ```
* Тут работает только для RandomAccess. Для ForwardIterator:
  ```
  template<typename It>
  It my_next(/*std::forward_iterator_tag, */It x, typename It::difference_type count) {
      for (; count; --count)
          ++x;
      return x;
  }
  ```
* А теперь фокус (tag dispatching): есть специальный тип `It::iterator_category`.
  Там бывает `std::random_access_iterator_tag` (пустая структура), наследуется
  от `std::bidirectional_iterator_tag`, наследуется от `std::forward_iterator_tag`.
  Так можно надо на этапе компиляции поставить `if`.
  Можно добавить фиктивный параметр и сделать перегрузки,
  а общую реализацию такую:
  ```
  template<typename It>
  It my_next(It x, typename It::difference_type count) {
      return my_next(typename It::iterator_category(), x, count);
  }
  ```
* Это не сработает с `T*`, хотя они тоже указатели.
  Поэтому `std::iterator_traits<It>::iterator_category`.
  Костыль вроде `std::begin`/`std::end`


## Range-based for
* Бывает `std::begin`/`std::end` и range-based-for:
  * https://cppinsights.io/
  * `std::begin`/`std::end` вызывают внутренние `begin`/`end` и перегружены для обычных массивов.
  * Тонкость: в `for (int &x : foo().bar()) { ... }` временное значение `foo()` не живёт, а `bar()` живёт.
  * Range-based for — синтаксический сахар для https://en.cppreference.com/w/cpp/language/range-for :
    ```
    for (DECL : EXPR) BODY
    // ~~~~
    {
        auto range = EXPR;
        auto begin = std::begin(range);
        auto end = std::begin(end);
        for (auto it = begin; it != end; ++it) {
            DECL = *it;
            BODY
        }
    }
    ```
* Не стоит менять то, по чему итерируемся

## `reverse_interator`

Как обычный итератор, только двигающийся в обратную сторону. Могут все то же, что и обычные итераторы для этого контейнера. 

Можно получить с помощью функции `make_reverse_iterator()`. Обычный итератор можно получить, вызвав метод `base()`.

