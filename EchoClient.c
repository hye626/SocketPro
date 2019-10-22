//에코서버 클라이언트 예제 6-1

#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

#define PORT 10000
#define IPADDR "127.0.0.1"
#define BUFSIZE 10000

int main(){
	int c_socket; 
	struct sockaddr_in c_addr;
	int n;
	char message[BUFSIZE];

	//1. 클라이언트 소켓 생성
	c_socket = socket(PF_INET, SOCK_STREAM, 0);

	//2.소켓 정보 초기화
	memset(&c_addr, 0, sizeof(c_addr));
	c_addr.sin_addr.s_addr = inet_addr(IPADDR);
	c_addr.sin_family = AF_INET;
	c_addr.sin_port = htons(PORT);

	//3. 서버에 접속
	if(connect(c_socket, (struct sockaddr *) &c_addr, sizeof(c_addr)) == -1){ 
		printf("Cannot Connect\n");
		close(c_socket);
		return -1;
	}

	//4. 서버에서 보낸 메시지 읽기 
	while(1){
		fputs("메시지 입력 : ", stdout);
		fgets(message,sizeof(message), stdin);
		message[strlen(message)-1] = '\0';
		write(c_socket, message, strlen(message));
		if(strncasecmp(message, "quit", 4) == 0 || strncasecmp(message, "kill server", 11) == 0)
			break;

		n = read(c_socket, message, sizeof(message)); 
		if (n < 0){
			printf("Read Failed\n");
			return -1;
		}

		message[n] = '\0'; //문자열 뒷부분 깨짐 방지
		printf("메시지 받음: %s\n", message); //서버에서 받은 메세지 출력
	}
	close(c_socket);
	return 0;	
}
