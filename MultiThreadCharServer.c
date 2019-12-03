#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

void *do_chat(void *); //채팅 메세지를 보내는 함수
int pushClient(int, char *); //새로운 클라이언트가 접속했을 때 클라이언트 정보 추가
int popClient(int); //클라이언트가 종료했을 때 클라이언트 정보 삭제

pthread_t thread;
pthread_mutex_t mutex;

#define MAX_CLIENT 10 //최대 클라이언트 수
#define CHATDATA 1024
#define INVALID_SOCK -1
#define PORT 9000
#define WHISPER "/w"
#define DELIMETER " "

struct chatClient {
	int c_socket;
	char nickname[CHATDATA];
};

struct chatClient list_c[MAX_CLIENT]; // 현재 접속하고있는 클라이언트 관리 mutex 필요
char    escape[ ] = "exit"; //클라이언트 종료
char    greeting[ ] = "Welcome to chatting room\n"; //인사메세지
char    CODE200[ ] = "Sorry No More Connection\n"; //접속이 잘 안됐을 때 , 클라이언트 수 이상으로 접속되었을 경우


int main(int argc, char *argv[ ])
{
    int c_socket, s_socket;
    struct sockaddr_in s_addr, c_addr;
    int    len;
    int    i, n;
    int    res;
	 char nickname[CHATDATA];

    if(pthread_mutex_init(&mutex, NULL) != 0) { //mutex 초기화
        printf("Can not create mutex\n");
        return -1;
    }
    s_socket = socket(PF_INET, SOCK_STREAM, 0); // 서버소켓 생성

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);

    if(bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1) {
        printf("Can not Bind\n");
        return -1;
    }

    if(listen(s_socket, MAX_CLIENT) == -1) {
        printf("listen Fail\n");
        return -1;
    }

    for(i = 0; i < MAX_CLIENT; i++) //리스트 클라이언트 목록을 초기화
        list_c[i].c_socket = INVALID_SOCK;

    while(1) { //accept 대기
        len = sizeof(c_addr);
        c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);

		if((n = read(c_socket, nickname, sizeof(nickname))) < 0){
			printf("nickname read fail.\n");
			return -1;
		}

		res = pushClient(c_socket, nickname); //현재 접속한 클라이언트 정보를 list에 추가

        if(res < 0) { //MAX_CLIENT만큼 이미 클라이언트가 접속해 있다면, 마이너스 값 리턴
            write(c_socket, CODE200, strlen(CODE200));
            close(c_socket);
        } else {
			write(c_socket, greeting, strlen(greeting));
			pthread_create(&thread, NULL, do_chat, (void*)&c_socket);
           //pthread_create with do_chat function.
			//스레드 생성 인자값 : 1) 생성된 스레드의 식별자를 저장할 변수의 포인터, 2) 일반적으로 NULL, 3)void *인 함수를 가리키는 포인터
			//4)스레드에 의해 호출되는 함수에 전달하고자 하는 인자
        	}
  	  }
}

void *do_chat(void *arg) //현재 접속한 해당 클라이언트가 메시지 보내는 걸 읽음
{
    int c_socket = *((int *)arg);
    char chatData[CHATDATA];
    int i, n;
    while(1) {
        memset(chatData, 0, sizeof(chatData)); //챗데이터에 저장
        if((n = read(c_socket, chatData, sizeof(chatData))) > 0){
		
			char *token = NULL;
			char *nickname = NULL;
			char *message = NULL;

			if(strncasecmp(chatData, WHISPER, 2) == 0){
				token = strtok(chatData, DELIMETER); // \w
				nickname = strtok(NULL, DELIMETER); //nickname
				message = strtok(NULL, "\0"); 
			}
			for(i = 0; i < MAX_CLIENT; i++) {
				if(list_c[i].c_socket != INVALID_SOCK) {
					if(nickname != NULL){
						if(strcasecmp(list_c[i].nickname, nickname) == 0)
							write(list_c[i].c_socket, message, strlen(message));
					}else{
						write(list_c[i].c_socket, chatData, n);
					}
				}
			}
			if(strstr(chatData, "/exit") != NULL) { //클라이언트가 보낸 메시지가 exit이면 대화방 나가기
                popClient(c_socket); //클라이언트 목록에서 삭제
                break;
          		 }
       		 }
    	}
}

int pushClient(int c_socket, char *nickname) {
	int i;
	for(i = 0; i < MAX_CLIENT; i++) {
		if(list_c[i].c_socket == INVALID_SOCK) {
			pthread_mutex_lock(&mutex);
			list_c[i].c_socket = c_socket;
			memset(list_c[i].nickname, 0, CHATDATA);
			strcpy(list_c[i].nickname, nickname);
			pthread_mutex_unlock(&mutex);
			return i;
		}
	}
	if(i == MAX_CLIENT)
		return -1;
}	

int popClient(int c_socket)
{			
	int i;	 
	close(c_socket); 
	for(i = 0; i < MAX_CLIENT; i++) {
		if(c_socket == list_c[i].c_socket) {
			pthread_mutex_lock(&mutex);
			list_c[i].c_socket = INVALID_SOCK;
			pthread_mutex_unlock(&mutex);
			break;
		}
	}
	return 0;
}
