#pragma once
#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <map>
#include <array>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <queue>
#include <thread>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "SignInPSQL.h"

#define MAXPACKETSIZE 4096
#define BACKLOG 10
#define QUEUE_SIZE 10

class ServerBD;
struct paramCS;

//Функция-оболочка для обработки ошибок
int CheckError(int code, const char *message);
// Специальная рабочая функция, рабочий поток
// Рабочий поток обрабатывает очередь, выполняет запрос и отправляет его клиенту
void* WorkerThread(void* server);
//Функция для чтения входящего запроса с клиента
void *GetClientRequest(void *clientInfo);
//Функция для фильтрации запроса
void FilterRequest(std::string request, int connectFD, ServerBD *serverBD);
//Функция для обработки запроса в рабочем потоке
std::string HandleRequest(std::string request, ServerBD *serverBD);
//Функция для отправки ответа клиенту в рабочем потоке
void *SendAnswer(std::string clientRequest, ServerBD *server);
// Функция для добавления в главную очередь запросов клиентов 
void PushQueueClients(std::string clientAnswer, ServerBD *serverBD);

//Перечисление видов запроса
enum TypeRequest {
    REG = 10,
    AUT = 20,
    DEPOSIT = 21,
    WITHDROW = 22,
    TRANSFER = 23,
    UPDATEDATA = 24,
    UNKNOWN = 0
};
//Перечисление видов ответа
enum TypeAnswer {
    REGSUC = 101,
    REGFAL = 102,
    AUTSUC = 201,
    AUTFAL = 202,
    DEPSUC = 211,
    WITSUC = 221,
    WITFAL = 222,
    TRANSUC = 231,
    TRANFAL = 232,
    UPDSUC = 241
};

//Перечисление видов сессии: ожидание, подтверждение и провал
enum TypeSession{
    WAIT,
    CONFIRMED,
    FAILED
};

// Структура для передачи параметров в поток чтения/обработки запросов
struct paramCS{
    int* clientSocket;
    ServerBD* server;
};

// Основной класс для работы приложения. Устанавливает сервер. Подключается к
// базе данных PostgreSQL. Создает рабочие потоки и поток для обработки входящих соединений
// 
class ServerBD {
private:
    //Оболочка для bind
    int BindPort(int socketfd, int port);
public:
    //Сокеты 
    int sockfd;
    int newsockfd;
    struct sockaddr_in ServerAddress;
    struct sockaddr_in ClientAddress;
    
    //TID для создания потока обработки соединения
    pthread_t ServerThread;
    
    //Главная очередь запросов клиентов
    std::queue<std::string> Clients;
    
    //Мьютексы для безопасной работы с очередью
    pthread_mutex_t PoolMutex;
    pthread_cond_t PoolCond, FullPool;

    //Объект для подлючения к базе данных PostgreSQL и работы с его интерфейсом
    SignInPSQL BDPSQL;

    //map сокетов для безопасной работы с сокетами
    std::map<int, pthread_mutex_t> SocketMap;
    pthread_mutex_t SocketMutex;

    //map сессии сокетов для работы потоков
    std::map<int, TypeSession> SessionMap;
    pthread_mutex_t SessionMutex;


    //Интерфейс для работы с классом

    //Конструктор класса
    ServerBD();
    
    //Метод для установки сервера
    void Setup(int port);
    
    //Метод для установки соединения с базой данных
    void SetupInPSQL();
    
    //Главный поток, принимающий соединения от клиентов
    void Receive();
    
    //Метод для создания рабочих потоков по специальной рабочей функции
    void CreateWorkerThreads(int threadPoolSize);
};

#endif
