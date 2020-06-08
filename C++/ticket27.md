## Билет 27
Автор: Кирилл Бриллиантов
>### Конвенции именований стандартной библиотеки, отличия stdio.h и cstdio
>* В стандартной библиотеке C++ есть кусок под названием STL (standard template library),
>  это всякие вектора и алгоритмы.
>  Ввод-вывод (`iostream`) туда не входит.
>  Но иногда STL путается с "стандартная библиотека C++".
>* Конвенция в стандартной библиотеке C++: заголовок пишется без `.h`
>  и обычно добавляет имена только в `namespace std`
>  * Добавили что-то своё в `namespace std` (за парой исключений) — UB
>* В C++ есть два вида заголовков из Си:
>  * `<stdio.h>` — всё подключил в глобальный namespace, опционально в `std`
>  * `<cstdio>` всё подключил в `std`, опционально в глобальный 
### Адаптеры

Основное свойство - построены на основе других контейнеров.

#### stack и queue

Дек, у которого убрали половину методов

```cpp
template<typename T, typename Container = std::deque<T>>
struct stack {
    Container c;
    ... // методы связанные с итераторами
    void push(const T &value) { c.push_back(value); }
    void push(T &&value) { c.push_back(std::move(value)); }
    void pop() { c.pop_back(); }
    T& top() { return c.back(); }
    const T& top() const { return c.back(); }
};
template<typename T, typename Container = std::deque<T>>
struct queue {
    Container c;
    ....
    void push(const T &value) { c.push_front(value); }
    void push(T &&value) { c.push_front(std::move(value)); }
    void pop() { c.pop_front(); }
    T& front() { return c.front(); }
    const T& front() const { return c.front(); }
};
```
Следствием того, что они хранят контейнер является то, что инвалидация ссылок и итераторов
происходит тогда же, когда и у **Conatiner** и гарантии соответствующих методов предоставляются такие же. 

Вместо deque можно подставить что-то у чего есть соответствующие методы.

#### priority_queue

Адаптор, представляющий собой двоичную кучу. Помимо контейнера использует компаратор и библиотечные функции связанные с кучей
```cpp
template<typename T, typename Container = std::vector<T>, typename Compare = std::less<T>>
struct priority_queue {
    Container c;
    ....
    const T& top() { return c[0]; }
    void push(const T &value) {
        c.push_back(value);
        std::push_heap(....);
    }
    void push(T &&value);
    void pop() {
        std::pop_heap(....);
        c.pop_back();
    }
};
```
- Итераторов нет. Можно получить доступ только к голове куче. 
- Гарантии методов такие же, как и у контейнера, так как методы что-то_heap не кидают исключения (они просто переставляют элементы).
- Методы что-то_heap можно вызывать самостоятельно. 
- Работает быстрее, чем ```set<T>```

### bitset
Контейнер, хранящий последовательно ``` N ``` (известно на этапе компиляции) битов.
- конвертируется в строки для вывода на экран
- гарантии строгие (в основном может вылететь `std::out_of_range`, `std::bad_alloc` не вылетает, так как память выделена на этапе компиляции)
- операции с индивидуальными битами происходят медленнее, чем в ```vector<char>```
- массовые же наоборот во много раз быстрее (Егор говорит в 10 раз)
- полезно для ускорения динамического программирования
```cpp
bitset<10> bs(   "0000010011");
bs.set(2);     // 0000010111
assert(bs.test(2));
assert(bs[2]);
bs.reset(2);   // 0000010011
bs.flip();     // 1111101100
assert(bs.count() == 7);
bs ^= bitset<10>("1100000001");
               // 0011101101
assert(bs.count() == 6);
```

