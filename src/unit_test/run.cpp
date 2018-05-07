#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <functional>
#include <algorithm>
#include <thread>
#include <string.h>
#include <string>
#include <sstream>
#include <iterator>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <utility>
#include <fstream>
#include <netdb.h>
#include <sys/time.h>

#include "lib.hpp"
#include "defines.hpp"

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

void execute_inserts(std::vector<std::pair<std::string,int>>& inserts, int inserts_number, int * ports, int ports_length){

	struct timeval tts,ttf;
	double ttotal;
	char message[300];
	char server_port[6];
	int sd,i,port,rd;
	struct hostent *hp;
	struct sockaddr_in sa;

	cout << inserts_number << endl;
	gettimeofday(&tts,NULL);
	for (i=0; i<inserts_number; i++){

		/* Create message */
    	sprintf(message,"INSERT,%s,%d",(inserts[i].first.c_str()),inserts[i].second);

		/* Connect to random chord node */
		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}
		if ( !(hp = gethostbyname("localhost"))) {
			printf("DNS lookup failed\n");
			exit(1);
		}

		port = ports[rand() % ports_length];
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));
		if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			perror("connect");
			printf("port: %d\n",port);
		}

		/* Send message */
		if (write(sd,message,strlen(message)) != strlen(message))
			perror("write");
#ifdef DEBUG
    	cout << message <<endl;
#endif
		/* Wait answer */
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

void execute_queries(std::vector<std::string> queries, int queries_number, int * ports, int ports_length){

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

    	sprintf(message,"QUERY,%s",queries[i].c_str());

		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}

		if ( !(hp = gethostbyname("localhost"))) {
			printf("DNS lookup failed\n");
			exit(1);
		}

    	port = ports[rand() % ports_length];
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));
		if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			perror("connect");
		}

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

void execute_requests(std::vector<std::string> requests, int requests_number, int * ports, int ports_length){

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

		sprintf(message,"%s",requests[i].c_str());
	
		if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}

		if ( !(hp = gethostbyname("localhost"))) {
			printf("DNS lookup failed\n");
			exit(1);
		}

		port = ports[rand() % ports_length];
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));
		if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			perror("connect");
		}

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
	cout << "Time: " << ttotal << " " << "Requests per second: " << requests_number/ ttotal << endl;

}

template<typename Out> void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

int main(int argc, char** argv){

	std::vector<std::pair<std::string, int>> inserts;
	std::vector<std::string> queries;
	std::vector<std::string> requests;
	std::ifstream infile_inserts("inserts.txt");
	std::ifstream infile_queries("query.txt");
	std::ifstream infile_requests("requests.txt");
	std::string line;

	/*
	*  Read the ATTENTION node in the top in order to choose the right table
	*/
	int ports[] = {49464, 49603, 49854, 50106, 50331, 50655, 50812, 51197, 51209, 52803};
	int ports1[] = {50106, 50655, 51197, 51209, 53560, 53699, 53950, 54427, 54908, 56899};
	int ports2[] = {49603, 49854, 50812, 51197, 52803};
	int length=sizeof(ports)/sizeof(int);
	int length1=sizeof(ports1)/sizeof(int);
	int length2=sizeof(ports2)/sizeof(int);

	/* Read inserts file */
	while (std::getline(infile_inserts, line)) {
		std::vector<std::string> elems = split(line, ',');
		inserts.push_back(std::make_pair(elems[0], stoi(elems[1])));
	}

	/* Read queries file */
	while(std::getline(infile_queries, line)){
		queries.push_back(line);
	}

	/* Read request file */
	while(std::getline(infile_requests, line)){
		requests.push_back(line);
	}

	execute_inserts(inserts, inserts.size(), ports, length);
	execute_queries(queries, queries.size(), ports, length);
	execute_requests(requests, requests.size(), ports, length);
	return 0;
}
