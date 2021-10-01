 /*
 *  Autor:              Juliano Cesar Petini & Michel Gomes de Souza
 *  Data de criação:    25/07/2021
 *  Descrição:           
 *                  Faça um Jogo da Velha que possibilite dois clientes se conectarem ao servidor usando sockets. No
 *                  servidor, para cada cliente será criado um processo filho. Os processos filhos devem se comunicar via
 *                  pipe para trocarem os lances dos jogadores. Ao final do jogo, os processos filhos devem ser finalizados. 
 */
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

/*  Main.
 *  Descrição:  
 *             - Cria a estrutura do socket.
 *             - Cria os pipes.
 *             - Gerencia a comunicação entre os processos.
 */
int main(){
    int fd, conn, child_pid, pipe_1[2], pipe_2[2], flag = 1;
    char message[100] = " ", buffer_1[4], buffer_2[4]; 
    
	fd = socket(AF_INET, SOCK_STREAM, 0);
    
    printf("Servidor Iniciado!\n");
    //Certifica de estar limpa os espaços de memória.
    memset(message, '\0',sizeof(message));
    memset(buffer_1, '\0',sizeof(buffer_1));
    memset(buffer_2, '\0',sizeof(buffer_2));

    //Configura o Socket.
    struct sockaddr_in serv; 
    memset(&serv, '\0', sizeof(serv)); 
	serv.sin_family = AF_INET;
	serv.sin_port = htons(8096); 
	serv.sin_addr.s_addr = htonl(INADDR_ANY);;
	bind(fd, (struct sockaddr *)&serv, sizeof(serv)); 
	listen(fd, 2);

    //Abre os pipes.
    pipe(pipe_1); pipe(pipe_2);

    //Fica aguardando uma conexão ser aceita pelo socket.
	while((conn = accept(fd, (struct sockaddr *)NULL, NULL))) {
        int flag_fork = flag;//Flag diferenciar os filhos.
        if(flag == 1){ flag = 0; }

		if ( (child_pid = fork ()) == 0 ){//Faz o fork.
            if (flag_fork == 1){
                send(conn, "Yes", 4, 0);//Emite uma mensagem avisando que é o primeiro a se conectar.
                bzero(message, 4);

                while (!read(pipe_1[0], buffer_1, sizeof(buffer_1))){ }
                send(conn, "Con", 4, 0);//Emite uma mensagem que já chegou outro jogador.

                while (!read(pipe_1[0], buffer_1, sizeof(buffer_1))){ }
                send(conn, buffer_1, sizeof(buffer_1), 1);//Espera a primeira jogada do oponente.

                //Fica gerenciando o pipe e oque é enviado para o processo via socket.
                while (recv(conn, message, 100, 0) > 0) {
                    close(pipe_2[0]);//Escreve no pipe.
                    write(pipe_2[1], message, 4);

                    close(pipe_1[1]);//Le do pipe.
                    read(pipe_1[0], buffer_1, sizeof(buffer_1));
                    send(conn, buffer_1, 4, 0);//Envia jogada feita pelo oponente via socket.
                }
            }
            else {
                send(conn, "No", 4, 0);//Emite uma mensagem avisando que é o segundo a se conectar.
                bzero(message, 4);

                close(pipe_1[0]);//Avisa para o outro processo que chegou outro jogador.
                write(pipe_1[1], "Sta", 4);

                //Fica gerenciando o pipe e oque é enviado para o processo via socket.
                while (recv(conn, message, 100, 0) > 0) {
                    close(pipe_1[0]);//Escreve no pipe.
                    write(pipe_1[1], message, 4);

                    close(pipe_2[1]);//Le do pipe.
                    read(pipe_2[0], buffer_2, sizeof(buffer_2));
                    send(conn, buffer_2, 4, 0);//Envia jogada feita pelo oponente via socket.
                }
            }
        }
	}
    close(conn);
	exit(0);
}
