
#include <sys/wait.h>//NOT
#include <stdio.h>
#include <sys/stat.h>//NOT
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>//NOT
#include <signal.h>//NOT~
#include <sys/resource.h>//NOT

#include <arpa/inet.h>//NOT
#include <unistd.h>//NOT


#include "../../src/dependencies/declaration.h"
#include "../../src/dependencies/definition.c"

typedef struct CwebKeyVal{
    char *key;
    char *value;

}CwebKeyVal;

typedef struct CwebDict {
  int size;
  CwebKeyVal **keys_vals;

}CwebDict;

typedef struct CwebHttpRequest{

    char *url;
    int socket;
    char *route;
    char *method;
    char *client_ip;
    int content_error;
    CwebDict *params;
    CwebDict *headers;
    int content_length;
    unsigned char *content;
    cJSON *json;
    void *hydratation;
    void *extra_args;
    UniversalGarbage *garbage;

}CwebHttpRequest;

typedef struct CwebHttpResponse{
    int status_code;
    struct CwebDict *headers;
    int content_length;
    bool exist_content;
    unsigned char *content;

}CwebHttpResponse;

typedef struct CwebServer{
    int port;
    int function_timeout;
    double client_timeout;
    int max_queue;
    bool single_process;
    long max_requests;
    bool allow_cors;
    bool use_static;
    const char *static_folder;
    bool use_cache;

    CwebHttpResponse *(*request_handler)(CwebHttpRequest *request);

}CwebServer;

#define PORTA 3000

 CwebServer  newCwebSever(int port , CwebHttpResponse *(*request_handler)(CwebHttpRequest *request)){
    struct CwebServer self = {0};
    self.port = port;
    self.function_timeout = 30;
    self.client_timeout = 5;
    self.max_queue = 100;
    self.single_process = false;
    self.allow_cors = true;
    self.max_requests = 1000;
    self.static_folder = "static";
    self.use_static = true;
    self.use_cache = true;

    self.request_handler = request_handler;

    return self;
}

CwebDict *newCwebDict(){
    CwebDict *self = (CwebDict *)malloc(sizeof(struct CwebDict));
    self->keys_vals = (CwebKeyVal**)malloc(1);
    self->size = 0;
    return self;
}

CwebHttpRequest *newCwebHttpRequest(int socket){
    CwebHttpRequest *self = ( CwebHttpRequest*)malloc(sizeof( CwebHttpRequest));
    *self = (CwebHttpRequest){0};
    self->socket = socket;
    self->params = newCwebDict();
    self->headers = newCwebDict();
    self->garbage = newUniversalGarbage();
    return self;
}

void private_CWebServer_execute_request(CwebServer *self,int socket,const char *client_ip){

    printf("Parsing Request\n");
    CwebHttpRequest *request = newCwebHttpRequest(socket);
    request->client_ip = strdup(client_ip);


    int result = CwebHttpRequest_parse_http_request(request);

    if(result == INVALID_HTTP){
        cweb_print("Invalid HTTP Request\n");
        CwebHttpRequest_free(request);
        return;
    }

    if(result == READ_ERROR){
        cweb_print("Read Error \n");
        CwebHttpRequest_free(request);
        return;
    }
    if(result == MAX_HEADER_SIZE_CODE){
        cweb_print("Max Header Size\n");
        CwebHttpRequest_free(request);
        return;
    }

    cweb_print("created request\n");
    cweb_print("Request method: %s\n", request->method);
    cweb_print("Request url: %s\n", request->url);



    CwebHttpResponse *response = NULL;

    if(self->use_static){
        response = private_cweb_generate_static_response(request,self->use_cache);
    }


    if(!response){
        //lambda que o usuario passa
        response = self->request_handler(request);
    }

    if(response && self->allow_cors){
        private_cweb_generate_cors_response(response);
    }
    cweb_print("executed client lambda\n");


    //means that the main function respond nothing
    if (response == NULL){

        if(self->use_static){
            char formated_404_path[1000]={0};
            sprintf(formated_404_path,"%s/404.html", cweb_static_folder);
            char *formated_html = cweb_load_string_file_content(formated_404_path);

            if(formated_html != NULL){
                response = cweb_send_var_html_cleaning_memory(
                        formated_html,
                        404);
            }

            else{
                response = cweb_send_var_html(private_cweb_404, 404);
            }
        }
        else{
            response = cweb_send_var_html(private_cweb_404, 404);

        }


    }

    char *response_str = CwebHttpResponse_generate_response(response);
    cweb_print("Response created\n");





    send(socket, response_str, strlen(response_str), MSG_NOSIGNAL);

    // Enviando conteúdo byte a byte
    if (response->exist_content)
    {
        size_t sent = 0;
        while (sent < response->content_length)
        {
            size_t chunk_size = response->content_length - sent;

            ssize_t res = send(socket, response->content + sent, chunk_size, MSG_NOSIGNAL);
            if (res < 0)
            {
                break;
            }
            sent += res;
        }
    }


    free(response_str);
    CwebHttpResponse_free(response);
    CwebHttpRequest_free(request);
    cweb_print("Cleared memory\n");
}

int private_CWebServer_run_server_in_single_process(CwebServer *self) {

    int port_socket;

    // Creating socket file descriptor
    if ((port_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Faluire to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Configurando a estrutura de endereço do servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(self->port);


    // Vinculando o socket à porta especificada
    if (bind(port_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("Faluire to bind socket to port %d\n", self->port);
        return 1;

    }

    // Waiting for connections
    if (listen(port_socket, self->max_queue) < 0)
    {
        perror("Faluire to listen connections");
        exit(EXIT_FAILURE);
    }

    // Main loop
    printf("Sever is running on port:%d\n", self->port);


    while (1)
    {

        if(private_cweb_end_server){
                    cweb_print("Break in request %lld\n", cweb_actual_request)
                    break;
        }

        cweb_actual_request++;

        //clear every trash

        // Accepting a new connection in every socket
        int client_socket = accept(
            port_socket,
            (struct sockaddr *)&address,
            (socklen_t *)&addrlen
        );

        char client_ip[INET_ADDRSTRLEN] ={0};
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);


        cweb_print("----------------------------------------\n")
        cweb_print("Executing request:%lld\n", cweb_actual_request)
        cweb_print("Socket: %d\n", client_socket)


        if ( client_socket< 0){
            perror("Faluire to accept connection");
            exit(EXIT_FAILURE);
        }

        struct timeval timer1;
        timer1.tv_sec =  0;
        timer1.tv_usec =  0100000;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timer1, sizeof(timer1));


        char buffer[1];
        ssize_t peek_result = recv(client_socket, buffer, 1, MSG_PEEK);

        if (peek_result <= 0) {
            cweb_print("peek: %li\n",peek_result)
            cweb_print("Conection closed By the  Client\n")
            close(client_socket);  // Fechar o socket do cliente
            continue;
        }


        struct timeval timer2;
        long seconds =  (long)self->client_timeout;
        timer2.tv_sec =  seconds ;  // tempo em segundos
        timer2.tv_usec =(long)((self->client_timeout - (double )seconds) * 1000000);
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timer2, sizeof(timer2));


        private_CWebServer_execute_request(self,client_socket, client_ip);


        close(client_socket);


        cweb_print("Closed Conection with socket %d\n", client_socket)


    }
    return 0;
}
