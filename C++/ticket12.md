## Билет 12
Автор: Кирилл Бриллинатов

## Строгая гарантия в конструкторах (ctor) и деструкторах (dtor)
# ctor
В **ctor** обеспечить строгую гарантию исключений очень просто, так как до конструктора у объекта 
не было состояния - его не существовало. Тогда если в конструкторе вылетело исключение, 
то после этого состояние не поменяется. Единственное, надо следить за тем, чтобы не утекали 
ресурсы - этого проще всего добится использованием RAII-оберток. 
```cpp
struct HuffmanArchiver {
  HuffamnArchiver(const std::string& input_filename, const std::string& output_filename) 
      : input_filename_{input_filename}, output_filename_{output_filename} {}
      // если std::bad_alloc вылетело при копировании в output_filename_, то input_filename_ корректно удалится
private:
  std::string input_fileaname_, output_filename_;
};
```

# dtor 
В **dtor** обеспечение строгой гарантии должно быть выполнено, так как неявно все **dtor** помечены **nothrow**
