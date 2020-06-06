## Билет 34
Автор: Александр Морозов

## Специализации шаблонов
### Синтаксис для `stack<bool>`
```
template<typename T> struct stack { ... }; // Общий случай
template<>
struct stack<bool> { // Для конкретных параметров.
    // Абсолютно независимая реализация.
};
```

### Частичная специализация
```
template<typename T> struct unique_ptr {
    T *data;
    ~unique_ptr() { delete data; }
};
template<typename T> struct unique_ptr<T[]> {
    T *data;
    ~unique_ptr() { delete[] data; }
};
```

Работает pattern matching.
Можно зачем-нибудь специализировать как `template<typename A, typename B> struct unique_ptr<map<A, B>> { ... }`.

### Немного type traits
Так можно делать вычисления над типами и значениями на этапе компиляции:
```
template<typename T> struct is_reference { constexpr static bool value = false; };
template<typename T> struct is_reference<T&> { constexpr static bool value = true; };
template<typename T> struct is_reference<T&&> { constexpr static bool value = true; };
//
printf("%d\n", is_reference<int&>::value);
```

Это один из способов реализовывать `<type_traits>` (второй будет в 4 модуле, называется SFINAE).

Ещё пример:
```
template<typename U, typename V> struct is_same { constexpr static bool value = false; };
template<typename T> struct is_same<T, T> { constexpr static bool value = true; };
```

А ещё это можно использовать в своих библиотеках.
Например, для сериализации: вы пишете общий случай `template<typename T> struct serializer {};`,
а дальше для каждого типа реализуете `struct serializer { static std::string serialize(const T&); static T deserialize(std::string); }`

## Специализация функций
Для функций есть только полная специализация:
```
template<> void swap(Foo &a, Foo &b) { ... }
```
Частичной нет.
Вместо неё предполагается использовать перегрузки, если очень надо:
```
template<typename T> void swap(vector<T> &a, vector<T> &b) { ... }
```
По техническим причинам это не всегда хорошо, правда, но таков факт жизни.

## Прокси-объекты
### Для `vector<bool>` 
Если мы делаем битовую упаковку в `vector<bool>`, то мы больше не можем вернуть `bool&`.

Но надо вернуть что-то такое, чтобы `a[i] = false;` работало.
В C++03 и раньше хорошей идеей были прокси-объекты:

```
template<>
struct vector<bool> {
private:
    struct vector_bool_proxy {
        vector_bool_proxy operator=(bool value) {
            // Установка бита.
        }
        operator bool() {
            return // Чтение бита.
        }
        // А ещё надо operator</operator== для bool по-хорошему, брр...
    };
public:
    vector_bool_proxy operator[](std::size_t i) { return vector_bool_proxy(....); }
    bool operator[](std::size_t i) const { return ....; }
};
```

### Проблемы
[Ссылка с таймкодом](https://youtu.be/YJ2TKViIkSU?t=4460)
Начиная с C++11, как только передали в `auto`: `auto x = v[10];`, а потом захотели написать `auto y = x`, вызвалось присваивание прокси-объектов, а хотелось присваивание булов. Можно добавить костыль в виде rvalue-ref-qualifier'а в оператор присваивания, но проблемы останутся. 

Если были forwarding-ссылки: можно использовать хитрость и писать `auto&&` (называется universal reference или forwarding reference, и является ссылкой, если передать lvalue, или значением, если передать rvalue). 

Если сделать `for (auto i : v)`, то из-за того, что `i` - прокси, изменение `i` повлечет изменение элемента `v`. В `vector<int>` не повлечет. Если же написать `for (auto&& i : v)`, то изменяться содержимое вектора будет вне зависимости от того, лежит внутри `bool` или `int`.

То есть, если мы хотим сохранить ссылку на элемент вектора с произвольным типом, то можно написать `auto&& elem = v[0]`, и это будет работать в том числе для `vector<bool>`.
Кроме того, можно написать `vector<T>::reference_type elem = v[0]`.



## Tag dispatching
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
  Поэтому можно использовать `std::iterator_traits<It>::iterator_category` вместо `typename It::iterator_category()`.
	Также есть std::begin(arr), std::end(arr), которые умеют получать начало и конец как у стандартного контейнера, так и у массива. 

