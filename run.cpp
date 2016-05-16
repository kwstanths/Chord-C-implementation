#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <functional>
#include <algorithm>
#include <thread>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <utility>
#include <fstream>
#include <netdb.h>
#include <sys/time.h>

#include "lib.h"
#include "defines.h"

using namespace std;

/*
* The run executable reads files query.txt and inserts.txt and sends these requests to the nodes
* ATTENTION: ports table holds the ports at a 4096 ring size i.e ./chord_emu nodes 12
*	           and ports1 holds the ports at a 8192 ring size i.e ./chord_emu nodes 13
*/


int find(int* ports, int length, int port){
	for (int i = 0; i < length; i++) {
		if (ports[i] == port) return i;
	}
}

void execute_inserts(pair<char *,int> * inserts, int inserts_number, int * ports, int ports_length){

	struct timeval tts,ttf;
	double ttotal;
	char message[300];
	int sum[] ={0,0,0,0,0,0,0,0,0,0};
	char server_port[6];
	int sd,i,port,rd;
	struct hostent *hp;
	struct sockaddr_in sa;

	cout << inserts_number << endl;
	gettimeofday(&tts,NULL);
	for (i=0; i<inserts_number; i++){

		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}

		if ( !(hp = gethostbyname("localhost"))) {
			printf("DNS lookup failed\n");
			exit(1);
		}

		//again:
    port = ports[rand() % ports_length];
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));
		if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			//usleep(200000);
			//goto again;
			perror("connect");
			printf("port: %d\n",port);
		}

    sprintf(message,"INSERT,%s,%d",(inserts[i].first),inserts[i].second);

		if (write(sd,message,strlen(message)) != strlen(message))
			perror("write");
#ifdef DEBUG
    cout << message <<endl;
#endif
		char answer[150];

		if (rd=read(sd,answer,150)){
			answer[rd]='\0';
#ifdef DEBUG
			cout << answer << endl;
#endif
		}
    if (close(sd) < 0)
			 perror("close");
	}
	gettimeofday(&ttf,NULL);
	ttotal=(ttf.tv_sec-tts.tv_sec)+(ttf.tv_usec-tts.tv_usec)*0.000001;
	cout << "Time: " << ttotal << " " << "Inserts per second: " << inserts_number/ ttotal << endl;
}

void execute_queries(char ** queries, int queries_number, int * ports, int ports_length){

	struct timeval tts,ttf;
	double ttotal;
	char message[300];
	char server_port[6];
	int sd,i,port,rd;
	struct hostent *hp;
	struct sockaddr_in sa;

	cout << queries_number << endl;
	gettimeofday(&tts,NULL);
	for (i=0; i<queries_number; i++){

		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}

		if ( !(hp = gethostbyname("localhost"))) {
			printf("DNS lookup failed\n");
			exit(1);
		}

		//again:
    port = ports[rand() % ports_length];
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));
		if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			//usleep(200000);
			//goto again;
			perror("connect");
		}

    sprintf(message,"QUERY,%s",queries[i]);

		if (write(sd,message,strlen(message)) != strlen(message))
			perror("write");
#ifdef DEBUG
    cout << message << endl;;
#endif
		char answer[150];

		if (rd=read(sd,answer,150)){
			answer[rd]='\0';
#ifdef DEBUG
			cout << answer << endl;
#endif
		}
    if (close(sd) < 0)
			 perror("close");
	}
	gettimeofday(&ttf,NULL);
	ttotal=(ttf.tv_sec-tts.tv_sec)+(ttf.tv_usec-tts.tv_usec)*0.000001;
	cout << "Time: " << ttotal << " " << "Queries per second: " << queries_number/ ttotal << endl;

}

void execute_requests(char ** requests, int requests_number, int * ports, int ports_length){

	struct timeval tts,ttf;
	double ttotal;
	char message[300];
	char server_port[6];
	int sd,i,port,rd;
	struct hostent *hp;
	struct sockaddr_in sa;

	cout << requests_number << endl;
	gettimeofday(&tts,NULL);
	for (i=0; i<requests_number; i++){

		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}

		if ( !(hp = gethostbyname("localhost"))) {
			printf("DNS lookup failed\n");
			exit(1);
		}

		//again:
    port = ports[rand() % ports_length];
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));
		if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			//usleep(200000);
			//goto again;
			perror("connect");
		}

    sprintf(message,"%s",requests[i]);

		if (write(sd,message,strlen(message)) != strlen(message))
			perror("write");
#ifdef DEBUG
    cout << message << endl;;
#endif
		char answer[150];

		if (rd=read(sd,answer,150)){
			answer[rd]='\0';
#ifdef DEBUG
			cout << answer << endl;
#endif
		}
    if (close(sd) < 0)
			 perror("close");
	}
	gettimeofday(&ttf,NULL);
	ttotal=(ttf.tv_sec-tts.tv_sec)+(ttf.tv_usec-tts.tv_usec)*0.000001;
	cout << "Time: " << ttotal << " " << "Queries per second: " << requests_number/ ttotal << endl;

}

int main(int argc, char** argv){

	pair<char *,int> * inserts = new pair<char *,int> [500];
	char * queries[500];
	char * requests[500];
  FILE * fp = fopen("inserts.txt","r");
	FILE * fp1 = fopen("query.txt","r");
	FILE * fp2 = fopen("requests.txt","r");
	char * pch;
	int inserts_number,queries_number,requests_number=0;
	char line[100];


	/*
	*  Read the ATTENTION node in the top in order to choose the right table
	*/
	int ports[] = {49464, 49603, 49854, 50106, 50331, 50655, 50812, 51197, 51209, 52803};
	int ports1[] = {50106, 50655, 51197, 51209, 53560, 53699, 53950, 54427, 54908, 56899};
	int length=sizeof(ports)/sizeof(int),port;

	inserts_number=0;
	while (fgets(line,sizeof(line),fp)){

    pch = strtok(line,",");
    inserts[inserts_number].first=(char *)malloc(strlen(pch)*sizeof(char));
    strcpy(inserts[inserts_number].first,pch);
    pch = strtok(NULL,",");
    inserts[inserts_number].second=atoi(pch);
    //cout << inserts[inserts_number].first << " " << inserts[inserts_number].second << endl;
    inserts_number++;
	}
	queries_number=0;
	while (fgets(line,sizeof(line),fp1)){
    queries[queries_number]=(char *)malloc(strlen(line)*sizeof(char)+1);
		strcpy(queries[queries_number],line);

		//cout << queries[queries_number];
    queries_number++;
	}

	int len=0;
	requests_number=0;
	while (fgets(line,100,fp2)){
		len = strlen(line);
		line[len-2]='\0';
		requests[requests_number]=(char *)malloc(strlen(line)*sizeof(char)+1);
		strcpy(requests[requests_number],line);
		//cout << requests[requests_number] << endl;;
		requests_number++;
	}
	execute_inserts(inserts,inserts_number,ports1,length);
	execute_queries(queries,queries_number,ports1,length);
	execute_requests(requests,requests_number,ports1,length);
	return 0;
}
