#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>

#define CHATDATA 1024
#define IPADDR "127.0.0.1"
#define PORT 9000
#define WHISPER "/w"
#define DELIMETER " "

void *do_send_chat(void *);
void *do_receive_chat(void *);
pthread_t thread_1, thread_2; //두 개의 스레드 생성

char    escape[ ] = "exit";
char    nickname[20];

int main(int argc, char *argv[ ])
{
    int c_socket; //소켓을 담을 변수 선언
    struct sockaddr_in c_addr; //소켓 구조체
    int len;
    char chatData[CHATDATA];
    char buf[CHATDATA];
    int nfds;
    fd_set read_fds; //fd 구조체
    int n;
    c_socket = socket(PF_INET, SOCK_STREAM, 0); //소켓 생성
    memset(&c_addr, 0, sizeof(c_addr)); //소켓 구조체 설정
    c_addr.sin_addr.s_addr = inet_addr(IPADDR);
    c_addr.sin_family = AF_INET;
    c_addr.sin_port = htons(PORT);
    printf("Input Nickname : ");
    scanf("%s", nickname); //닉네임 설정

    if(connect(c_socket, (struct sockaddr *) &c_addr, sizeof(c_addr)) == -1) { //소켓 연결
        printf("Can not connect\n");
        return -1;
    }
	write(c_socket, nickname, strlen(nickname));
	pthread_create(&thread_1, NULL, do_send_chat, (void *) &c_socket); //스레드 생성
	pthread_create(&thread_2, NULL, do_receive_chat, (void *) &c_socket);
	// 1: 성공적으로 함수가 호출되면 이곳에 스레드 ID 저장 이 인자로 pthread_join 함수 사용
	// 2: 스레드의 특성 정의 기본적으로 NULL 3: 어떤 로직을 할지 함수포인터를 매개변수로 받음 4:전달될 인자
	pthread_join(thread_1, NULL); //th로 시작된 스레드가 종료되는걸 기다림
	pthread_join(thread_2, NULL);
	close(c_socket);
}

void * do_send_chat(void *arg) //채팅 보내는 함수
{
    char chatData[CHATDATA];
    char buf[CHATDATA];
    int n;
    int c_socket = *((int *) arg);  //클라이언트 소켓
    while(1) {
        memset(buf, 0, sizeof(buf)); //어떤 메모리의 시작점부터 연속된 범위를 모두 지정
        if((n = read(0, buf, sizeof(buf))) > 0 ) { //버퍼의 사이즈만큼 읽기 0은 파일의 끝을 나타내기 때문에 0보다 크다면
			char *token = NULL; //채팅에 사용할 변수 선언
			char *toNickname = NULL;
			char *message = NULL;
			if(strncasecmp(buf, WHISPER, 2) == 0){ //입력한 버퍼가 /w 와 같다면
				token = strtok(buf, DELIMETER); //DELIMETER만큼 문자열 잘라서 token에  /w 저장
				if(token != NULL)  //토큰이 NULL값이 아니면 문자열을 DELIMETER 단위로 분리해서 닉네임 저장
					toNickname = strtok(NULL, DELIMETER);
				if(toNickname != NULL) //나머지 메시지 잘라서 저장
					message = strtok(NULL, "\0");
				if(token == NULL || toNickname == NULL || message ==  NULL){
					printf("Your whisper message is wrong. Please input '/w nickname message'\n");
					continue; //셋 중 하나라도 NULL이면 오류메시지 출력하여 귓속말 전송 실패
				}

				sprintf(chatData, "%s %s [%s] %s", token, toNickname, nickname, message);
				// 귓속말 문자열을 만들어서 charData에 문자열의 길이 저장
			} else { 
				sprintf(chatData, "[%s] %s", nickname, buf); //일반 채팅 메시지 문자열을 만들어서 chatData에 저장
			}
			write(c_socket, chatData, strlen(chatData)); // 채팅 메시지 전달
			if(!strncmp(buf, escape, strlen(escape))) { //'exit' 메세지를 입력하면
				pthread_kill(thread_2, SIGINT); //do_receive_char 스레드를 종료시킴
				break; //자신도 종료
            }
        }
    }
}
void *do_receive_chat(void *arg)
{
    char   chatData[CHATDATA]; //메시지 변수
    int    n;
    int    c_socket = *((int *)arg); //클라이언트 소켓
    while(1) {
        memset(chatData, 0, sizeof(chatData));
        if((n = read(c_socket, chatData, sizeof(chatData))) > 0 ) { //
            write(1, chatData, n); //chatData를 화면에 출력함 (1 = stdout (모니터))
        }
    }
}
