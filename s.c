#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUFFER_SZ 2048
#define PORTNUM 18166

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

/* Client structure */
typedef struct
{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	int point;
    int hint;
    int answer;
    int trial;
    int correcto;
    float rate;
	char name[32];
    struct tm utm;
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;

int quiz_start;
char *start = ";;start";
char *list = ";;list";
char *help = ";;help";
char *help_list = "+--------------------------  WELLCOME TO QUIZ SHOW ---------------------------+\n|---------------------------------- SERVER -----------------------------------|\n\n|------------- QUIZ SHOW가 시작되면 SERVER는 답을 알 수 있습니다.  -----------|\n\n|------------------- SERVER는 채팅에 참여할 수 없습니다. ---------------------|\n\n|------- ';;list' 명령어로 현재 CLIENTS의 정보를 확인할 수 있습니다.  --------|\n\n|----------- ';;start' 명령어로 QUIZ SHOW를 진행할 수 있습니다. --------------|\n\n|----------- ';;help' 명령어로 위 내용을 다시 확인할 수 있습니다. ------------|\n\n+-----------------------------------------------------------------------------+\n\n";
char *myinfo = ";;info";
char problem_set[BUFFER_SZ];
char problem_buf[15][BUFFER_SZ];
char answer_buf[BUFFER_SZ];
char correct_buf[15][BUFFER_SZ];
int correct;
int pa, ca;
int pc;
int check[15];
char *chelp = "+--------------------------  WELLCOME TO QUIZ SHOW ----------------------------+\n|---------------------------------- CLIENT ------------------------------------|\n\n|-------------  QUIZ SHOW가 시작되면 채팅은 답안으로 처리됩니다.  -------------|\n\n|------------------- SERVER는 채팅에 참여할 수 없습니다. ----------------------|\n\n|---- ';;list' 명령어로 현재 CLIENTS의 점수와 순위를 확인할 수 있습니다. ------|\n\n|--------- ';;info' 명령어로 현재 자신의 통계정보를 알 수 있습니다.------------|\n\n|----------- ';;help' 명령어로 위 내용을 다시 확인할 수 있습니다. -------------|\n\n|--------------- 첫번째로 맞춘 사람만 점수를 획득할 수 있습니다.---------------|\n\n|-다수의 오답 후 정답을 맞출 시 오답을 외친 횟수만큼 차감된 점수를 획득합니다.-|\n\n|-----문제가 출제된 후 15초 안에 정답을 입력해야 점수를 획득할 수 있습니다.----|\n\n|------- 15초가 지나면 다음문제가 출제될 때까지 전체채팅이 가능합니다. --------|\n\n+------------------------------------------------------------------------------+\n\n";

void print_ments()
{
    printf("             _____  _   _  _____  ______  _____  _   _  _____  _    _            \n");
    printf("            |  _  || | | ||_   _||___  / /  ___|| | | ||  _  || |  | |           \n");
    printf("            | | | || | | |  | |     / /  \\ `--. | |_| || | | || |  | |          \n");
    printf("            | | | || | | |  | |    / /    `--. \\|  _  || | | || |/\\| |         \n");
    printf("            \\ \\/' /| |_| | _| |_ ./ /___ /\\__/ /| | | |\\ \\_/ /\\  /\\  /    \n");
    printf("             \\_/\\_\\ \\___/  \\___/ \\_____/ \\____/ \\_| |_/ \\___/  \\/  \\/ \n");


    printf("+--------------------------  WELLCOME TO QUIZ SHOW ---------------------------+\n");
    printf("|---------------------------------- SERVER -----------------------------------|\n\n");
    printf("|------------- QUIZ SHOW가 시작되면 SERVER는 답을 알 수 있습니다.  -----------|\n\n");
    printf("|------------------- SERVER는 채팅에 참여할 수 없습니다. ---------------------|\n\n");
    printf("|------- ';;list' 명령어로 현재 CLIENTS의 정보를 확인할 수 있습니다.  --------|\n\n");
    printf("|----------- ';;start' 명령어로 QUIZ SHOW를 진행할 수 있습니다. --------------|\n\n");
    printf("|----------- ';;help' 명령어로 위 내용을 다시 확인할 수 있습니다. ------------|\n\n");
    printf("+-----------------------------------------------------------------------------+\n\n");
}


void str_overwrite_stdout()
{
	printf("\r");
	fflush(stdout);
}

void str_trim_lf (char* arr, int length)
{
	int i;
	for (i=0;i<length;i++)
	{ // trim \n
		if (arr[i] == '\n')
		{
			arr[i] = '\0';
			break;
    	}
	}
}

void print_client_addr(struct sockaddr_in addr)
{
	printf("%d.%d.%d.%d",
		addr.sin_addr.s_addr & 0xff,
		(addr.sin_addr.s_addr & 0xff00) >> 8,
		(addr.sin_addr.s_addr & 0xff0000) >> 16,
		(addr.sin_addr.s_addr & 0xff000000) >> 24);
}

/* Add clients to queue */
void queue_add(client_t *cl)
{
	pthread_mutex_lock(&clients_mutex);
	int i;
	for(i=0;i<MAX_CLIENTS;i++)
	{
		if(!clients[i])
		{
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients to queue */
void queue_remove(int uid)
{
	pthread_mutex_lock(&clients_mutex);
	int i;
	for(i=0;i<MAX_CLIENTS;i++)
	{
		if(clients[i])
		{
			if(clients[i]->uid == uid)
			{
                strcpy(clients[i]->name, "\0");
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Send message to all clients except sender */
void send_message(char *s, int uid)
{
	pthread_mutex_lock(&clients_mutex);
	int i;
	for(i=0;i<MAX_CLIENTS;i++)
	{
		if(clients[i])
		{
			if(clients[i]->uid != uid)
			{
				if(write(clients[i]->sockfd, s, strlen(s)) < 0)
				{
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void send_pointlist(int uid, int uidsockfd)
{    
    char sbuf[BUFFER_SZ];
    int clients_list[MAX_CLIENTS];
    int i, clients_num = 0, j, mnind, tmp;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i])
        {
            clients_list[clients_num++] = i;
        }
    }

    //sort
    for(i=0;i<clients_num;i++)
    {
        mnind = i;
        for(j=i+1;j<clients_num;j++)
        {
            if(clients[clients_list[mnind]]->point<clients[clients_list[j]]->point)
            {
                mnind = j;
            }
        }
        tmp = clients_list[i];
        clients_list[i] = clients_list[mnind];
        clients_list[mnind] = tmp;
    }
    // send
    sprintf(sbuf, "\t\tname\t\tpoints\n\n");
    if(write(uidsockfd, sbuf, strlen(sbuf))<0)
    {
        perror("!ERROR: write to descriptor failed");
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    bzero(sbuf, BUFFER_SZ);
    for(i=0;i<clients_num;i++)
    {
        sprintf(sbuf, "%d\t\t%s\t\t%d\n", i+1, clients[clients_list[i]]->name, clients[clients_list[i]]->point);
        if(write(uidsockfd, sbuf, strlen(sbuf))<0)
        {
            perror("ERROR: write to descriptor failed");
            break;
        }
        bzero(sbuf, BUFFER_SZ);
    }
    write(uidsockfd, "\n\n", 2);
}

void locked_send_message(char *s, int uid)
{
   int i;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i])
        {
            if(clients[i]->uid != uid)
            {
                if(write(clients[i]->sockfd, s, strlen(s)) < 0)
                {
                    perror("ERROR: write to descriptor failed.");
                    break;
                }
            }
        }
    } 
}

void my_info(int uid)
{
    pthread_mutex_lock(&clients_mutex);
    
    int i, clinum;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i]->uid==uid)
        {
            clinum = i;
            break;
        }
    }
    char info[BUFFER_SZ];
    sprintf(info, "name : %s\n", clients[clinum]->name);
    if((write(clients[clinum]->sockfd, info, strlen(info))<0))
    {
        perror("EROOR: write to descriptor failed.");
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    sprintf(info, "point : %d\n", clients[clinum]->point);
    if((write(clients[clinum]->sockfd, info, strlen(info))<0))
    {
        perror("EROOR: write to descriptor failed.");
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    sprintf(info, "trial : %d\n", clients[clinum]->trial);
    if((write(clients[clinum]->sockfd, info, strlen(info))<0))
    {
        perror("EROOR: write to descriptor failed.");
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    sprintf(info, "correct : %d\n", clients[clinum]->correcto);
    if((write(clients[clinum]->sockfd, info, strlen(info))<0))
    {
        perror("EROOR: write to descriptor failed.");
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    sprintf(info, "rate : %f\n", clients[clinum]->rate);
    if((write(clients[clinum]->sockfd, info, strlen(info))<0))
    {
        perror("EROOR: write to descriptor failed.");
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    struct tm ktm = clients[clinum]->utm;
    sprintf(info, "%d-%d-%d %d:%d:%d\n",ktm.tm_year+1900, ktm.tm_mon+1, ktm.tm_mday, ktm.tm_hour, ktm.tm_min, ktm.tm_sec);
    if((write(clients[clinum]->sockfd, info, strlen(info))<0))
    {
        perror("EROOR: write to descriptor failed.");
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

void rate_set()
{
    pthread_mutex_lock(&clients_mutex);

    int i;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i])
        {
            clients[i]->trial += clients[i]->answer;
            if(clients[i]->trial == 0)
            {
                clients[i]->rate = 0.0;
                continue;
            }
            clients[i]-> rate = ((float)(clients[i]->correcto))/((float)(clients[i]->trial));
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void make_answer_reset()
{
    pthread_mutex_lock(&clients_mutex);

    int i;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i])
        {
            clients[i]->answer = 1;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}


/* Handle all communication with the client */
void *handle_client(void *arg)
{
	char buff_out[BUFFER_SZ];
    char sbuf[BUFFER_SZ];
	char name[32];
	int leave_flag = 0;
    int correct_flag = 0;
    int i, j;


	cli_count++;
	client_t *cli = (client_t *)arg;

	// Name
	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1)
	{
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	}
	else
	{
        //name redundancy check
        for(i=0;i<MAX_CLIENTS;i++)
        {
            if(clients[i])
            {
                if(strcmp(clients[i]->name, name)==0)
                {
                    write(cli->sockfd, name, strlen(name));
                    printf("clients[%d]->name = %s\n%s\n", i, clients[i]->name, name);
                    if(write(cli->sockfd, "\n[SERVER]The name Already exists. please reconnect.\n", 52) < 0)
                    {
                        perror("ERROR: write to descriptor failed");
                        leave_flag = 1;
                        break;
                    }
                    leave_flag = 1;
                    break;
                }
            }
        }
        if(leave_flag == 0)
        {
		    strcpy(cli->name, name);
		    sprintf(buff_out, "[SERVER] %s has joined\n", cli->name);
		    printf("%s", buff_out);
		    send_message(buff_out, cli->uid);
        }
	}

	bzero(buff_out, BUFFER_SZ);

	while(1)
	{
		if (leave_flag)
		{
			break;
		}

		int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
		if(quiz_start == 1)
        {
            if(cli->answer==11 && receive > 0)
            {
                sprintf(buff_out, "[SERVER] %s, your answer counter is ALEADY 10.\n", cli->name);
                send_message(buff_out, 1);
            }
            else if(receive > 0)
            {
                if(strcmp(help, buff_out)==0)
                {
                    if(write(cli->sockfd, chelp, strlen(chelp))<0)
                    {
                        perror("write error");
                    }
                    continue;
                }
                pthread_mutex_lock(&clients_mutex);
                
                sprintf(sbuf, "%s\n", buff_out);
                if(strlen(buff_out)>0)
                {
                    strcpy(buff_out, sbuf);
                    strcpy(answer_buf, buff_out);
                    printf("\n%s\'s answer %d : %s\n", cli->name, cli->answer, answer_buf);
                    sprintf(buff_out, "[SERVER] %s\'s answer %d : %s\n", cli->name, cli->answer, answer_buf);
                    
                    //send message
                    locked_send_message(buff_out, 1);

                    //correct answer
                    //printf("%d\n", strcmp(answer_buf, correct_buf[pc]));
                    if(strcmp(answer_buf, correct_buf[pc])==0)
                    {
                        sprintf(buff_out, "[SERVER] %s\'s answer %d is correct!\n", cli->name, cli->answer);
                        locked_send_message(buff_out, 1);
                        printf("%s", buff_out);
                        correct = 1;
                        cli->point += 10-(cli->answer)+1;
                        bzero(buff_out, BUFFER_SZ);
                        sprintf(buff_out, "[SERVER] %s\'s get +%d points!\n\n", cli->name, 10-(cli->answer)+1);
                        locked_send_message(buff_out, 1);
                        printf("%s", buff_out);
                        cli->correcto += 1;
                        //locked_send_pointlist(cli->uid, cli->sockfd);
                        sleep(1);
                        send_pointlist(0, 1);
                        for(j=0;j<MAX_CLIENTS;j++)
                        {
                            if(clients[j])
                            {
                                send_pointlist(clients[j]->uid, clients[j]->sockfd);
                            }
                        }
                    }
                    //wrong answer
                    else
                    {
                        sprintf(buff_out, "[SERVER] %s\'s answer %d is wrong!\n", cli->name, cli->answer);
                        locked_send_message(buff_out, 1);
                        printf("%s", buff_out);
                        cli->answer++;
                    }
                }

                pthread_mutex_unlock(&clients_mutex);
            }
            else if (receive == 0 || strcmp(buff_out, "exit") == 0)
            {
                sprintf(buff_out, "[SERVER] %s has left\n", cli->name);
                printf("%s", buff_out);
                send_message(buff_out, cli->uid);
                leave_flag = 1;
            }
            else
            {
                printf("ERROR: -1\n");
                leave_flag = 1;
            }
        }
        else
        {
            if (receive > 0)
		    {
			    if(strlen(buff_out) > 0)
			    {
                    if(strcmp(list, buff_out)==0)
                    {
                        send_pointlist(cli->uid, cli->sockfd);
                    }
                    else if(strcmp(myinfo, buff_out)==0)
                    {
                        my_info(cli->uid);
                    }
                    else if(strcmp(help, buff_out)==0)
                    {
                        if(write(cli->sockfd, chelp, strlen(chelp))<0)
                        {
                            perror("write error");
                        }
                        continue;
                    }
                    else
                    {
                        sprintf(sbuf, "%s : %s\n", cli->name, buff_out);
                        send_message(sbuf, cli->uid);
				        str_trim_lf(sbuf, strlen(sbuf));
				        printf("%s\n", sbuf);     
                    }
                }
		    }
		    else if (receive == 0 || strcmp(buff_out, "exit") == 0)
		    {
			    sprintf(buff_out, "[SERVER] %s has left\n", cli->name);
			    printf("%s", buff_out);
			    send_message(buff_out, cli->uid);
			    leave_flag = 1;
		    }
		    else
		    {
			    printf("ERROR: -1\n");
			    leave_flag = 1;
		    }
        }

        bzero(buff_out, BUFFER_SZ);
        bzero(sbuf, BUFFER_SZ);
	}

  /* Delete client from queue and yield thread */
	close(cli->sockfd);
	queue_remove(cli->uid);
 	free(cli);
 	cli_count--;
 	pthread_detach(pthread_self());

	return NULL;
}

void *command_input()
{
    char command_buf[BUFFER_SZ];
    char hint_buf[15][BUFFER_SZ];
    char sbuf[BUFFER_SZ];
    char count_down[3] = "0\n\0";
    int n, fd, i, j;

    while(1)
    {
        fflush(stdin);
        scanf(" %s", command_buf);
        printf("input command = %s\n", command_buf);
        // quiz show start!!!
        if(strcmp(command_buf, start)==0)
        {
            pthread_mutex_lock(&server_mutex);
            if(cli_count==0)
            {
                printf("The number of clients is 0.\n You can\'t start quiz show!\n");
                continue;
            }
            //fd = open("problem_set.txt", O_RDONLY);
            /*
            if(fd == -1)
            {
                perror("problem_set open error.\n");
                exit(1);
            }
            */
            send_message("[SERVER] THE QUIZ IS STARTING NOW!!\n", 1);
            printf("[SERVER] THE QUIZ IS STARTING NOW!!\n");
            // quiz show 
            memset(check, 0, sizeof(check));
            for(i=0;i<10;i++)
            {
                pc = rand()%13;
                if(check[pc] == 1)
                {
                    i--;
                    continue;
                }
                check[pc] = 1;
                correct = 0;
                if(cli_count==0)
                {
                    printf("The number of clients is 0.\n You can\'t start quiz show!\n");
                    break;
                }

                send_message("[SERVER] ARE YOU READY?\n", 1);
                sleep(1);
                make_answer_reset();
                
                for(j=3;j>0;j--)
                {
                    sleep(1);
                    count_down[0] = '0'+j;
                    printf("%s", count_down);
                    send_message(count_down, 1);
                }

                sprintf(sbuf, "[SERVER] problem %d : %s", i+1,  problem_buf[pc]);
                send_message(sbuf, 1);
                printf("%s", sbuf);
                printf("the correct answer : %s\n", correct_buf[pc]);
                quiz_start = 1;
                for(j=0;j<15;j++)
                {
                    if(correct==1)
                    {
                        quiz_start = 0;
                        printf("problem %d correct!\n", i+1);
                        break;
                    }
                    sleep(1);
                }
                quiz_start = 0;
                if(correct==0)
                {
                    printf("[SERVER] The answer is %s\n", correct_buf[pc]);
                    sprintf(answer_buf, "[SERVER] The answer is %s\n", correct_buf[pc]);
                    send_message(answer_buf, 1);
                }
                //sleep(1);
                //send_pointlist(0, 1);
                /*for(j=0;j<MAX_CLIENTS;j++)
                {
                    if(clients[j])
                    {
                        send_pointlist(clients[j]->uid, clients[j]->sockfd);
                    }
                }*/

                rate_set();

                sleep(1);
            }
            pthread_mutex_unlock(&server_mutex);
            // the quiz show is over.
            quiz_start = 0;
        }
        else if(strcmp(list, command_buf)==0)
        {
            send_pointlist(0, 1);
        }
        else if(strcmp(help, command_buf)==0)
        {
            printf("%s", help_list);
        }
        else
        {
            printf("command error\n");
        }
        sleep(1);
        bzero(command_buf, BUFFER_SZ);
    }
    pthread_detach(pthread_self());
}


int main(int ac, char *av[])
{
	if(ac != 2)
	{
		printf("Usage: %s <ip>\n", av[0]);
		return EXIT_FAILURE;
	}
	int port = PORTNUM;
	int option = 1;
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid, stid;
    time_t t;
    int fd;
    fd = open("problem.txt", O_RDONLY);
    if(fd==-1)
    {
        perror("problem read error.\n");
    }
    int rn = read(fd, problem_set, BUFFER_SZ);
    int len = strlen(problem_set);
    int i, j, nflag = 0, pind = 0, cind = 0;
    for(i=0;i<len;i++)
    {
        if(nflag==0 && problem_set[i]!='\n')
        {
            problem_buf[pa][pind++] = problem_set[i];
        }
        else if(nflag==0 && problem_set[i]=='\n')
        {
            problem_buf[pa][pind++] = '\n';
            problem_buf[pa][pind] = '\0';
            pa++;
            pind = 0;
            nflag = 1;
        }
        else if(nflag==1 && problem_set[i]!='\n')
        {
            correct_buf[ca][cind++] = problem_set[i];
        }
        else if(nflag==1 && problem_set[i]=='\n')
        {
            correct_buf[ca][cind++] = '\n';
            correct_buf[ca][cind] = '\0';
            ca++;
            cind = 0;
            nflag = 0;
        }
    }
    
    /*
    for(i=0;i<pa;i++)
    {
        printf("%s\n", problem_buf[i]);
        printf("%s\n", correct_buf[i]);
    }
    */

  /* Socket settings */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(av[1]);
	serv_addr.sin_port = htons(port);

  /* Ignore pipe signals */
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
	{
		perror("ERROR: setsockopt failed");
    	return EXIT_FAILURE;
	}

	/* Bind */
 	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
 	{
		perror("ERROR: Socket binding failed");
		return EXIT_FAILURE;
	}

  /* Listen */
 	if(listen(listenfd, 10) < 0)
 	{
		perror("ERROR: Socket listening failed");
		return EXIT_FAILURE;
	}

	print_ments();
    srand((unsigned int)time(NULL));

    pthread_create(&stid, NULL, &command_input, NULL);
	while(1)
	{
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

		/* Check if max clients is reached */
		if((cli_count + 1) == MAX_CLIENTS)
		{
			printf("Max clients reached. Rejected: ");
			print_client_addr(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;
		cli->point = 0;
        cli->hint = 1;
        cli->answer = 1;
        cli->trial = 0;
        cli->correcto = 0;
        cli->rate = 0.0;
        t = time(NULL);
        cli->utm = *localtime(&t);

		/* Add client to the queue and fork thread */
		queue_add(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);
		/* Reduce CPU usage */
		sleep(1);
	}

	return EXIT_SUCCESS;
}
