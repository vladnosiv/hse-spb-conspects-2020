## Билет 38
Автор: Кирилл Карнаухов

### SFINAE (начало)
Расшифровка: Substitution Failure Is Not An Error.

Означает, что если при подстановке типовых параметров в функцию/специализацию/конструктор что-то не удалось вывести через `::`, то просто забиваем на конкретную перегрузку функции/специализацию/конструктор и пробуем дальше.

Мотивация: хочется, чтобы работали всякие `is_move_constructible` — они смотрят только на объявление, не определение.

Пример:
```cpp
template<typename T>
void duplicate_element(T &container, typename T::iterator iter) {
    container.insert(iter, *iter);
}

template<typename T>
void duplicate_element(T *array, T *element) {
   assert(array != element);
   *(element - 1) = *element;
}

int main() {
    std::vector a{1, 2, 3};
    duplicate_element(a, a.begin() + 1);
    int b[] = {1, 2, 3};
    duplicate_element(b, b + 1);  // Нет ошибки, когда пробуем первую перегрузку: не бывает int[]::iterator, но это не ошибка компиляции. SFINAE.
}
```

Отличается от hard compilation error тем, что если произошла ошибка при подстановке, то это ошибки компиляции не будет (будет только тогда, когда ни одна специализация/перегрузка не подошла). 

Важна следующая тонкость: SFINAE работае только на один уровень. То есть, если сделать свою структуру, а внутри `using`, то он должен быть нормальным. Пример:
```cpp
struct BotvaHolder {
    using botva = int;
};

//****рабочая версия****
template<typename T> using GetBotva = typename T::botva;

template<typename T>
void foo(T, GetBotva<T>) {  // Просто псевдоним, так можно, проблемы возникают как бы "тут".
    std::cout << "1\n";
}
//****конец рабочей версии****
//****нерабочая версия****
template<typename T>
struct GetBotva {
    using type = typename T::botva;  // hard compilation error.
};

template<typename T>
void foo(T, typename GetBotva<T>::type) {  // Ошибка компиляции, потому что проблема возникла уже внутри GetBotva.
    std::cout << "1\n";
}
//****конец нерабочей версии****

template<typename T>
void foo(T, std::nullptr_t) {
    std::cout << "2\n";
}

int main() {
    foo(BotvaHolder(), 10);  // 1
    foo(BotvaHolder(), nullptr);  // 2
    // foo(10, 10);  // CE
    foo(10, nullptr); // 2
}
```

### SFINAE (`auto` + `decltype`)
Первый способ вырезать перегрузку (и т.д.). Пример:
```cpp
template<typename T>
auto add(const T &a, const T &b) -> decltype(a + b) {
    return a + b;
}

auto add(...) { 
    return 239;
}

int main() {
    std::cout << add(10, 20) << "\n";
    std::cout << add(nullptr, nullptr) << "\n";
}
```
Пояснения. Если `a` и `b` можно сложить, то `a + b` корректно определено, то есть можно выполнить `decltype(a + b)` и получить правильный тип. Тогда эта перегрузка не будет вырезана. А так как аргумент `...` имеет приоритет ниже, то будет использована именно первая перегрузка. Таким образом, если два объекта можно сложить, то это произойдет.

Иначе эта перегрузка будет вырезана по SFINAE, так как произошла ошибка подстановки. В таком случае будет вызвана вторая функция, так как осталась только она.

Если хотим проверить, можно ли сложить два числа, то можно сделать так:
```cpp
template<typename T>
auto isAddable(const T &a, const T &b) -> decltype(a + b, true) {
    return true;
}

auto isAddable(...) { 
    return false;
}
```
Если `operator,` не перегружен для `T`, то это сработает следующим образом. Выполнится выражение `a + b` и проверится, что это возможно. И если это было успешно, то возвращаемое значение будет `decltype(true) = bool`, так как `operator,` возвращает то, что справа. Иначе будет вызвана вторая перегрузка.

Заключение. Метод хороший, если хочется проверить лишь корректность выражения. Не работает для конструкторов, потому что нет возвращаемого значения.

### SFINAE (Фиктивный шаблонный параметр-тип, `decltype` и `nullptr`)
Второй способ: сделать фиктивный шаблонный параметр-тип. Пример:
```cpp
struct Foo {
    template<typename T, typename = decltype(std::declval<T>().foo())>
    Foo(T) {
        std::cout << "1\n";
    }

    template<typename T>
    explicit Foo(T) {
        std::cout << "2\n";
    }
};

struct Bar {
    void foo() {}
};

int main() {
    Foo f1 = Bar();  // 1, потому что 2 недоступно из-за explicit
    Foo f2(10);      // 2, потому что 1 недоступно из-за SFINAE
}
```
Пояснение. Пытаемся подставить `int`. Но возникает проблема, что у `int` нет `.foo()`, поэтому по SFINAE конструктор вырезается.

К сожалению, для `Bar` оба конструктора одинаково подходят, поэтому, как вариант, необходим `explicit`. 

Также можно изменить `typename = ` на следующую конструкцию:
```cpp
struct Foo {
    template<typename T, std::void_t<decltype(std::declval<T>().foo())>* = nullptr>
    Foo(T) {
        std::cout << "1\n";
    }

    template<typename T>
    explicit Foo(T) {
        std::cout << "2\n";
    }
};
```
Здесь мы скастовали наше старое выражение к `void`, взяли указатель и сделали `nullptr` стандартным значением. То есть это вспомогательный фиктивный шаблонный параметр-значение. 

Примечание: происходит конвертация к `void`, потому что нельзя взять указатель на произвольный тип (например, указатель на `int&` сделать нельзя).

Мотивация предыдущего изменения в том, что мы не можем два раза использовать что-то вроде `template<typename T, typename = decltype(std::declval<T>().foo())>`, так как в таком случае компилятор не различит перегрузки. Но если использовать `void* = nullptr`, то все будет хорошо.

Но есть еще один подводный камень. Он состоит в `std::void_t`. Компилятор просто соптимизирует и скажет, что это всегда `void`, и таким образом может забить на выражение внутри. Поэтому можно написать свой `void_t`:
```cpp
template<typename...>
struct my_void { using type = void; };

template<typename... Ts>
using my_void_t = typename my_void<Ts...>::type;
```

### SFINAE (Вспомогательный `void_t` в специализациях шаблонных классов)
Третий способ. Пример:
```cpp
template<typename T, typename = void>
struct MyFormatHelper {
    void func1() {}
};

template<typename T>
struct MyFormatHelper<T, decltype(static_cast<void>(std::declval<T>().foo()))> {
    void func2() {}
};

struct Foo {
    void foo() {}
};

int main() {
    MyFormatHelper<int> f1;  // 1
    f1.func1();
    MyFormatHelper<Foo> f2;  // 2
    f2.func2();
}
```
Здесь все происходит аналогично предыдущим пунктам. Но мы изначально указали фиктивный шаблонный аргумент, чтобы потом использовать его для вырезания по SFINAE. Чтобы можно было создавать `FormatHelper<T>`, не указывая второй параметр (а он не нужен пользователю), сделали `void` значением по умолчанию. 

Примечание: поиск конкретного метода или чего-то такого называется member detection idiom.
