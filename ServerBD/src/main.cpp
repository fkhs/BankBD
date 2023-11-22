#include "Server.h"

int main () {
    ServerBD BD;
    BD.SetupInPSQL();
    BD.Setup(5000); 
    BD.CreateWorkerThreads(5);
    BD.Receive();
    return 0;
}
