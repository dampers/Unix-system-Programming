#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORTNUM 18166
#define LENGTH 2048

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];
char redundancy[] = "The name Already exists.\nplease reconnect.\n";
char buf[LENGTH];

void print_ments()
{
    printf("             _____  _   _  _____  ______  _____  _   _  _____  _    _            \n");
    printf("            |  _  || | | ||_   _||___  / /  ___|| | | ||  _  || |  | |           \n");
    printf("            | | | || | | |  | |     / /  \\ `--. | |_| || | | || |  | |          \n");
    printf("            | | | || | | |  | |    / /    `--. \\|  _  || | | || |/\\| |         \n");
    printf("            \\ \\/' /| |_| | _| |_ ./ /___ /\\__/ /| | | |\\ \\_/ /\\  /\\  /    \n");
    printf("             \\_/\\_\\ \\___/  \\___/ \\_____/ \\____/ \\_| |_/ \\___/  \\/  \\/ \n");

    printf("+--------------------------  WELLCOME TO QUIZ SHOW ----------------------------+\n");
    printf("|---------------------------------- CLIENT ------------------------------------|\n\n");
    printf("|-------------  QUIZ SHOW가 시작되면 채팅은 답안으로 처리됩니다.  -------------|\n\n");
    printf("|------------------- SERVER는 채팅에 참여할 수 없습니다. ----------------------|\n\n");
    printf("|---- ';;list' 명령어로 현재 CLIENTS의 점수와 순위를 확인할 수 있습니다. ------|\n\n");
    printf("|--------- ';;info' 명령어로 현재 자신의 통계정보를 알 수 있습니다.------------|\n\n");
    printf("|----------- ';;help' 명령어로 위 내용을 다시 확인할 수 있습니다. -------------|\n\n");
    printf("|--------------- 첫번째로 맞춘 사람만 점수를 획득할 수 있습니다.---------------|\n\n");
    printf("|-다수의 오답 후 정답을 맞출 시 오답을 외친 횟수만큼 차감된 점수를 획득합니다.-|\n\n");
    printf("|-----문제가 출제된 후 15초 안에 정답을 입력해야 점수를 획득할 수 있습니다.----|\n\n");
    printf("|------- 15초가 지나면 다음문제가 출제될 때까지 전체채팅이 가능합니다. --------|\n\n");
    printf("+------------------------------------------------------------------------------+\n\n");
}

void str_overwrite_stdout()
{
	printf("\r");
	fflush(stdout);
}

void str_trim_lf (char* arr, int length)
{
	int i;
	for(i=0;i<length;i++)
	{ // trim \n
		if(arr[i] == '\n')
		{
			arr[i] = '\0';
			break;
		}
	}
}

void catch_ctrl_c_and_exit(int sig)
{
	flag = 1;
}

void send_msg_handler()
{
	char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {};

	while(1)
	{
		str_overwrite_stdout();
		fgets(message, LENGTH, stdin);
		str_trim_lf(message, LENGTH);

		if(strcmp(message, "exit") == 0)
		{
			break;
		}
		else
		{
			sprintf(buffer, "%s", message);
			send(sockfd, buffer, strlen(buffer), 0);
		}

		bzero(message, LENGTH);
		bzero(buffer, LENGTH + 32);
	}
	catch_ctrl_c_and_exit(2);
}

void recv_msg_handler()
{
	char message[LENGTH] = {};
	while(1)
	{
		int receive = recv(sockfd, message, LENGTH, 0);
		if(receive>0)
		{
			printf("%s", message);
			str_overwrite_stdout();
		}
		else if(receive == 0)
		{
			break;
    	}
    	else
    	{
			// -1
		}
		memset(message, 0, sizeof(message));
	}
}

int main(int ac, char *av[])
{
	if(ac != 2)
	{
		printf("Usage: %s <ip>\n", av[0]);
		return EXIT_FAILURE;
	}
	int port = PORTNUM;

	signal(SIGINT, catch_ctrl_c_and_exit);

	printf("Please enter your name: ");
	fgets(name, 32, stdin);
	str_trim_lf(name, strlen(name));


	if(strlen(name) > 32 || strlen(name) < 2)
	{
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	/* Socket settings */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(av[1]);
	server_addr.sin_port = htons(port);


  // Connect to Server
	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(err == -1)
	{
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(sockfd, name, 32, 0);
    int rec = (sockfd, buf, LENGTH, 0);
    if(strcmp(redundancy, buf)==0)
    {
        printf("%s", redundancy);
        return EXIT_FAILURE;
    }
	
    print_ments();

	pthread_t send_msg_thread;
	if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0)
	{
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
	if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0)
	{
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1)
	{
		if(flag)
		{
			printf("\nBye\n");
			break;
		}
	}

	close(sockfd);

	return EXIT_SUCCESS;
}

