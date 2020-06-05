## Билет 07
Автор: Денис Филиппов

* Синтаксис и пример
	* Появлась возможность отнаследовать одну структуру / класс от нескольких. Пример:

		``` C++
		struct PieceOfArt { std::chrono::time_point date; }
		struct Music : PieceOfArt { ... };
		struct Lyrics : PieceOfArt { ... };
		struct Song : Music, Lyrics {
		Song(...) : Music(...), Lyrics(...), album(...) {}
		std::string album;
		// using Music::date;
		};
		Song s;
		x = s.Music::date;
		y = s.Lyrics::date;
		```

	* Синтаксис как в обычном наследовании, только теперь мы через запятую перечисляем несколько базовых предков.
   
    

