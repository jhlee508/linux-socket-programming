/* server.c */
#include <stdio.h>
#include <stdlib.h> // atoi 함수 사용
#include <string.h> // memset 함수 사용
#include <unistd.h> // sockaddr_in, read, write 등 사용 
#include <arpa/inet.h> // htnol, htons, INADDR_ANY, sockaddr_in 등 사용
#include <sys/socket.h> // 소켓 라이브러리

#define BUFFER_SIZE 1460 // 버퍼 크기 설정

// 에러 핸들링 함수 선언
void error_handling(char* error_message);

int main(int argc, char* argv[]) {

	int serv_sock; // server 소켓
	int clnt_sock; // client 소켓

	// sockaddr_in: 소켓 주소의 틀을 형성해주는 구조체(AF_INET일 경우 사용)
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr; // accept 함수에서 사용

	socklen_t clnt_addr_size;

	// 영상 파일 전송을 위한 변수 생성
	FILE* fp;

	char buffer[BUFFER_SIZE]; // 버퍼 생성

	int read_size = 0; // read size
	int byte_size = 0; // 파일의 크기

	char serv_msg[] = "Hello Client!\n";
	char message_from_client[30]; // 클라이언트에서 전송한 메시지
	char error_message[] = "Error!"; // 에러 발생 메시지

	if (argc != 2) { // argc: 메인 함수에 전달되는 정보의 개수
		printf("Usage: %s <port>\n", argv[0]); // argv[]: 메인 함수에 전달되는 실질적 정보(문자열의 배열)
		exit(1);
	}

	// [fopen 함수] 전송할 파일 열기
	fp = fopen("./video.mp4", "rb"); // 옵션: read binary

	// [socket 함수] 소켓 생성 
	serv_sock = socket(PF_INET, SOCK_STREAM, 0); // ipv4, TCP, protocol
	if (serv_sock == -1) {
		error_handling("socket error");
	}

	memset(&serv_addr, 0, sizeof(serv_addr)); // serv_addr 크기만큼 0으로 초기화
	serv_addr.sin_family = AF_INET; // 주소 체계
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // IPv4 주소 저장
	serv_addr.sin_port = htons(atoi(argv[1])); // Port 저장

	// [bind 함수] 소켓과 서버 주소를 바인딩, -1(에러) 이나 0(성공) 반환
	if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
		error_handling("bind error!");
	}
	
	// [listen 함수] 소켓 연결 요청 받아들일 수 있도록 함, 연결 대기열 5개로 설성
	if (listen(serv_sock, 5) == -1) {
		error_handling("listen error!");
	}

	// [accept 함수] 연결 요청 수락
	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
	if (clnt_sock == -1) { // -1(에러) 이나 0(성공) 반환
		error_handling("accept error!");
	}

	// [read 함수] Client 로부터 온 인사 메시지 출력
	if (read(clnt_sock, message_from_client, sizeof(message_from_client) - 1) == -1) {
		error_handling("read error!");
	}
	printf("Message from client: %s\n", message_from_client);

	// Client로 인사 메시지 전송
	write(clnt_sock, serv_msg, sizeof(serv_msg));

	printf("Transferring...\n");

	// 본격적으로 파일 전송!
	while (read_size = fread((void*)buffer, sizeof(char), BUFFER_SIZE, fp) == BUFFER_SIZE) {
		write(clnt_sock, buffer, BUFFER_SIZE);
		byte_size += BUFFER_SIZE;
	}
	write(clnt_sock, buffer, read_size);
	byte_size += read_size;

	// 전송 바이트 수 출력
	printf("Complete Transferring, total %d (bytes)\n", byte_size);

	// [shutdown 함수] 전송 버퍼를 닫음 
	shutdown(clnt_sock, SHUT_WR);

	// Client로부터 온 감사 메시지 확인
	char thanks_from_client[30];
	read(clnt_sock, thanks_from_client, sizeof(thanks_from_client));
	printf("Received a message from client: %s\n", thanks_from_client);

	// [fclose 함수] 파일 닫기
	fclose(fp);

	// [close 함수] 소켓을 해제함
	close(clnt_sock);
	close(serv_sock);

	return 0;
}

// 에러 핸들링 함수 정의
void error_handling(char* error_message) {
	fputs(error_message, stderr);
	fputc('\n', stderr);
	exit(1);
}