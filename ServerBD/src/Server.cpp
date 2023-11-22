#include "Server.h"
#include <pthread.h>
#include <string>

void* GetClientRequest(void* clientInfo) {
    ServerBD* serverBD=((struct paramCS*)clientInfo)->server;
    int connectFD=*(((struct paramCS*)clientInfo)->clientSocket);
    delete ((struct paramCS*)clientInfo)->clientSocket;
    delete (struct paramCS*)clientInfo;

    //Считываем входящее сообщение
    int buffer=MAXPACKETSIZE;
    size_t bytes_read;
    char buff[buffer];
    memset(buff, '\0', sizeof(char)*buffer);
    std::string clientRequest;
    while((bytes_read = recv(connectFD, &buff, sizeof(char)*buffer,0))>0){
	clientRequest +=buff;
	memset(buff, '\0', sizeof(char)*buffer);
	if (bytes_read < buffer) break;
    }
    CheckError(bytes_read, "read from socket");

    //Фильтруем по виду запроса входящеее сообщение
    if(bytes_read!=0)  FilterRequest(clientRequest, connectFD, serverBD);
    return NULL;
    }

void PushQueueClients(std::string clientAnswer, ServerBD* serverBD){
    while(serverBD->Clients.size()>=QUEUE_SIZE)
	pthread_cond_wait(&serverBD->FullPool, &serverBD->PoolMutex);
    // push the new work in the queue
    serverBD->Clients.push(clientAnswer);
    pthread_cond_signal(&serverBD->PoolCond);                   
    pthread_mutex_unlock(&serverBD->PoolMutex);
}

void FilterRequest(std::string request, int connectFD, ServerBD* serverBD){
    //считываем вид запроса
    std::string buff="";
    int i=0;
    while(request[i]!=' '){
	if (request[i]=='\0') break;
	buff+=request[i];
	i++;
    }
    i++;
    
    TypeRequest typeRequest;
    if(buff=="") {
	typeRequest=UNKNOWN;
    }
    else {
	typeRequest=(TypeRequest)stoi(buff);
    }


    switch (typeRequest) {
    //Запрос на регистрацию
    case REG:
	PushQueueClients(std::to_string(connectFD)+" "+request, serverBD);
	break;
	
    //Запрос на авторизацию пользователя
    case AUT:
	PushQueueClients(std::to_string(connectFD)+" "+request, serverBD);
        //Вставляем пару сокет-сессия для ожидания ответа (пройдена авторизация или нет). От этого зависит дальнейшая работа с клиентом
	pthread_mutex_lock(&serverBD->SessionMutex);
	serverBD->SessionMap.insert(std::pair<int, TypeSession>(connectFD, WAIT));
	pthread_mutex_unlock(&serverBD->SessionMutex);

        //Ожидание конца обработки запроса
	while(serverBD->SessionMap[connectFD]==WAIT){}

	if(serverBD->SessionMap[connectFD]==FAILED) break;

        //Если пользователь авторизировался и его данные подтвердились, то начинается цикл работы с клиентом
        //Считывание запросов клиента и добавление в очередь
	while(true){
	    int buffer=MAXPACKETSIZE;
	    size_t bytes_read;
	    char buff[buffer];
	    memset(buff, '\0', sizeof(char)*buffer);
	    std::string clientRequest;
	    while((bytes_read = recv(connectFD, &buff, sizeof(char)*buffer,0))>0){
		clientRequest +=buff;
		memset(buff, '\0', sizeof(char)*buffer);
		if (bytes_read < buffer) break;
	    }
	    CheckError(bytes_read, "read from socket");
	    if(bytes_read==0){
		std::cout<<"The client has completed the session.\n"<<std::endl;
		break;
	    }
	    PushQueueClients(std::to_string(connectFD)+" "+clientRequest, serverBD);
	}
	close(connectFD);
	serverBD->SocketMap.erase(serverBD->SocketMap.find(connectFD));
	serverBD->SessionMap.erase(serverBD->SessionMap.find(connectFD));
	break;
    default:
	std::cout<<"Unknown command!"<<std::endl;
	break;
    }
}

