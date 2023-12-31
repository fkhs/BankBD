--Скрипт создания базы данных банка
--Создаем таблицы клиентов customers
DROP TABLE IF EXISTS customers;
CREATE TABLE customers(
    id INT GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, 
    login VARCHAR(20)
);
--создаем таблицу с хэшами пин-кодов и кодовым словом.

DROP TABLE IF EXISTS shadow;
CREATE TABLE shadow(
    id INT,
    pincode INT,
    keyword VARCHAR(20),
    FOREIGN KEY (id) REFERENCES customers(id)
);
--создаем таблицу аккаунтов с информацией о клиенте

DROP TABLE IF EXISTS accounts;
CREATE TABLE accounts(
    id INT,
    surname VARCHAR(100),
    name VARCHAR(100),
    patronymic VARCHAR(100),
    dataofbirth DATE,
    FOREIGN KEY (id) REFERENCES customers(id)
);
--создаем таблицу с информацией о счетах клиентов

DROP TABLE IF EXISTS bank_accounts;
CREATE TABLE bank_accounts(
    id INT,
    numberacc INT,
    countacc INT,
    FOREIGN KEY (id) REFERENCES customers(id)
);
    
DROP SEQUENCE IF EXISTS numberacc_serial;
CREATE SEQUENCE numberacc_serial START 10000001;
     
