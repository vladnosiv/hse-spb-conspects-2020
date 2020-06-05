## Билет 39
Автор: Кирилл Бриллиантов

### ```enable_if```

Помимо того, чтобы вырезать перегрузки по корректности какого-то выражения, можно вырезать еще и по корректности условия. Для этого существует структурка ```enable_if```

Возможная реализация enalble_if

```cpp
template<bool B, class T = void>
struct enable_if {};
 
template<class T>
struct enable_if<true, T> { using type = T; };

template<bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type; // чтобы в коде не лезть внутрь enable_if
```

То есть если условие ложно, то выберется основная версия ```enable_if``` (имеется в виду не специализация). А в такой версии нет псевдонима type, получается, что если ```enable_if_t<B, T>``` полезет внутрь ```enable_if``` , то он не найдет член type, поэтому будет substitution failure.

### Удаление перегрузок методов внутри класса

Иногда хочется удалить некоторые перегрузки изначально нешаблонных методов внутри шаблонного класса. Наивное решение ждет неудача

```cpp
template <typename T>
struct MemberDetecter {
  std::enable_if_t< /*some condition*/, bool> test() {
    return true;
  }
  bool test() const /* const, чтобы не было ambigous */{ 
    return false;
  }
};
```
В такой реализации, если условие неверно, произойдет ошибка компиляции, так как SFINAE не работает для нешаблонных объектов, а в данном случае  ```test``` - нешаблонная фукнция. Чтобы это пофиксить

```cpp
template <typename T>
struct MemberDetecter {
  template <typename U = T> // Теперь test - шаблонная
  std::enable_if_t< /*some condition*/, bool> test() {
    return true;
  }
  bool test() const /* const, чтобы не было ambigous */{ 
    return false;
  }
};
```

Теперь при вызове test ничего не поменялось, так как есть параметр по умолчанию, но устранилась ошибка компилции, так как врубилось SFINAE

### Упрощенный member detection

Две предыдущие идея можно объединить для упрощенного member detection

```cpp
// детектим метод int getValue() const;
template <typename T>
struct MemberDetecter {
  bool test (...) const { return false; } // 1
  
  template <typename U = T>
  std::enable_if_t<
    std::is_same_v<int, decltype(std::declval<const T&>()).getValue()>,
    bool
  > test (std::nullptr_t) cosnt { // 2
     return true;
  }
};
```
Внутри ```enable_if_t``` происходит многоуровневая проверка
- Вначале провеятся можно ли вызывать getValue() от const& на объект типа T. Если нельзя то данная перегрузка вырежется по SFINAE
- Далее происходит проверка того, что возвращаемые типы сошлись при помощи ```is_same_v```

Также тут используется ```...``` для того чтобы сделать перегрузку с приоритетом меньше, чем у 2. Помимо точек надо, чтобы 2 что-то принимало (например std::nullptr_t). Возможно, здесь можно обойтись без этого, но такой трюко удобен, когда кол-во перегрузок больше двух.