void* WorkerThread(void* server){
    ServerBD* serverBD=(ServerBD*)server;
    while(1){
        // Первое закрытие
	pthread_mutex_lock(&serverBD->PoolMutex);
        // Если очередь пуста, дожидаемся условия
	while(serverBD->Clients.empty())
	    pthread_cond_wait(&serverBD->PoolCond, &serverBD->PoolMutex);

        // Удаляем запрос из очереди
	std::string clientRequest = serverBD->Clients.front();
	serverBD->Clients.pop();

        // Печатаем подтверждающее сообщение
	std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: Received task"<< std::endl;

        // Потоки ожидания сигнала в полном пуле
	pthread_cond_signal(&serverBD->FullPool);
	pthread_mutex_unlock(&serverBD->PoolMutex);
	//Отправляем ответ после обработки запроса
	SendAnswer(HandleRequest(clientRequest,serverBD), serverBD);
    }
    return NULL;
}

//Возвращает ответ сервера, обрабатывает запрос
std::string HandleRequest(std::string clientRequest, ServerBD *serverBD){  
    std::string buff="";
    int i=0;
    TypeRequest typeRequest;
    int connectFD;
  
    while(clientRequest[i]!=' '){
	if (clientRequest[i]=='\0') break;
	buff+=clientRequest[i];
	i++;
    }
    i++;
    
    connectFD=stoi(buff);
    std::string clientAnswer=buff+" ";
    buff="";
  

    while(clientRequest[i]!=' '){
	if (clientRequest[i]=='\0') break;
	buff+=clientRequest[i];
	i++;
    }
    i++;
    if(buff=="") {
	typeRequest=UNKNOWN;
    }
    else {
	typeRequest=(TypeRequest)stoi(buff);
    }

    //Определяем вид запроса и заполняем массив входящими данными, которые были вместе с запросом
    switch(typeRequest){
    case REG:{
	const int N=7;
	std::array<std::string, N> data{""};
	for(int k=0;k<N;k++){
	    for(;clientRequest[i]!=' ';i++){
		if(clientRequest[i]=='\0') break;
		data[k]+=clientRequest[i];
	    }
	    i++;
	}

        //Проверяем существование логина в БД
	if((serverBD->BDPSQL).CheckExistsLogin(data[0])==false){
            //Используем интерфейс работы с БД, регистрируем пользователя
	    (serverBD->BDPSQL).RegistrationUser(data[0], stoi(data[1]), data[2], data[3], data[4], data[5], data[6]);
	    clientAnswer+=std::to_string(REGSUC); 
	}
	else {
            //в случае существовании такого же логина, отправляем код ошибки на клиент
	    clientAnswer+=std::to_string(REGFAL);
	}
	std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: About registraion answer "<< data[0]<< std::endl;
	break;
    }
    case AUT:{
	const int N=2;
	std::array<std::string, N> data{""};
	for(int k=0;k<N;k++){
	    for(;clientRequest[i]!=' ';i++){
		if(clientRequest[i]=='\0') break;
		data[k]+=clientRequest[i];
	    }
	    i++;
	}

        //Проверяем сущестовавание логина, идентичность пароля
	if((serverBD->BDPSQL).CheckExistsLogin(data[0])==true){
	    if((serverBD->BDPSQL).AuthorizationUser(data[0])==stoi(data[1])){
                //Если все пройдено, то создаем ответ с данными о пользователе и в дальнейшем отправляем клиенту
		clientAnswer+=std::to_string(AUTSUC)+" "+(serverBD->BDPSQL).GetUserAccount(data[0])+" "+(serverBD->BDPSQL).GetUserBankAccount(data[0])+"\0";
		std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: About authorization answer "<< data[0]<< std::endl;
                //Подтверждаем авторизацию пользователя
		pthread_mutex_lock(&serverBD->SessionMutex);
		serverBD->SessionMap[connectFD]=CONFIRMED;
		pthread_mutex_unlock(&serverBD->SessionMutex);
		break;
	    }
	}

        //В случае ошибки, создаем ответ код-ошибки
	clientAnswer+=std::to_string(AUTFAL);
	std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: About authorization answer "<< data[0]<< std::endl;

        //Сессия не удалась
	pthread_mutex_lock(&serverBD->SessionMutex);
	serverBD->SessionMap[connectFD]=FAILED;
	pthread_mutex_unlock(&serverBD->SessionMutex);
	break;
    }
    case DEPOSIT:{
	const int N=2;
	std::array<std::string, N> data{""};
	for(int k=0;k<N;k++){
	    for(;clientRequest[i]!=' ';i++){
		if(clientRequest[i]=='\0') break;
		data[k]+=clientRequest[i];
	    }
	    i++;
	}

        //Выполняем запрос пополнение, благодаря флагу true идет добавление к счету
	(serverBD->BDPSQL).SetUserCount(data[0], data[1], true);
	clientAnswer+=std::to_string(DEPSUC)+" "+(serverBD->BDPSQL).GetUserCount(data[0])+'\0';
	std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: About deposit answer "<< data[0]<< std::endl;
	break;
    }
    case WITHDROW:{
	const int N=2;
	std::array<std::string, N> data{""};
	for(int k=0;k<N;k++){
	    for(;clientRequest[i]!=' ';i++){
		if(clientRequest[i]=='\0') break;
		data[k]+=clientRequest[i];
	    }
	    i++;
	}
	
        //Выполняем запрос снятие, благодаря флагу false идет снятие со счета, также проверка счета на возможность перевода
	if(stoi(data[1])<=stoi((serverBD->BDPSQL).GetUserCount(data[0]))){
	    (serverBD->BDPSQL).SetUserCount(data[0], data[1], false);
	    clientAnswer+=std::to_string(WITSUC)+" "+(serverBD->BDPSQL).GetUserCount(data[0])+'\0';
	    std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: About withdrow answer "<< data[0]<<std::endl;
	    break;
	}
	else {
	    clientAnswer+=std::to_string(WITFAL)+" "+(serverBD->BDPSQL).GetUserCount(data[0])+'\0';
	    std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: About withdrow answer "<< data[0]<< std::endl;
	}
    }
    case TRANSFER:{
	const int N=3;
	std::array<std::string, N> data{""};
	for(int k=0;k<N;k++){
	    for(;clientRequest[i]!=' ';i++){
		if(clientRequest[i]=='\0') break;
		data[k]+=clientRequest[i];
	    }
	    i++;
	}
        //Выполняем запрос перевод на другой счет с помощью логина. Проверка на сущестовование и вохможность перевода
	if((serverBD->BDPSQL).CheckExistsLogin(data[2])==true){
	    if(stoi(data[1])<=stoi((serverBD->BDPSQL).GetUserCount(data[0]))){
		(serverBD->BDPSQL).Transaction(data[0], data[1], data[2]);    
		clientAnswer+=std::to_string(TRANSUC)+" "+(serverBD->BDPSQL).GetUserCount(data[0])+'\0';
		std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: About transfer answer "<< data[0]<< std::endl;
		break;
	    }
	}
	clientAnswer+=std::to_string(TRANFAL)+" "+(serverBD->BDPSQL).GetUserCount(data[0])+'\0';
	std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: About transfer answer "<< data[0]<< std::endl;
	break;
    }
    case UPDATEDATA:{
	const int N=1;
	std::array<std::string, 1> data{""};
	for(int k=0;k<N;k++){
	    for(;clientRequest[i]!=' ';i++){
		if(clientRequest[i]=='\0') break;
		data[k]+=clientRequest[i];
	    }
	    i++;
	}
	clientAnswer+=std::to_string(UPDSUC)+" "+(serverBD->BDPSQL).GetUserAccount(data[0])+" "+(serverBD->BDPSQL).GetUserBankAccount(data[0])+"\0";
	std::cout << "[Thread: "<< std::hash<std::thread::id>()(std::this_thread::get_id()) << "]: About profile answer "<< data[0]<< std::endl;
	break;
    }
    case UNKNOWN:{
	std::cout<<"UNKNOWN command"<<std::endl;
    }
    }
    return clientAnswer;
}

