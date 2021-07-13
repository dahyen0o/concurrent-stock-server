/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
 */
#include "csapp.h"
#define BUF_SIZE 100
#define MAXLEN 1000

struct item{
	int id, left_stock, price, readcnt;
	//sem_t mutex
};

typedef struct node{
	struct node *lchild, *rchild;
	struct item i;
} node;
node *head;

void insertnode(int, int, int);
node* searchnode(int);
void error_handling(char *buf);
int echo(int, char []);
void preorder(node *, char []);
 
int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;
 
    socklen_t adr_sz;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];
    if(argc!=2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

	head = NULL;
	int id, left_stock, price;
	FILE *finput = fopen("stock.txt", "r");
	while(1){
		if(fscanf(finput, "%d %d %d", &id, &left_stock, &price) < 0) break;
		insertnode(id, left_stock, price);
	}
	fclose(finput);

    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));
    
    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
        error_handling("bind() error");
    if(listen(serv_sock, 5)==-1)
        error_handling("listen() error");
 
    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max=serv_sock;
 
    while(1)
    {
        cpy_reads=reads;
        timeout.tv_sec=500;
        timeout.tv_usec=5000;
 
        if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1)
            break;
        
        if(fd_num==0)
            continue;
 
        for(i=0; i<fd_max+1; i++)
        {
            if(FD_ISSET(i, &cpy_reads))
            {
                if(i==serv_sock)     // connection request!
                {
                    adr_sz=sizeof(clnt_adr);
                    clnt_sock=
                        accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
                    FD_SET(clnt_sock, &reads);
                    if(fd_max<clnt_sock)
                        fd_max=clnt_sock;
                    printf("connected client: %d \n", clnt_sock);
                }
                else    // read message!
                {
					buf[0] = '\0';
                    str_len=read(i, buf, BUF_SIZE);
                    if(str_len==0)    // close request!
                    {
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client: %d \n", i);
                    }
                    else
                    {
						int flag = echo(i, buf);
						buf[0] = '\0';
						if (flag == 0) FD_CLR(i, &reads);
                        //write(i, buf, str_len);    // echo!
                    }

					FILE *foutput = fopen("stock.txt", "w");
					char output[MAXLEN] = "";
						
					preorder(head, output);
					fprintf(foutput, "%s", output);
					fclose(foutput);
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}
 
void error_handling(char *buf)
{
    fputs(buf, stderr);
    fputc('\n', stderr);
    exit(1);
}

void insertnode(int id, int left_stock, int price){
	node *newnode = (node *)malloc(sizeof(node));
	newnode->i.id = id;
	newnode->i.left_stock = left_stock;
	newnode->i.price = price;
	newnode->lchild = newnode->rchild = NULL;

	if(!head){
		head = newnode; return;
	}

	node *curr = searchnode(id);
	if(id < curr->i.id) curr->lchild = newnode;
	else curr->rchild = newnode;
}

node *searchnode(int id){
	node *curr = head;
	while(1){
		if(curr->i.id == id) return curr;
		else if(curr->i.id < id){
			if(curr->rchild) curr = curr->rchild;
			else return curr;
		}
		else{
			if(curr->lchild) curr = curr->lchild;
			else return curr;
		}
	}
}

int echo(int fd, char buf[]){
	char *argv[4];
	char output[MAXLEN] = "";

	argv[0] = strtok(buf, " ");

	if(!strcmp(argv[0], "show\n")){
		preorder(head, output);
		write(fd, output, sizeof(output));
		return 1;
	}

	else if(!strcmp(argv[0], "exit\n")){
        close(fd);
        printf("closed client: %d \n", fd);
		return 0;
	}

	else if(!strcmp(argv[0], "buy")){
	argv[1] = strtok(NULL, " ");
	argv[2] = strtok(NULL, "\n");
		int id = atoi(argv[1]);
		int stock = atoi(argv[2]);

		node *curr = searchnode(id);
		if(curr->i.id != id){
			//write(fd, "Not id exists\n", MAXLINE);
			strcpy(output, "Not id exists\n");
			write(fd, output, sizeof(output));
			return 1;
		}
		else if(curr->i.left_stock < stock){
			//write(fd, "Not enough left stock\n\0", MAXLINE);
			strcpy(output, "Not enough left stock\n");
			write(fd, output, sizeof(output));
			return 1;
		}
		
		curr->i.left_stock = curr->i.left_stock - stock;
		//write(fd, "[buy] succes\n\0", MAXLINE);
		strcpy(output, "[buy] success\n");
		write(fd, output, sizeof(output));
		return 1;
	}

	else if(!strcmp(argv[0], "sell")){
	argv[1] = strtok(NULL, " ");
	argv[2] = strtok(NULL, "\n");
		int id = atoi(argv[1]);
		int stock = atoi(argv[2]);
		
		node *curr = searchnode(id);
		strcpy(output, "[sell] success\n");
		if(curr->i.id != id){
			strcpy(output, "Not id exists\n");
			//write(fd, "Not id exists\n\0", MAXLINE);
			write(fd, output, sizeof(output));
			return 1;
		}
		
		curr->i.left_stock = curr->i.left_stock + stock;
		write(fd, output, sizeof(output));
		//write(fd, "[sell] success\n", MAXLINE);
		return 1;
	}

	else{
		write(fd, "Not proper command\n\0", MAXLEN);
	}

	return 1;
}

void preorder(node *curr, char output[]){
	if(!curr) return;

	char tmp[100];
	sprintf(tmp, "%d %d %d\n", 
			curr->i.id, curr->i.left_stock, curr->i.price);
	strcat(output, tmp);

	preorder(curr->lchild, output);
	preorder(curr->rchild, output);
}

