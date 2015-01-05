//  Hello World server

#include "panda_zmq.hpp"
#include "panda_head.hpp"
using namespace zmq;
int main (void)
{
    //  Socket to talk to clients
    context_t ctx(16);
    socket_t s(ctx,ZMQ_REP);
    s.bind ("tcp://*:5556");
    while (1) {
        char buffer [10];
        s.recv(buffer,10);
        cout<<"Received Hello"<<endl;
        sleep (1);          //  Do some 'work'
        s.send ("World", 5);
    }
    return 0;
}
