## Прочие мелочи, не в билетах
Авторы: ..., Пётр Сурков

### If-with-init statement
Хотим завести переменную, чтобы она была доступна только в условиях и телах `if`, `else if`, `else`.

Очевидное решение:
```c++
map<int, string> m = ...;
{
    typename map<int, string>::iterator it = m.find(10);
    if (it != m.end()) cout << it->second << '\n';
    else               cout << "Not found\n";
}  // После этой скобки `it` недоступен
```

Решение строго лучше, пишем объявление прямо в `if`:
```c++
map<int, string> m = ...;
if (auto it = m.find(10); it != s.end()) {
    cout << it->second << '\n';
} else {
    cout << "Not found\n";  // it тут доступен.
}  // После этой скобки `it` недоступен.
```

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

RTTI: не написано, пусть хоть ссылка на презу будет: https://github.com/yeputons/hse-2019-cpp/blob/master/26-200421/presentation.md#311-rtti-%D0%B8-dynamic_cast
### TODO: RTTI
#### TODO: "полиморфный класс"
#### TODO: typeid
#### TODO: const std::type_info& и std::type_index
#### TODO: Mangling имён и type_info::name()
