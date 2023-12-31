# BankBD
Multithreaded TCP Client/Server with database PostgreSQL, realization in C++

Цель проекта: Реализация взаимодействия TCP клиент/серверной архитектуры в виде многопоточного банковского приложения с хранением информации в PostgreSQL.

Выбор совершенный в сторону такого проекта сделан для ярко выраженной простоты. В том числе, выделения важных деталей.
- Во-первых, клиент/серверная архитектура дает возможность гибкой работы сервера с клиентами, что очень важно для создания как новых версий клиентов, так и поддержке старой, также работает в обратную сторону.
- Во-вторых, банковское приложение - это безопасность и надежность. Важно проследить каждой взаимодействие клиента и сервера, сделать такие операции безопасными и без утечек данных. Многопоточность сокращает время взаимодействия, благодаря работе с несколькими клиентами. Работа с mutex and condition дают точную синхронизацию потоков и гарантируют сохранность данных.
- В-третьих, реляционная база данных PostgreSQL в проекте дает возможность хранение структурированной информации, таких как данные клиентов.
Для конструирования базы данных нужен язык SQL. Работа приложения с базой данных PostgreSQL поддерживется библиотекой <libpq>.

Идейно проект задумывался как итог всех моих навыков в программировании. По ходу реализации проекта поступало много новых знаний. Такие как библиотека <libpq> со своим интерфейсом, переменные ожидания и различные тонкости заметные только при тщательном изучении и практической работе. Название банка вымышленно. Конечно, в проекте существуют проблемы, и мне, как программисту, предостоит еще долгая работа в изучении данной сферы деятельности. Прошу вас, в случае нахождения проблемы, писать мне на почту. 
Email: budazhap.radnaev@inbox.ru

## Абстракция проекта
![bankbd](https://github.com/fkhs/BankBD/assets/149998060/af76eed2-4ab1-41f8-b622-a2942f5f0f7e)


## Работа проекта
1) Запускаемый файл сервера подключается к БД PostgreSQL на устройстве, также требуется настройка конфигурации PostgreSQL. Развертывается сервер (ServerBD), настраиваются переменные, создаются рабочие потоки (worker thread) и поток receive, принимающий подключения и создающий соединяющий поток (connection thread). 
2) Клиент подключается к серверу, отправляется запрос-регистрация или запрос-авторизация. Сервер определяет логику работы соединяющего потока. Для определения запросов используется ENUM TypeRequest
    - Запрос-регистрация: TypeRequest-> REG = 10
Соединяющий поток отправляет запрос на регистрацию с данными, которые пришли вместе с запросом. Проверка БД и самих данных. Создается аккаунт с паролем.
    - Запрос-авторизация: TypeRequest-> AUT = 20
Соединящий поток отправляет запрос на проверку пароля, и, при соотвествии пароля, переходит в состояние работы с клиентом. Клиент отправляет различные запросы: пополнение, перевод или снятие.
3) При завершении работы, сокет клиента закрывается и освобождаются ресурсы, занимаемые потоком.

## Сборка и запуск проекта
- Для сборки был использован CMake, для клиента и сервера есть файлы CMakeLists.txt
- Установить БД PostgreSQL, настроить ее и загрузить файл BankBD.sql
- После сборки, сначала запускаем сервер, а после подключаемся с клиента.

Для удобства есть скрипты для оболочки bash в каждой папке.

## Демонстрация работы
### Клиент BankBD
![clientreg](https://github.com/fkhs/BankBD/assets/149998060/8ca76ad8-966e-4612-8f4b-c7c211a4cc7a)

![clientaut](https://github.com/fkhs/BankBD/assets/149998060/6916b11d-32ad-47ac-910e-fe0a502a5583)

![clientwork](https://github.com/fkhs/BankBD/assets/149998060/5c7d8429-b339-4a26-a0f3-c844467cd381)

### Сервер ServerBD
![serverwork](https://github.com/fkhs/BankBD/assets/149998060/99fc8dec-c069-456f-a640-c76cd8993db2)

![serverlog](https://github.com/fkhs/BankBD/assets/149998060/b9a3b3f5-c64c-4cb7-9647-20d305cd62e3)

### База данных PostgreSQL
![sqlbankbd](https://github.com/fkhs/BankBD/assets/149998060/829e2d05-0300-4f2c-866c-26d12a9b1cc6)

![sqlbankbddata](https://github.com/fkhs/BankBD/assets/149998060/affeb23d-c431-471d-a472-fc0bed866d6d)

![sqlbankcount](https://github.com/fkhs/BankBD/assets/149998060/5b8cf82f-3a44-44be-9165-d04fe6310304)
