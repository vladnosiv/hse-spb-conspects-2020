## Билет 02
Автор: Носивской Влад

За поиск багов спасибо Данилу Бубнову.

* Зачем: во избежание коллизий. Например, пишем клиент-сервер, класс User есть и у клиента, и у сервера. Для этого заведем namespace client и namespace server. Теперь обращение client::User и server::User соответственно.

* Еще немного про мотивацию:
    * Можно использовать вложенные классы:

        ```C++
        class Database {
        public:
            class User { ... };
        };
        ```

        ```C++
        class Application {
        public:
            class User { ... };
            static Database connectToDb() { ... }
        };
        ```

        Здесь Database::User и Application::User - разные классы. Есть минусы: нужно писать много static-ов и в разных файлах использовать разные классы, либо заводить один God object, который содержит все функции и переменные со всех файлов. 
        
        Зачем здесь написан static для функции connectToDb? static функции как члены класса - это функции, которые не требуют наличия экземпляра класса для вызова (в данном случае мы не обязаны иметь экземпляр класса Application для вызова функции connectToDb).

    * А можно использовать namespaces:
        
        ```C++
        namespace database {
        class Database {
        public:
            class User { ... };
        };
        } // namespace database

        namespace application {
        class User { ... };
        database::Database connectToDb() { ... }
        } namespace application
        ```
* Немного про стиль при использовании namespace:
    * Обычно в одном файле один namespace. Внутри пишется без отступов (см. пример выше)
    * Имя в snake_case
    * namespace можно переоткрывать (в отличие от классов), в том числе в разных файлах. При этом содержимое конкатенируется. 
    * Если пишете библиотеку - хороший тон обернуть ее в namespace, а дальше творить в ней что угодно.
    * Заглянуть внутрь это ::, как у классов.

* Вложенные пространства имен.
    ```C++
    namespace database {
        namespace internal {
            void foo();
            void bar();
            struct Foo {};
        } // namespace internal
        struct User {};
    } // namespace database
    ```

    Начиная с С++11 можно писать так:

    ```C++
    namespace database::internal {
        void foo();
        void bar();
    } // namespace database::internal
    ```

    Чтобы обратиться к глобальному пространству имен, обратитесь будто к безымянному пространству:

    ```C++
    int foo;
    namespace bar {
        int foo;
        void some() {
            ::foo = 0; // Изменилось значение переменной, которая объявлена в глобальном пространстве имен.
        }
    } 
    ```

* Static и анонимные пространства имен.
    * Internal linkage позволяет использовать объект/функцию/класс только в той единице трансляции, в которой они объявлены и определены, что позволяет избежать коллизий.
        
        file1.cpp

        ```C++
        static int var = 82; // Передаю привет Гимназии №82 города Краснодара
        int foo() { return var; }
        ```

        file2.cpp

        ```C++
        static int var = 239;
        int bar() { return var; }
        ```

        main.cpp
        ```C++
        #include <iostream>

        int foo();
        int bar();

        int main() {
            std::cout << foo() << std::endl; // 82
            std::cout << bar() << std::endl; // 239
        }
        ```

    * Выше был показан сишный подход, в плюсах решили, что для internal linkage будут использовать анонимные пространства имен. Также анонимные пространства имен позволяют делать internal linkage для типов (классов, структур), что не позволяет static
        
        header.cpp
        ```C++
        namespace {

        int var = 82;

        class A{ int x { 0 }; }

        void f() {}

        }
        ```

        В плюсах лучше всегда использовать анонимные пространства имен.

* Про typedef и using, а главное - про отличие между ними.

    typedef пришел к нам из Си и означает он синоним имени. using же протаскивает имя.

    Например, можно сказать typedef int score_t; чтобы использовать score_t в коде, это будет псевдоним для int.

    * Вне контекста пространств имен typedef проигрывает using-у, так как ему очень плохо дается работа с шаблонами. Пример:

        ```C++
        template<typename T>
        using myAllocList = std::list<T, myAlloc<T>>;

        myAllocList<Object> ml;
        ```

        С typedef-ом написать подобный код будет больно и сложно (но все же возможно, пусть и использование усложнится).

        ```C++
        template<typename T>
        struct MyAllocList {
            typedef std::list<T, MyAlloc<T>> type;
            
        };

        MyAllocList<Widget>::type lw;
        ```

    * В контексте пространств имен using позволяет протащить имя в точку.

        ```C++
        using database::Database;
        ```

    * Еще можно протаскивать целиком все имена из пространства имен:

        ```C++
        using namespace database;
        ```

        Здесь нужно быть очень аккуратным и уверенным, что не будет коллизий с именами. Поэтому не стоит писать в коде using namespace std; (привет контестам), ведь мы не знаем все имена из стандартной библиотеки. Например, там лежат функции left и right, а назвать так переменную иногда может хотеться. 

        Стоит отметить, что использование using namespace внутри .h/.hpp файлов идея сомнительная. Пользователь может добавить хедер и надеяться, что коллизий не будет.


