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