void* SendAnswer(std::string clientRequest, ServerBD* serverBD){
    std::cout<<clientRequest<<std::endl;
    std::string buff="";
    int i=0;
    while (clientRequest[i]!=' '){
	buff+=clientRequest[i];
	i++;
    }
    int connectFD = stoi(buff);
    i++;
  
    std::string typework="";
    typework+=clientRequest[i];

    buff="";
    while(clientRequest[i]!='\0'){
	buff+=clientRequest[i];
	i++;
    }
    buff+='\0';

    pthread_mutex_lock(&serverBD->SocketMutex);
    if((serverBD->SocketMap).find(connectFD)==(serverBD->SocketMap).end()){
	CheckError(-1, "mutex for socket unavailable");
    }
    pthread_mutex_unlock(&serverBD->SocketMutex);
    pthread_mutex_lock(&serverBD->SocketMap[connectFD]);
    write(connectFD, buff.c_str(),strlen(buff.c_str()));
    pthread_mutex_unlock(&serverBD->SocketMap[connectFD]);

    //Если ответ тип работы 2, это значит авторизация, сокет не закрываем
    if(stoi(typework)==2){
	return NULL;
    }

    //В другом случае, сокет закрывается и удаляется из map сокетов
    else{
	close(connectFD);
	pthread_mutex_lock(&serverBD->SocketMutex);
	serverBD->SocketMap.erase(serverBD->SocketMap.find(connectFD));
	pthread_mutex_unlock(&serverBD->SocketMutex);
	return NULL;
    }
}

