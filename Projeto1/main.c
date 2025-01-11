//Gabryella Mika e Julia Mendes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h> //Para resolver nomes de hosts em endereços IP e para preparar estruturas de sockaddr que podem ser usadas com funções de sockets
#include <sys/socket.h> //Define as funções e estruturas básicas necessárias para a programação com sockets
#include <arpa/inet.h> //Fornece funções para manipulação de endereços de rede, especialmente para conversão para formatos binários usados pelos sockets

#define PORT_MANAGER "8080"

void check_error(const int code, const char *func_name){
    if (code == -1) {
        perror("func_name");
        exit(1);
    }
}

void criar_worker(int id){
    char comando[100];
    sprintf(comando, "gnome-terminal -- bash -c './trabalhador %d; exec bash'", id);
    system(comando);
}

int main(){
    struct addrinfo hints, *res;
    int trabalhadorfd;

    printf("Iniciando\n");
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err = getaddrinfo(NULL, PORT_MANAGER, &hints, &res);

    if(err != 0){
        fprintf(stderr, "getaddrinfo: %s", gai_strerror(err));
        exit(1);
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); //Utiliza sockfd para a comunicacao
    check_error(sockfd, "socket()");

    err = bind(sockfd, res->ai_addr, res->ai_addrlen);
    check_error(err, "bind()");

    err = listen(sockfd, 5);
    check_error(err, "listen()");
    
    criar_worker(7);
    criar_worker(4);
    criar_worker(2);
    criar_worker(5);
    criar_worker(0);
    criar_worker(6);
    criar_worker(1);
    criar_worker(3);
    
    trabalhadorfd = accept(sockfd, NULL, NULL);
    check_error(trabalhadorfd, "accept()");

    int numero;
    int bytes = recv(trabalhadorfd, &numero, sizeof(numero), 0);
    if(bytes == -1){
    	    perror("send");
    	    exit(1);
    }
    
    printf("Soma total: %d\nFechando conexao com servidor.\n", numero);
    
    close(sockfd);
    close(trabalhadorfd);
    freeaddrinfo(res);
}
