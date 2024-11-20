
#include "../uniq.definitions_requirements.h"
int private_CWebServer_run_server_in_single_process(CwebServer *self) {

    int port_socket;

    // Creating socket file descriptor
    if ((port_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
        printf("\nFaluire to create socket, error:%d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Configurando a estrutura de endereço do servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(self->port);


    // Vinculando o socket à porta especificada
    if (bind(port_socket, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        printf("Faluire to bind socket to port %d\n", self->port);
        return 1;

    }

    // Waiting for connections
    if (listen(port_socket, self->max_queue) < 0)
    {
        printf("\nFaluire to listen connections, error: %d\n", WSAGetLastError());
        closesocket(port_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Main loop
    printf("Sever is running on port:%d\n", self->port);


    
    while (1) {
        if (private_cweb_end_server) {
            cweb_print("Break in request %lld\n", cweb_actual_request);
            break;
        }

        cweb_actual_request++;

        SOCKET client_socket = accept(port_socket, (struct sockaddr *)&address, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            printf("Failure to accept connection: %d\n", WSAGetLastError());
            continue;
        }

        char client_ip[INET_ADDRSTRLEN] = {0};
        sprintf(client_ip, "%s", inet_ntoa(address.sin_addr));

        cweb_print("----------------------------------------\n");
        cweb_print("Executing request:%lld\n", cweb_actual_request);
        cweb_print("Socket: %d\n", client_socket);

        struct timeval timer1;
        timer1.tv_sec = 0;
        timer1.tv_usec = 100000;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timer1, sizeof(timer1));

        char buffer[1];
        ssize_t peek_result = recv(client_socket, buffer, 1, MSG_PEEK);
        if (peek_result == SOCKET_ERROR || peek_result == 0) {
            cweb_print("peek: %lld\n", peek_result);
            cweb_print("Connection closed By the Client\n");
            closesocket(client_socket);
            continue;
        }

        struct timeval timer2;
        long seconds = (long)self->client_timeout;
        timer2.tv_sec = seconds;
        timer2.tv_usec = (long)((self->client_timeout - (double)seconds) * 1000000);
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timer2, sizeof(timer2));

        private_CWebServer_execute_request(self, client_socket, client_ip);

        closesocket(client_socket);

        cweb_print("Closed Connection with socket %d\n", client_socket);
    }

    return 0;
}

