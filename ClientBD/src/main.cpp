#include "Client.h"

ClientBD CBD;
//Переменная для хранения статуса программы, работает или нет
bool C_STATUS = true;

void MesSucc() { std::cout << "Операция выполнена успешно." << std::endl; }
void MesFail() { std::cout << "Ошибка: операция не выполнена!" << std::endl; }
//В функция в случае завершения программы
void SigExit (int code)
{
    CBD.exit();
    std::cout<<std::endl;
    exit (code);
}
//Главный цикл программы при авторизации
void Invitation(){
    //Переменная для хранения вводимых команд
    std::string enterUser="";
    std::string commandUser="";
    char commands[10]{'1','2','3','4','5','6','7','8','9'};
    std::cout<<CBD.GetLogin()<<"=> ";
    std::cin>>enterUser;
    commandUser=+enterUser[0];
  
    int i=0;
    while (i<10){
	if(commandUser[0]==commands[i]) break;
	if(i==9) commandUser="0";
	i++;
    }
    switch(stoi(commandUser)){
    case 1:
	CBD.ShowInfoClient();
	break;
    case 2:
	std::cout<<"Счет: "<<CBD.GetCount()<<" рублей\n"<<std::endl;;
	break;
    case 3:
	CBD.Deposit();
	if(CBD.GetAnswer()==211) MesSucc();
	else MesFail();
	break;

    case 4:
	CBD.Withdraw();
	if(CBD.GetAnswer()==221) MesSucc();
	else MesFail();
	break;
    case 5:
	CBD.Transfer();
	if(CBD.GetAnswer()==231) MesSucc();
	else MesFail();
	break;
    case 7:
	CBD.UpdateData();
	if(CBD.GetAnswer()==241) std::cout<<"Обновлено."<<std::endl;
	else MesFail();
	break;
    case 8:
	std::cout<<"Доступные команды:\n1 - Отобразить профиль\n2 - Отобразить счет\n3 - Пополнить счет\n4 - Снять со счета\n5 - Перевод\n7 - Обновить данные\n8 - Помощь\n9 - Выйти\n"<<std::endl;
	break;
    case 9:
	C_STATUS=false;
	break;
    default:
	std::cout<<"Неизвестная команда. Введите \"8\" для помощи.\n"<<std::endl;
	break;
    } 
}

int main(int argc, char **argv){  
    if(argc>3||argc<2){
	std::cerr << "Использование: ./ClientBD [параметр]\nПараметры:\n  -R         -регистрация\n  -U [login] -вход"<<std::endl;
	exit(1);
    }
    int cr;
    char typeStream='Q';
    while((cr=getopt(argc, argv, "RU:"))!=-1){
	switch(cr){
	case 'R':
	    typeStream='R';
	    break;
	case 'U':
	    typeStream='U';
	    CBD.SetLogin(optarg);
	    break;
	default:
	    std::cerr << "Использование: ./ClientBD [параметр]\nПараметры:\n  -R         -регистрация\n  -U [login] -вход"<<std::endl;
	    exit(1);
	}
    }
    signal(SIGINT, 0);
    CBD.Setup("0.0.0.0", 5000);
    switch(typeStream){
    case 'R':
	CBD.Registration();
	if(CBD.GetAnswer()==101){std::cout<<"Регистрация прошла успешно."<<std::endl;}
	else std::cout<<"Ошибка: Такой логин уже существует! Повторите попытку!"<<std::endl;
	CBD.exit();
	break;
    case 'U':
	CBD.Authorization();
	if(CBD.GetAnswer()==201) std::cout<<"Авторизация прошла успешно."<<std::endl;
	else {
	    std::cout<<"Ошибка: Неверный логин или пароль. Повторите попытку!"<<std::endl;
	    break;
	}
	std::cout<<"\nBankBD (1.0)\nВведите \"8\" для помощи.\n"<<std::endl;
	while(C_STATUS){
	    Invitation();
	}
	CBD.exit();
	break;
    default:
	SigExit(0);
    }
    return 0;
}
