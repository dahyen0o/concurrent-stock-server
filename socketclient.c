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
#define BUF_SIZE 500
#define MAXLEN 500

void error_handling(char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);
 
int main(int argc, char *argv[]) {
    int sock;
    char buf[MAXLEN];
    struct sockaddr_in serv_adr;
 
    //타이머를 계산하기 위해 timeval형 변수 timeout 설정
    struct timeval timeout;
    //fd_set형 변수 선언하여 파일 디스크립터 정보 등록
    fd_set reads, cpy_reads;
    //검사할 파일 디스크립터의 수
    int fd_max, fd_num;
 
    if(argc!=3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
 
    sock=socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_adr.sin_port=htons(atoi(argv[2]));
 
    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
        error_handling("connect() error!");
 
    //fd_set형 변수 reads의 모든 비트를 0으로 초기화
    FD_ZERO(&reads);
    //입력 및 소켓 디스크립터 정보를 등록
    FD_SET(0, &reads);
    FD_SET(sock, &reads);
    //소켓의 번호를 저장
    fd_max = sock;
    while(1) {
        cpy_reads = reads;
        //타임아웃 시간 설정
        timeout.tv_sec=500;
        timeout.tv_usec=0;
        //소켓 포함 모든 파일디스크립터를 대상으로 '수신된 데이터의 존재여부' 검사
        fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout);
        //오류가 발생했다면
        if(fd_num==-1) {
            error_handling("select() error\n");
        }
        //Timeout이 발생했다면
        else if (fd_num == 0) {
            puts("Time-out!");
            continue;
        }
        //변화가 발생했다면
        else {
            //만약 입력 디스크립터에 변화가 발생했다면 (사용자로부터 데이터를 입력받았다면)
            if(FD_ISSET(0, &cpy_reads)) {
                //입력된 내용을 서버로 전송하는 함수
                write_routine(sock, buf);
                //파일 디스크립터 정보 삭제
                FD_CLR(0, &cpy_reads);
            }
            //만약 포트에 변화가 발생했다면(서버로부터 데이터를 받았다면)
            if(FD_ISSET(sock, &cpy_reads)) {
                //받은 데이터를 출력하는 함수
                read_routine(sock, buf);
                //파일 디스크립터 정보 삭제
                FD_CLR(sock, &cpy_reads);
            }
        }
    }
    close(sock);
    return 0;
}
 
void read_routine(int sock, char *buf)
{
    int str_len=read(sock, buf, MAXLEN);
    if(str_len==0)
        return;
 
    buf[str_len]=0;
    printf("%s", buf);
}

void write_routine(int sock, char *buf)
{
    fgets(buf, BUF_SIZE, stdin);
    if(!strcmp(buf,"exit\n"))
    {
        shutdown(sock, SHUT_WR);
        exit(0);
    }
    write(sock, buf, sizeof(buf));
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

