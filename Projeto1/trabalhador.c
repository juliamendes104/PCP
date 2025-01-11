#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h> //Para resolver nomes de hosts em endereços IP e para preparar estruturas de sockaddr que podem ser usadas com funções de sockets
#include <sys/socket.h> //Define as funções e estruturas básicas necessárias para a programação com sockets
#include <arpa/inet.h> //Fornece funções para manipulação de endereços de rede, especialmente para conversão para formatos binários usados pelos sockets
#include <time.h>

#define PORT_MANAGER "8080"
#define PORT_WORKER0 "8081"
#define PORT_WORKER1 "8082"
#define PORT_WORKER2 "8083"
#define PORT_WORKER3 "8084"
#define PORT_WORKER4 "8085"
#define PORT_WORKER5 "8086"
#define PORT_WORKER6 "8087"
#define PORT_WORKER7 "8088"
#define WAIT_TIME 2

typedef struct{
    int id;
    int num;
}Worker;

void check_error(const int code, const char *func_name){
    if (code == -1) {
        perror("func_name");
        exit(1);
    }
}

struct addrinfo* resolver_porta(struct addrinfo hints, int id){
    struct addrinfo *res;
    int err;
    if(id == 0){
    	err = getaddrinfo("127.0.0.1", PORT_WORKER0, &hints, &res);
    }
    else if(id == 1){
    	err = getaddrinfo("127.0.0.1", PORT_WORKER1, &hints, &res);
    }
    else if(id == 2){
    	err = getaddrinfo("127.0.0.1", PORT_WORKER2, &hints, &res);
    }
    else if(id == 3){
    	err = getaddrinfo("127.0.0.1", PORT_WORKER3, &hints, &res);
    }
    else if(id == 4){
    	err = getaddrinfo("127.0.0.1", PORT_WORKER4, &hints, &res);
    }
    else if(id == 5){
    	err = getaddrinfo("127.0.0.1", PORT_WORKER5, &hints, &res);
    }
    else if(id == 6){
    	err = getaddrinfo("127.0.0.1", PORT_WORKER6, &hints, &res);
    }
    else{
    	err = getaddrinfo("127.0.0.1", PORT_WORKER7, &hints, &res);
    }

    if(err != 0){
        fprintf(stderr, "getaddrinfo: %s", gai_strerror(err));
        exit(1);
    }
    return res;
}

void barreira_borboleta_recursiva(int id, int num, int nivel, struct addrinfo hints){
    if (nivel == 3) {
        struct addrinfo *res;
        int err = getaddrinfo("127.0.0.1", PORT_MANAGER, &hints, &res);
        if(err != 0){
            fprintf(stderr, "getaddrinfo: %s", gai_strerror(err));
            exit(1);
    	}		
        int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); //Utiliza sockfd para a comunicacao
    	check_error(sockfd, "socket()");

    	err = connect(sockfd, res->ai_addr, res->ai_addrlen);
    	check_error(err, "connect()");
    	
    	int bytes = send(sockfd, &num, sizeof(num), 0);
    	if(bytes == -1){
    	    perror("send");
    	    exit(1);
    	}
    	close(sockfd);
        freeaddrinfo(res);
        return; //todos os estagios foram concluidos
    }
    
    int parceiro_id = id ^ (1 << nivel);
    int mensagem;
    int err;
    int sockfd;
    struct addrinfo *res;
    
    if(id > parceiro_id){
    	//processo e mensageiro
    	res = resolver_porta(hints,parceiro_id);
    	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); //Utiliza sockfd para a comunicacao
    	check_error(sockfd, "socket()");

	//se a conexao falhar, processo remetente pode nao ter chego ainda
	while(1){
	    err = connect(sockfd, res->ai_addr, res->ai_addrlen);
	    if(err == -1){
	        sleep(WAIT_TIME);
	    }
	    else{
	        break;
	    }
	}
    	
    	while(1){
    	    int bytes = send(sockfd, &id, sizeof(id), 0);
    	    if(bytes == -1){
    	    	perror("send");
    	        exit(1);
    	    }
    	
    	    bytes = recv(sockfd, &mensagem, sizeof(mensagem), 0);
    	
    	    if(mensagem == 1){
	        //confirmacao, mensageiro pode concluir sua tarefa
	    	bytes = send(sockfd, &num, sizeof(num), 0);
	    	close(sockfd);
	    	freeaddrinfo(res);
	    	return;
    	    }
    	    else{
    	    	//se negacao, mensageiro deve tentar reconectar em momento posterior
		//printf("Negacao recebida, tentando reconectar...\n");

		close(sockfd); // Fechar o socket atual
		freeaddrinfo(res); // Liberar as informações da conexão anterior

		sleep(WAIT_TIME); // Espera antes de tentar reconectar

		// Resolver novamente o endereço e tentar reconectar
		res = resolver_porta(hints, parceiro_id);
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		check_error(sockfd, "socket()");

		err = connect(sockfd, res->ai_addr, res->ai_addrlen);
		if (err == -1) {
		    perror("Erro ao tentar reconectar");
		}
    	    }
    	}
    }
    else{
    	//processo e remetente
    	res = resolver_porta(hints,id);
    	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); //Utiliza sockfd para a comunicacao
    	check_error(sockfd, "socket()");
    	
    	int opt = 1;
    	int trabalhadorfd;
    	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    	
    	err = bind(sockfd, res->ai_addr, res->ai_addrlen);
    	check_error(err, "bind()");

    	err = listen(sockfd, 5);
    	check_error(err, "listen()");
    	
    	while(1){
    	    //printf("Esperando\n");
	    trabalhadorfd = accept(sockfd, NULL, NULL);
	    check_error(err, "accept()");
	    	
	    int mensageiro_id;
	    int bytes = recv(trabalhadorfd, &mensageiro_id, sizeof(mensageiro_id), 0);
	    
	    if(mensageiro_id == parceiro_id){
		mensagem = 1;
		int num_mensageiro;
	    	send(trabalhadorfd, &mensagem, sizeof(mensagem), 0);
	    	bytes = recv(trabalhadorfd, &num_mensageiro, sizeof(num_mensageiro), 0);
		num = num + num_mensageiro;
		//printf("Soma parcial: %d\n", num);
		break;
	    }
	    else{
	        mensagem = 0;
	    	send(trabalhadorfd, &mensagem, sizeof(mensagem), 0);
	    }
    	
    	}
    	close(trabalhadorfd);
    }
    
    close(sockfd);
    freeaddrinfo(res);
    
    barreira_borboleta_recursiva(id,num,nivel+1,hints);
}

int main(int argc, char* argv[]){
    Worker no;
    if(argc > 1){
    	int id = atoi(argv[1]);
    	srand(time(NULL) + id);
    	no.id = id;
    	no.num = rand() % 101;
    	printf("ID: %d\n", no.id);
    	printf("Numero: %d\n", no.num);
    }
    else{
    	printf("Nenhum argumento foi passado.\n");
    }
    struct addrinfo hints;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    barreira_borboleta_recursiva(no.id, no.num, 0, hints);
    
    printf("Finalizado");

}
