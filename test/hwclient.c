//  Hello World client
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main (void)
{
    printf ("Connecting to hello world server...\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5556");
    //zmq_connect (requester, "tcp://localhost:5556");
    int t=5000;
    zmq_setsockopt(requester,ZMQ_LINGER,&t,sizeof(int));
    int request_nbr;
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        char buffer [10];
        printf ("Sending Hello %d...\n", request_nbr);
        zmq_send (requester, "Hello", 5, 0);
	printf("Sendover\n");
        //zmq_recv (requester, buffer, 10, ZMQ_DONTWAIT);
        zmq_recv (requester, buffer, 10, 0);
        printf ("Received World %d\n", request_nbr);
    }
    zmq_close (requester);
    printf("close\n");
    zmq_ctx_destroy (context);
    printf("destroy\n");
    return 0;
}
