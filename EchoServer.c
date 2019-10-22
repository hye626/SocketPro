//에코서버 서버

#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

#define PORT 10000
#define BUFSIZE 10000

char buffer[BUFSIZE] = "Hello World.\n";
char rcvBuffer[BUFSIZE];

int main(){
	int c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	int len;
	int n;

	// 1. 서버 소켓 생성
	s_socket = socket(PF_INET, SOCK_STREAM, 0);

	//2. 서버 소켓 주소 설정
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);

	//3. 서버 소켓바인딩
	if(bind(s_socket,(struct sockaddr *) &s_addr, sizeof(s_addr)) == -1){
		printf("Cannot Bind\n");
		return -1;
	}

	//4.listen() 함수 실행
	if(listen(s_socket, 5) == -1){
		printf("listen Fail\n");
		return -1;
	}

	//5. 클라이언트 요청 처리
	while(1){
		char *token;
		char *str[3];
		int i = 0;

		len = sizeof(c_addr);
		printf("클라이언트 접속을 기다리는 중....\n");
		c_socket = accept(s_socket, (struct sockaddr *)&c_addr, &len);
		printf("/client is connected\n");
		printf("클라이언트 접속 허용\n");

		while(1) {
			n = read(c_socket, rcvBuffer, sizeof(rcvBuffer));
			printf("메시지 받음 : %s\n", rcvBuffer);
			rcvBuffer[n] = '\0'; //개행 문자 삭제

			if(strncasecmp(rcvBuffer, "quit", 4) == 0 || strncasecmp(rcvBuffer, "kill server", 11) == 0)
					break;
			else if(!strncmp(rcvBuffer, "안녕하세요", strlen("안녕하세요")))
				strcpy(buffer, "안녕하세요. 만나서 반가워요.");
			else if(!strncmp(rcvBuffer, "이름이 머야?", strlen("이름이 머야?")))
					strcpy(buffer, "내 이름은 강혜경이야.");
			else if(!strncmp(rcvBuffer, "몇 살이야?", strlen("몇 살이야?")))
					strcpy(buffer, "나는 22살이야.");
			else if(!strncasecmp(rcvBuffer, "strlen ", 7))
					sprintf(buffer, "문자열의 길이 : %d", strlen(rcvBuffer)-7);
			else if(!strncasecmp(rcvBuffer, "strcmp ", 7)){
					i = 0;
					token = strtok(rcvBuffer, " ");
				while(token != NULL){
						str[i++] = token;
						token = strtok(NULL, " ");
				}
				if(i<3)
					sprintf(buffer, "문자열 비교를 위해서는 두 문자열이 필요합니다.");
				else if(!strcmp(str[1], str[2]))
					sprintf(buffer, "%s %s : 같은 문자열 입니다.", str[1], str[2]);
				else
					sprintf(buffer, "%s %s : 다른 문자열 입니다.", str[1], str[2]);
				}
			else if(!strncasecmp(rcvBuffer, "readfile ", 9)) {
				i=0;
				token = strtok(rcvBuffer, " ");
				while(token != NULL){
					str[i++] = token;
					token = strtok(NULL, " ");
				}
				//str[0] = readfile
				//str[1] = filename
				if(i<2)
					sprintf(buffer, "파일명을 입력해주세요.");
				else{
					FILE *fp = fopen(str[1], "r");
					if(fp){
						char tempStr[BUFSIZE]; //파일내용을 저장할 변수
						memset(buffer, 0, BUFSIZE); //buffer 초기화
						while(fgets(tempStr, BUFSIZE, (FILE *)fp)){
							strcat(buffer, tempStr); //여러 줄의 내용을 하나의 buffer에 저장하기 위해 strcat() 함수
						}
						fclose(fp);
					}else{
						sprintf(buffer, "해당 파일은 존재하지 않습니다.");
					}
				}
			}
			else
				strcpy(buffer, "무슨 말인지 모르겠습니다.");
		
			n = strlen(buffer);
			write(c_socket, buffer, n);
			}
			close(c_socket);
			if (strncasecmp(rcvBuffer, "kill server", 11) == 0)
				break;
			}
	close(s_socket);
	return 0;	
}