//инициализируем мьютексы
ServerBD::ServerBD(){
    PoolMutex=PTHREAD_MUTEX_INITIALIZER;
    PoolCond=PTHREAD_COND_INITIALIZER;
    FullPool=PTHREAD_COND_INITIALIZER;
    SocketMutex = PTHREAD_MUTEX_INITIALIZER;
    }


// устанавливаем сервер с портом
void ServerBD::Setup(int port){
    sockfd=CheckError(socket(AF_INET, SOCK_STREAM, 0),"creating socket");
    BindPort(sockfd, port);
    CheckError(listen (sockfd, BACKLOG), "listen port");
    std::cout<<"Server was successfully initialised. Listening on port "<<port<<std::endl;
}

//Получение информации от клиента
void ServerBD::Receive(){
    while(true){

        //Принимаем соединение клиента
	socklen_t sosize =sizeof(ClientAddress);
	CheckError(newsockfd = accept(sockfd, (struct sockaddr*) &ClientAddress, &sosize),"accept connection");
	std::cout<<"Accepted connection from "<<inet_ntoa(ClientAddress.sin_addr)<<"\n";

        //Заполняем структуру для передачи в поток чтения входящего сообщения с клиента
	struct paramCS* clientInfo=new struct paramCS;
	clientInfo->clientSocket=new int;
	*(clientInfo->clientSocket)=newsockfd;
        //указатель для доступа к членам и методам сервера
	clientInfo->server=this;
	pthread_mutex_lock(&SocketMutex);
        // Cоздаем новый мьютекс, если он отсутствует
	if(SocketMap.find(newsockfd)==SocketMap.end()){
	    pthread_mutex_t new_socket_mutex = PTHREAD_MUTEX_INITIALIZER;
	    SocketMap.insert(std::pair<int, pthread_mutex_t>(newsockfd, new_socket_mutex));
	}
	pthread_mutex_unlock(&SocketMutex);
	
        //Создаем поток для чтения входящего сообщения с параметрами структуры
	pthread_create(&ServerThread, NULL, &GetClientRequest, (void*)clientInfo );
	pthread_detach(ServerThread);
    }
}

void ServerBD::SetupInPSQL() { BDPSQL.Setup(); }

int ServerBD::BindPort(int socketFD, int port){
    memset(&ServerAddress, 0, sizeof(ServerAddress));
    ServerAddress.sin_family=AF_INET;
    ServerAddress.sin_addr.s_addr=htonl(INADDR_ANY);
    ServerAddress.sin_port=htons(port);
    // binding socket socketFD to address 
    return CheckError(bind(socketFD, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)), "binding socket");
}

void ServerBD::CreateWorkerThreads(int threadPoolSize){
    pthread_t thread_pool[threadPoolSize];
    for(int i=0;i< threadPoolSize;i++){

        // Функции WorkerThread циклически ждут работы
        pthread_create(&thread_pool[i], NULL, WorkerThread, this);
    }
}


int CheckError(int code, const char * message){
  
    // Если возвращаемое значение -1 
    if (code ==-1){

        // Печатаем ошибку, определенное в коде сообщение и errno w
        std::cerr << "Error Encountered with: " << message << std::endl;
        std::cerr << "Errno: " << errno << std::endl;
        _exit(1);
    }

    // В другом случае возвращаем значение вызова 
    return code;
}
