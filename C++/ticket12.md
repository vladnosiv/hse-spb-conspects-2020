## Билет 12
Автор: Кирилл Бриллинатов

### Строгая гарантия в конструкторах (ctor) и деструкторах (dtor)
#### ctor
В **ctor** обеспечить строгую гарантию исключений очень просто, так как до конструктора у объекта 
не было состояния - его не существовало. Тогда если в конструкторе вылетело исключение, 
то после этого состояние не поменяется. Единственное, надо следить за тем, чтобы не утекали 
ресурсы - этого проще всего добится использованием RAII-оберток. 
```cpp
struct HuffmanArchiver {
  HuffamnArchiver(const std::string& inputFilename, const std::string& outputFilename) 
      : inputFilename_{inputFilename}, outputFilename_{outputFilename} {}
      // если std::bad_alloc вылетело при копировании в outputFilename_, то inputFilename_ корректно удалится
private:
  std::string inputFileaname_, outputFilename_;
};
```

#### dtor 
В **dtor** обеспечение строгой гарантии должно быть выполнено, так как неявно все **dtor** помечены **noexcept**

### Общий алгоритм для проведения остальных операций

В общем случае, чтобы обеспечить строгую гарантию исключений для метода класса, можно провести необходимую операцию в два шага: 
1. Сделать все "кидающие" операции, не меняя состояния класса
2. Обновить (если надо) состояние класса с помощью "не кидающих" операций (например мувами)

```cpp
struct Matrix {
  void setColNRow(int cols, int rows) { 
    data_.resize(rows);
    for (auto& row : data_)  {
      row.resize(cols);
    }
  }
  // Плохая реализация, так как внутри цикла может выкинутся bad_alloc, а состояние объекта уже изменено

  void setColNRow(int cols, int rows) { 
    std::vector<std::vector<int>> newdata(rows);
    std::copy(data_.begin(), data_.end(), newdata.begin()); // копирование векторов
    for (auto& row : newdata) { 
      row.resize(cols);
    }
    std::swap(data_, newdata);
  }
  // Лучше, так как swap по умолчанию noexcept

private:
  std::vector<std::vector<int>> data_;
};
```
### Copy-swap trick
```cpp
struct DataHolder {
  DataHolder(const DataHolder& other) : size_{other.size_} { 
    try {
      data_ = new char[size_];
      std::memcpy(data_, other.data_, other.size_);
    } catch(... /* можно специфировать */) {
        delete[] data_;
    }
  }
  
  DataHolder& operator=(DataHolder other) { 
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    return *this;
  }
  // При вызове создаетсь копия, того что передали (или мувается, но это видимо здесь неважно), 
  // затем происходит обмен полями с other и так как время жизни other ограничено фигурными 
  // скобаками, то он удалится и вызовется деструктор DataHolder и старый data_ удалится.

private:
  char* data_;
  std::size_t size_;
};
  
```
Из-за такой реализации код сильно упрощается.
### Проблемы с move-семантикой
Проблемы возникают, когда мы работаем с объектами, у которых move оператор присваивания или ctor 
не **noexcept**. Особенно сложно становится, когда мы храним такой объект по значению. Так как в 
таком случае, например, в move операторе приходится явно мувать этот объект. Можно скопировать, 
но у объекта может быть некопируемым (бывает достаточно часто). Получается, что иногда такие проблемы 
не получается решить и лучше для своих структур делать move ctor и оператор **noexcept**
#### Вывод
Пример, когда нельзя решить: пишем контейнер, хотим reserve - выделили новую память, туда
надо, либо мувать, либо копироавть. Move-оператор - не **noexcept**, нет ctor коипрования.

