#pragma once
#ifndef BD_CLIENT_H
#define BD_CLIENT_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <signal.h>
#include <cstdio>

int CheckError (int i, const char *message);

//Структура для хранения данных о пользователе
struct ClientInfo{
    std::string login;
    std::string surname;
    std::string name;
    std::string patronymic;
    std::string dataofbirthday;
    int numberacc;
    int count;
};

class ClientBD{
private:
    int Sock;
    std::string Address;
    int Port;
    struct sockaddr_in Server;
    bool Send(std::string data);
    struct ClientInfo InfoClient;
    void SetInfoClient(std::string info);
    void SetCount(std::string count);
public:
    ClientBD();
    bool Setup(std::string address, int port);
    int GetAnswer();
    void SetLogin(std::string login);
    std::string GetLogin();
    int GetCount();
    void ShowInfoClient();
    void Registration();
    void Authorization();
    void Deposit();
    void Withdraw();
    void Transfer();
    void UpdateData();
    void exit();
};

#endif
