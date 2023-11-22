#include "SignInPSQL.h"
#include <string>

SignInPSQL::SignInPSQL(){
    PGconn* conn = NULL;
    PGresult* res = NULL;
}

SignInPSQL::~SignInPSQL(){
    PQfinish(conn);
}

void SignInPSQL::Terminate(int code){
    if(code != 0)
	fprintf(stderr, "%s\n", PQerrorMessage(conn));

    if(res != NULL)
        PQclear(res);

    if(conn != NULL)
        PQfinish(conn);

    exit(code);
}

void SignInPSQL::ClearRes(){
    PQclear(res);
    res = NULL;
}

void SignInPSQL::ProcessNotice(void *arg, const char *message){
    UNUSED(arg);
    UNUSED(message);
}

void SignInPSQL::Setup(){
    int libpq_ver = PQlibVersion();
    printf("Version of libpq: %d\n", libpq_ver);
    std::string lconninfo=SignIn();
    const char* conninfo =lconninfo.c_str();
    conn = PQconnectdb(conninfo);
    if(PQstatus(conn) != CONNECTION_OK)
	Terminate(1);
    int server_ver = PQserverVersion(conn);
    char *user = PQuser(conn);
    char *db_name = PQdb(conn);
    printf("\nServer version: %d\n", server_ver);
    printf("User: %s\n", user);
    printf("Database name: %s\n", db_name);
}

int SignInPSQL::getch(){
    int ch;
    struct termios old_settings, new_settings;
    tcgetattr(STDIN_FILENO, &old_settings);
    new_settings = old_settings;
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
    return ch;
}
std::string SignInPSQL::GetPassworddb(){
    char password[SIZE];
    int i = 0;
    int ch;
    getch();
    printf("password: ");
    while ((ch = getch()) != '\n') {
	if (ch == 127 || ch == 8) { // handle backspace
            if (i != 0) {
                i--;
                printf("\b \b");
            }
        } else {
            password[i++] = ch;
        }
    }
    password[i] = '\0';

    std::string res=password;
    return res;
}
std::string SignInPSQL::GetUserdb(){
    std::string euser;
    std::cout<<"Initializing database login..."<<std::endl;
    std::cout<<"user: ";
    std::cin>>euser;
    return euser; 
}

std::string SignInPSQL::SignIn(){
    std::string resConnInfo="host=127.0.0.1 port=5432 dbname=bankbd user=";
    std::string a=GetUserdb();
    std::string b=GetPassworddb();
    return resConnInfo+a+" password="+b;
}

//возвращает pincode пользователя
int SignInPSQL::AuthorizationUser(std::string login){
    int shadow;
    std::string command = "select pincode from shadow where id=(select id from customers where login='"+login+"');";
    res = PQexec(conn, command.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
	Terminate(1);
    shadow=atoi(PQgetvalue(res,0,0));
    ClearRes();
    return shadow;
}

bool SignInPSQL::CheckExistsLogin(std::string login){
    std::string command = "select login from customers;";
    res = PQexec(conn, command.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
	Terminate(1);
    std::string checklogin;
    bool ex=false;
    int MAX_TUPLES=PQntuples(res);
    for(int i=0;i<MAX_TUPLES;i++){
	checklogin=PQgetvalue(res,i,0);
	if(checklogin==login) {
	    ex=true;
	    break;
	}
    }
    ClearRes();
    return ex;
}

void SignInPSQL::RegistrationUser(std::string login, int pswd, std::string surname, std::string name, std::string patronymic, std::string dataofb, std::string keyword){
    std::string command = "insert into customers values(default, '" + login +
       "'); insert into shadow values((select id from "
       "customers where login='" +
       login + "'), "+std::to_string(pswd)+", '"+keyword+"'); insert into accounts values((select id from "
       "customers where login='" +
       login + "'), '"+surname+"','"+name+"','"+patronymic+"','"+dataofb+"'); insert into bank_accounts values((select id from customers where login='"+login+"'), nextval('numberacc_serial'),0);"; 
    res = PQexec(conn, command.c_str());
    if(PQresultStatus(res) != PGRES_COMMAND_OK)
        Terminate(1);
    ClearRes();
}

std::string SignInPSQL::GetUserAccount(std::string login){
    std::string information="";
    std::string command = "select * from accounts where id=(select id from customers where login='"+login+"');";

    res = PQexec(conn, command.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
	Terminate(1);
    for(int i=1;i<5;i++){
	information+=PQgetvalue(res,0,i);
	if(i!=4)information+=" ";
    }
    ClearRes();
    return information;
}
std::string SignInPSQL::GetUserBankAccount(std::string login){
    std::string information="";
    std::string command = "select * from bank_accounts where id=(select id from customers where login='"+login+"');";

    res = PQexec(conn, command.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
	Terminate(1);
    for(int i=1;i<3;i++){
	information+=PQgetvalue(res,0,i);
	if(i!=2)information+=" ";
    }
    ClearRes();
    return information;
}

void SignInPSQL::SetUserCount(std::string login, std::string sum, bool op){
    std::string command="";
    if(op==true)
	command = "update bank_accounts set countacc=countacc+"+sum+" where id=(select id from customers where login='"+login+"');";
    else
	command = "update bank_accounts set countacc=countacc-"+sum+" where id=(select id from customers where login='"+login+"');";
    res = PQexec(conn, command.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
	Terminate(1);
    ClearRes();
}

std::string SignInPSQL::GetUserCount(std::string login){
    std::string count="";
    std::string command = "select countacc from bank_accounts where id=(select id from customers where login='"+login+"');";
    res = PQexec(conn, command.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
	Terminate(1);
    count=PQgetvalue(res,0,0);
    ClearRes();
    return count;
}

void SignInPSQL::Transaction(std::string sender, std::string sum, std::string receiver){
    std::string command = "begin; update bank_accounts set countacc=countacc-"+sum+" where id=(select id from customers where login='"+sender+"'); update bank_accounts set countacc=countacc+"+sum+" where id=(select id from customers where login='"+receiver+"'); commit;";
    res = PQexec(conn, command.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
	Terminate(1);
    ClearRes();
}
