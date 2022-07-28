<p align="center"><img src="https://sun9-63.userapi.com/impg/xzj5XrUs1h_YT-k9LoUQiO_SioNJW0ywRl7F3Q/-kDWnqONQuw.jpg?size=200x200&quality=96&sign=21e1809165d00028002d70664d8ec9ee"/></p>

## FileD
Работа с файлами-дубликатами произвольных типов (текст, видео, аудио, etc). Позиционируется как небольшая утилита для «домашнего» использования, примеры из личного опыта: 
1. Поиск повторяющихся скриптов/текстур игры/модификации;
2. Очистка изображений-дубликатов из архива (+ переименование остальных картинок по захардкоренной маске).

## Сборка
Тестовая сборка осуществлялась на дистрибутиве GNU/Linux при помощи компилятора __g++__.
```bash
g++ Source.cpp -std=c++20
```
Поскольку программа не содержит не совместимых с иными платформами компонентов, подключить и собрать файл в той же __Microsoft Visual Studio__ не составит никаких проблем.

## Использование
Существует поддержка следующих флагов:
1. __-h__ (__--help__) - вызов сообщения-подсказки.
2. __-p__ (__--path__) - абсолютный путь до директории. __Обязательный параметр.__
3. __-r__ (__--rename__) - переименование оригинальных файлов по пока что захардкоренной маске.
4. __-d__ (__--duplicates__) - автоматическое удаление файлов-дубликатов. Без флага сформированный список путей попросту выводится в консоль.
