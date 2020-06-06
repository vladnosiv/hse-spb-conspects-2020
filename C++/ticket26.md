## Билет 26
Автор: Саврасов Михаил

Этот билет содержит: 

* Ассоциативные контейнеры в STL
* Их свойства, требования к хранящимся классам
* Особенности map и unordered_map


>Тут спрашиваются отличия, например, stdio.h и cstdio. В C используем первый, в C++ — второй, он не загрязняет глобальное пространство имён, а кладёт всё в std::

>Автор: Сурков Пётр, билет 24

### `set`, `multiset`, `map`, `multimap`

Элементы хранятся в порядке `<`.

Все операции с элементами по умолчанию производятся за `O(log n)`.

При добавлении итераторы и ссылки никогда не инвалидируются.
При удалении инвалидируется ссылка и итератор на удаляемый элемент. (При вызове метода `extract()` инвалидируется только итератор).

Подробнее про инвалидацию итераторов и ссылок можно почитать [тут](https://stackoverflow.com/questions/6438086/iterator-invalidation-rules "stackoverflow").


Удаление по итератору работает амортизированно за `O(1)`.
Удаление по значению за `O(log n)`.

Для вставки можно использовать подсказки (метод `emplace_hint`). Если подсказка верна, то добавление работает амортизированно за `O(1)`.

#### Компараторы

По умолчанию `set` сравнивает на `less<T>(a, b)`.

```c++
template<typename T, typename Cmp = std::less<T>>
class set {
    Cmp cmp;
    set() : cmp() {}
    set(Cmp cmp_) : cmp(cmp_) {}
    ....
        if (cmp(node->key, key_to_find)) { .... } else { .... }
    ....
};
template<typename T>
class less {
     bool operator()(const T &a, const T &b) const/*!!!*/ {
        return a < b;  // Должен ли a быть строго раньше b?
    }
}
```

#### Функторы 

Можно использовать для сравнения свои функторы. У них должен быть определен `bool operator()` с определенной сигнатурой:

```c++
bool operator()(T a, T b) const;
``` 

Пример кода:

```c++
// объявление
struct CompareK {
    int k;
    CompareK(int k_) : k(k_) {}
    bool operator()(int a, int b) const {
        return a * k < b * k;
    }
};

// использование
set<int, CompareK> values(CompareK(-1));
values.insert(1);
values.insert(2);
// {2, 1}
```


#### Лямбды и функции

При их использовании всегда нужно явно указывать тип.

Например:

```c++
bool compare(int a, int b) { return a > b; }
....
set<int, bool(*)(int, int)> values(compare);
values.insert(2);
values.insert(1);
// {2, 1}
```

Или

```c++
bool compare(int a, int b) { return a > b; }
....
set<int, decltype(&compare)> values(compare);
values.insert(2);
values.insert(1);
// {2, 1}
```

И для лямбд:


```c++
auto compare = [](int a, int b) { return a > b; };
set<int, decltype(compare)> values(compare);
values.insert(2);
values.insert(1);
```


### `operator[]` для `map` и `multimap`

При обращении по несуществующему ключу, создается элемент с таким ключом и значением, созданным конструктором по умолчанию.

Например:

```c++
map<int, int> values{{1, 100}, {2, 200}};


assert(values[3] == 0); // Не упадет
``` 


Есть метод `at()`, который бросает исколючение в случае, если элемента с таким ключом нет.

### `unordered_set`, `unordered_multiset`, `unordered_map`, `unordered_multimap`

Элементы лежат в порядке, который зависит от хеш-функции. 
Все операции с элементами по умолчанию работают за `O(1)` амортизированно.


Из-за возможного rehashing, при добавлении элементов, могут испортиться все итераторы.

При удалении инвалидируется ссылка и итератор на удаляемый элемент. (При вызове метода `extract()` инвалидируется только итератор).

Подробнее про инвалидацию итераторов и ссылок можно почитать [тут](https://stackoverflow.com/questions/6438086/iterator-invalidation-rules "stackoverflow").

У них тоже есть `emplace_hint()`.

#### Свои хеш-функции

Объект должен уметь сравниваться на `==`

Хеш-функция должна возвращать `size_t`

Например:

```c++
struct point {
    int x = 0, y = 0;
    point() {}
    point(int x_, int y_) : x(x_), y(y_) {}
    bool operator==(const point &other) const {
        return x == other.x && y == other.y;
    }
};
struct PointHasher {
    size_t operator()(const point &p) const {
        return std::hash<int>()(p.x) * 239017 + std::hash<int>()(p.y);
    }
};
....
unordered_set<point, PointHasher/*, std::equal_to<point>*/> points;
points.emplace(10, 20);  // points.size() == 1
points.emplace(20, 10);  // points.size() == 2
points.emplace(10, 20);  // points.size() == 2
```

### `operator[]` для `map` и `multimap`

При обращении по несуществующему ключу, создается элемент с таким ключом и значением, созданным конструктором по умолчанию.

Например:

```c++
unordered_map<int, int> values{{1, 100}, {2, 200}};


assert(values[3] == 0); // Не упадет
``` 


Есть метод `at()`, который бросает исколючение в случае, если элемента с таким ключом нет.
