#pragma once
#ifndef SIGNINPSQL_H
#define SIGNINPSQL_H
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <iostream>
#include <string>
#include <array>
#include <termios.h>
#include <unistd.h>   
#include <cstring>
#include <string>

#define SIZE 100

#define UNUSED(x) (void)(x)

//Класс для взаимодействия с PostgreSQL, с определенными функциями для работы с БД
class SignInPSQL {
private:
    PGconn* conn;
    PGresult* res;
    
    //методы для ввода пароля
    int getch();
    std::string GetPassworddb();
    std::string GetUserdb();
    std::string SignIn();
    
public:
    //Выделение и освобождение памяти с помощью конструктора и деструктора
    SignInPSQL();
    ~SignInPSQL();
    
    //Метод устанавливает соединения с БД
    void Setup();
    void Terminate(int code);
    void ClearRes();
    
    //Запрос-авторизация
    int AuthorizationUser(std::string login);
    
    //Проверка существования логина в БД
    bool CheckExistsLogin(std::string login);
    
    //Обработчик замечаний
    void ProcessNotice(void *arg, const char *message);
    
    //Запрос-регистрация
    void RegistrationUser(std::string login, int pswd, std::string surname, std::string name, std::string patronymic, std::string dataofb, std::string keyword);
    
    //Запрос-получение данных с аккаунта
    std::string GetUserAccount(std::string login);
    
    //Запрос-получение данных счета
    std::string GetUserBankAccount(std::string login);

    //Запрос-установка суммы на счете
    void SetUserCount(std::string login, std::string sum, bool op);

    //Запрос-получение суммы на счете
    std::string GetUserCount(std::string login);

    //Запрос-транзация, перевод на другой счет
    void Transaction(std::string sender, std::string sum, std::string receiver);
};

#endif
