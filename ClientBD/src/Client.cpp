#include "Client.h"

ClientBD::ClientBD() {
    Sock = -1;
    Port = 0;
    Address = "";  
}

bool ClientBD::Setup (std::string address, int port){
    this->Address=address;
    this->Port=port;
    CheckError(Sock = socket(AF_INET, SOCK_STREAM, 0), "socket");
    Server.sin_addr.s_addr= inet_addr( Address.c_str() );
    Server.sin_family = AF_INET;
    Server.sin_port = htons (Port);
    CheckError(connect(Sock, (struct sockaddr *) &Server, sizeof(Server)), "connect");
    std::cout << "Установлено соединение с " << inet_ntoa(Server.sin_addr) << " на порте: " << Port << "\n\n";
    return true;
}

bool ClientBD::Send(std::string data){
    return CheckError(send(Sock, data.c_str(), strlen(data.c_str()),0), "send path");
}

void ClientBD::SetLogin(std::string login) { InfoClient.login = login; }

void ClientBD::Registration(){
    std::string password, cword;
    std::cout<<"\nВведите логин:";
    std::cin>>InfoClient.login;
    std::cout<<"Введите пароль:";
    std::cin>>password;
    std::cout<<"Введите ваши данные профиля:"<<std::endl;
    std::cout<<"Фамилия:";
    std::cin>>InfoClient.surname;
    std::cout<<"Имя:";
    std::cin>>InfoClient.name;
    std::cout<<"Отчество:";
    std::cin>>InfoClient.patronymic;
    std::cout<<"Формат 2000-01-26\nДата рождения:";
    std::cin>>InfoClient.dataofbirthday;
    std::cout<<"Кодовое слово восстановления:";
    std::cin>>cword;
    Send("10 "+InfoClient.login+" "+password+" "+InfoClient.surname+" "+InfoClient.name+" "+InfoClient.patronymic+" "+InfoClient.dataofbirthday+" "+cword+"\0");
}

void ClientBD::Authorization(){
    std::string password;
    std::cout<<"Введите пароль:";
    std::cin>>password;
    Send("20 "+InfoClient.login+" "+password+"\0");
}

void ClientBD::Deposit(){
    int sum;
    std::cout<<"Введите сумму пополнения:";
    std::cin>>sum;
    Send("21 "+InfoClient.login+" "+std::to_string(sum)+"\0");
}

void ClientBD::Withdraw(){
    int sum;
    std::cout<<"Введите сумму вывода:";
    std::cin>>sum;
    Send("22 "+InfoClient.login+" "+std::to_string(sum)+"\0");
}

void ClientBD::Transfer(){
    int sum;
    std::string receiver;
    std::cout<<"Введите логин получателя:";
    std::cin>>receiver;
    std::cout<<"Введите сумму перевода:";
    std::cin>>sum;
    Send("23 "+InfoClient.login+" "+std::to_string(sum)+" "+receiver+"\0");
}

void ClientBD::UpdateData(){
    Send("24 "+InfoClient.login+"\0");
}

int ClientBD::GetAnswer(){

    int buffer=4096;
    size_t bytes_read;
    char buff[buffer];
    memset(buff, '\0', sizeof(char)*buffer);

    std::string clientAnswer;

    while((bytes_read = recv(Sock, &buff, sizeof(char)*buffer,0))>0){
	clientAnswer +=buff;
	memset(buff, '\0', sizeof(char)*buffer);
	if (bytes_read < buffer) break;
    }
    CheckError(bytes_read, "read from socket");
    if(bytes_read==0) return 0;
    std::string code="";
    int i=0;
    
    while(i<3){
	code+=clientAnswer[i];
	i++;
    }
    i++;
    std::string clientInfo="";
    while(clientAnswer[i]!='\0'){
	clientInfo+=clientAnswer[i];
	i++;
    }
  
    int codes=stoi(code);
    switch (codes){
    case 201:
	SetInfoClient(clientInfo);
	return codes;
    case 211:
	SetCount(clientInfo);
	return codes;
    case 221:
	SetCount(clientInfo);
	return codes;
    case 231:
	SetCount(clientInfo);
	return codes;
    case 241:
	SetInfoClient(clientInfo);
	return codes;
    default:
	return codes;
    } 
}

std::string ClientBD::GetLogin(){
    return InfoClient.login;
}

void ClientBD::SetInfoClient(std::string info){
    std::string buff;
    int j=0;
    for(int i=0;i<6;i++){
	for(;info[j]!=' ';j++){
	    if(info[j]=='\0') break;
	    buff+=info[j];    
	}
	if(i==0)InfoClient.surname=buff;
	if(i==1)InfoClient.name=buff;
	if(i==2)InfoClient.patronymic=buff;
	if(i==3)InfoClient.dataofbirthday=buff;
	if(i==4)InfoClient.numberacc=stoi(buff);
	if(i==5)SetCount(buff);
	buff="";
	j++;
    }
}

void ClientBD::SetCount(std::string count) {
    InfoClient.count=stoi(count);
}

int ClientBD::GetCount(){
    return InfoClient.count;
}

void ClientBD::ShowInfoClient(){
    std::cout << "Профиль "<<InfoClient.login<<":";
    std::cout << "\nФамилия: "<<InfoClient.surname;
    std::cout << "\nИмя: "<<InfoClient.name;
    std::cout << "\nОтчество: "<<InfoClient.patronymic;
    std::cout << "\nДата рождения: "<<InfoClient.dataofbirthday;
    std::cout << "\nНомер счета: "<<InfoClient.numberacc;
    std::cout << "\nСчет: "<<GetCount()<<" рублей\n\n";
}

void ClientBD::exit(){
    close (Sock);
}

int CheckError(int i, const char * message) {
    // Если возвращаемое значение -1 
    if (i ==-1){
        // Если возвращаемое значение -1 
	std::cerr << "Error Encountered with: " << message << std::endl;
	std::cerr << "Errno: " << errno << std::endl;
	_exit(1);
    }
    
    // В другом случае возвращаем значение вызова 
    return i;;
}
