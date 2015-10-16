#include<curses.h>
#include<time.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdio_ext.h>
#include<string.h>
#include<sys/shm.h>

#define SIZE 2048

struct user
{
	int state;
	/*
	State of preparedness:
		~  ->	No connection
		19 ->	Connected. Not ready.
		23 ->	Connected. Ready.
		1  ->	Message Broadcasted
		2  ->	Message Recieved
		3  ->	Quit
	*/
	char msg[SIZE], usrname[16];
};


int main()
{
	void *shm[2];
	struct user *usr[2];
	int me, you, shmid[2],  y=1, /*end=1,*/ j, name, ch, prex, prey;
	char buf[SIZE];
	
	shm[0]=shm[1]=(void *)0;
	usr[0]=usr[1]=(struct user *)NULL;
	name=-1;
	
	shmid[0]=shmget((key_t)1234, sizeof(struct user), 0666|IPC_CREAT);
	shmid[1]=shmget((key_t)4321, sizeof(struct user), 0666|IPC_CREAT);
	
	if(shmid[0]==-1||shmid[1]==-1)
	{
		fprintf(stderr, "\nshmget() failed!!!\n");
		return 0;
	}

	shm[0]=shmat(shmid[0], (void *)0, 0);
	shm[1]=shmat(shmid[1], (void *)0, 0);

	if(shm[0]==(void *)-1||shm[1]==(void *)-1)
	{
		fprintf(stderr, "\nshmat() failed!!!\n");
		return 0;
	}

	printf("\nMemories attached at %lu\t&\t%lu\n", (unsigned long)shm[0], (unsigned long)shm[1]);

	usr[0]=(struct user*)shm[0];

	if(usr[0]->state==19||usr[0]->state==23)
	{
		me=1;
		you=0;
		usr[you]=(struct user*)shm[you];
		usr[me]=(struct user*)shm[me];
		usr[me]->state=19;
		printf("\nYou were the second user to connect\n");
	}

	else
	{
		me=0;
		you=1;
		usr[you]=(struct user*)shm[you];
		usr[me]=(struct user*)shm[me];
		usr[me]->state=19;
		printf("\nYou are the first user to connect\n");
	}

	printf("\nEnter your username: ");
	__fpurge(stdin);
	scanf("%s", buf);
	strncpy(usr[me]->usrname, buf, 15);
	usr[me]->state=23;

	if(usr[you]->state!=19&&usr[you]->state!=23)
	{
		printf("\nWaiting for the other user to connect...");
		while(usr[you]->state!=19)
		{
			sleep(1);
			printf(".");
		}
		printf("\nThe other user connected!!!\n");
	}

	if(usr[you]->state!=23)
	{
		printf("\nWaiting for the other user to log in...");
		while(usr[you]->state!=23)
		{
			sleep(1);
			printf(".");
		}
		printf("\nThe other user logged in!!!\n");
	}

	WINDOW *chat, *txtbox/*, *status*/;
	time_t now;
	initscr();
	clear();
	chat=newwin(LINES-4, COLS, 0, 0);
	txtbox=newwin(3, COLS, LINES-3, 0);
/****************************************************************************************************
	status=newwin(1, COLS, LINES-4, 0);
****************************************************************************************************/
	box(chat, 0, 0);
	box(txtbox, 0, 0);
	halfdelay(1);
	do
	{
		wattron(txtbox, A_BOLD);
		mvwprintw(txtbox, 1, 1, "%s: ", usr[me]->usrname);
		wattroff(txtbox, A_BOLD);
		wrefresh(chat);
		wrefresh(txtbox);
		wmove(txtbox, 1, strlen(usr[me]->usrname)+3);
		j=0;
		while(1)
		{
			box(txtbox, 0, 0);
			box(chat, 0, 0);
			ch=wgetch(txtbox);
			if(usr[you]->state==20)
			{
				mvwprintw(chat, LINES-6, 2, "%s is typing...", usr[you]->usrname);
				wrefresh(chat);
			}
			else if(usr[you]->state!=20)
			{
				wmove(chat, LINES-6, 0);
				wclrtoeol(chat);
			}
			if(ch=='\n')
				break;
			if(ch==127&&j!=0)
			{
				j--;
				if(j==0)
					usr[me]->state=2;
				getyx(txtbox, prey, prex);
				if(prex>(strlen(usr[me]->usrname)+2))
				{
					mvwdelch(txtbox, prey, prex-3);
					mvwdelch(txtbox, prey, prex-3);
					mvwdelch(txtbox, prey, prex-3);
					wclrtoeol(txtbox);
				}
			}

			else if(ch==127&&j==0)
			{
				usr[me]->state=2;
				getyx(txtbox, prey, prex);
				mvwdelch(txtbox, prey, prex-2);
				mvwdelch(txtbox, prey, prex-2);
				wclrtoeol(txtbox);
			}

			if((ch!=-1)&&(ch!=127))
				buf[j++]=ch;
			if(usr[you]->state==3)
				break;
			if(usr[you]->state==1)
			{
				time(&now);
				wattron(chat, A_BOLD);
				if(name!=you)
				{
					mvwaddstr(chat, y, 2, usr[you]->usrname);
					mvwprintw(chat, y++, COLS-26, "%s", ctime(&now));
				}
				wattroff(chat, A_BOLD);
				mvwprintw(chat, y++, 2, "%s", usr[you]->msg);
				box(chat, 0, 0);
				wrefresh(chat);
				wrefresh(txtbox);
				usr[you]->state=2;
				name=you;
			}
			if(y>LINES-6)
			{
				wclear(chat);
				box(chat, 0, 0);
				wrefresh(chat);
				y=1;
			}
			if(j>0)
				usr[me]->state=20;
/*
			else if(j==0)
				usr[me]->state=2;
*/
/****************************************************************************************************
			mvwprintw(status, 1, 5, "Test area. Blank area.");
*****************************************************************************************************/
		}
		buf[j]='\0';
		if(j>0)
		{
			strcpy(usr[me]->msg, buf);
			time(&now);
			wattron(chat, A_BOLD);
			if(name!=me)
			{
				mvwaddstr(chat, y, 2, usr[me]->usrname);
				mvwprintw(chat, y++, COLS-26, "%s", ctime(&now));
			}
			wattroff(chat, A_BOLD);
			mvwaddstr(chat, y++, 2, usr[me]->msg);
			box(chat, 0, 0);
			wmove(txtbox, 1, 3);
			wclrtoeol(txtbox);
			usr[me]->state=1;
			name=me;
		}
	}while(strcasecmp("end", buf)&&(usr[you]->state!=3));
	if((usr[you]->state!=3))
		usr[me]->state=3;
	endwin();

	if((usr[me]->state==3))
	{
		if((shmdt(shm[0])==-1)||(shmdt(shm[1])==-1))
		{
			fprintf(stderr, "shmdt() failed!!!\n");
			return 0;
		}
		
		if((shmctl(shmid[0], IPC_RMID, 0)==-1)||(shmctl(shmid[1], IPC_RMID, 0)==-1))
		{
			fprintf(stderr, "shmctl() failed!!!\n");
			return 0;
		}
	}

	printf("\nChat successfully ended\n");

	return 0;
}
