#include "lib.hpp"

#include <math.h>
#include <openssl/sha.h>
#include <string>
#include <string.h>
#include <utility>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#include "defines.hpp"


int int_length(int number){
	/*
	*	Returns the length of an integer number in number of digits from proper memory allocation
	*/
	int length =1;
	while(number /= 10)
		length++;

	return length;
}

void convert_int_to_char_array(unsigned char *buffer, int number){
	/*
	* Converts an integer number into a char array for proper hashing (Uses int_length())
	*/
	int digit,length,temp;

	length = int_length(number);

	temp = number;
	digit=length-1;
	while (temp!=0){
		buffer[digit] = temp%10;
		temp = (temp - buffer[digit])/10;
		digit++;
	}
};

int collect_hash_output(unsigned char *buffer,int buffer_size, int ring_size){
	/*
	* Returns the hash output of the SHA1 output(buffer) and moding with ring_size
	*/
	int length = buffer_size;
	long long int number=0;
	int digit=0;

	while(length >= 0){
		number += buffer[digit]*pow(10,length);
		length--;
		digit++;
	}
	return number % ring_size;
}

int compute_sha1_hash(int not_hashed,int ring_size){
	/*
	* Returns the SHA1 hash of an integer (Uses convert_int_to_char_array(),SHA1(),collect_hash_output())
	*/
	unsigned char *in_str;
	unsigned char out_hash[SHA_DIGEST_LENGTH];

	in_str=(unsigned char *)malloc(int_length(not_hashed)*sizeof(unsigned char));
	if (in_str==NULL){
		printf("Out of memory\n");
		exit(1);
	}
	convert_int_to_char_array(in_str,not_hashed);
	SHA1(in_str,int_length(not_hashed),out_hash);

	free(in_str);
	return collect_hash_output(out_hash,10,ring_size);
}

int compute_sha1_hash(const char * not_hashed,int ring_size){
	/*
	* Returns the SHA1 hash of a string (Uses convert_int_to_char_array(),SHA1(),collect_hash_output())
	*/
	unsigned char *in_str;
	unsigned char out_hash[SHA_DIGEST_LENGTH];

	in_str=(unsigned char *)malloc(strlen(not_hashed)*sizeof(unsigned char));
	if (in_str==NULL){
		printf("Out of memory\n");
		exit(1);
	}
	for(int i=0; i<strlen(not_hashed); i++)
		in_str[i]=not_hashed[i];
	SHA1(in_str,strlen(not_hashed),out_hash);

	free(in_str);
	return collect_hash_output(out_hash,10,ring_size);
};

Command parse_the_command(char * initial_buffer){
	/*
	* Each message a node recieves is being parsed by this function.This function returns a struct
	* which contains the information needed for the node to process the request. For more info About
	* the messages see README_FIRST file
	*/

	Command answer;
	char * buffer =(char *)malloc(strlen(initial_buffer)*sizeof(char)+1);
	if (buffer == NULL){
		printf("Out of memory\n");
		exit(1);
	}
	strcpy(buffer,initial_buffer);
	buffer[strlen(initial_buffer)]='\0';
	char * mini_string;
	int i,ret;

	answer.type = -1;
	answer.key_and_value.first = "";
	answer.key_and_value.second = -1;
	answer.initial_node = -1;
	answer.initial_node_hostname = "";
	answer.files = "";

	i=0;
	mini_string = strtok(buffer,",");
	while(mini_string != NULL){
		if (i==0){
			if (strcmp(mini_string,"INSERT")==0) answer.type = INSERT_TYPE;
			if (strcmp(mini_string,"QUERY")==0) answer.type = QUERY_TYPE;
			if (strcmp(mini_string,"DELETE")==0) answer.type = DELETE_TYPE;
			if (strcmp(mini_string,"QUERY *")==0) answer.type = QUERY_STAR;
			if (strcmp(mini_string,"JOIN_TRANSFER")==0) answer.type = JOIN_TRANSFER;
			if (strcmp(mini_string,"JOIN_RECEIVE")==0) answer.type = JOIN_RECEIVE;
			if (strcmp(mini_string,"DEPART_TRANSFER")==0) answer.type = DEPART_TRANSFER;
			if (strcmp(mini_string,"DEPART_RECEIVE")==0) answer.type = DEPART_RECEIVE;
		}
		if (answer.type == INSERT_TYPE){
			if (i==1) answer.key_and_value.first = std::string(mini_string);
			if (i==2) answer.key_and_value.second = std::atoi(mini_string);
			if (i==3) answer.initial_node = std::atoi(mini_string);
			if (i==4) answer.initial_node_hostname = std::string(mini_string);
		}else if (answer.type == QUERY_TYPE){
			if (i==1) answer.key_and_value.first = std::string(mini_string);
			if (i==2) answer.initial_node = std::atoi(mini_string);
			if (i==3) answer.initial_node_hostname = std::string(mini_string);
		}else if (answer.type == DELETE_TYPE){
			if (i==1) answer.key_and_value.first = std::string(mini_string);
			if (i==2) answer.initial_node = std::atoi(mini_string);
			if (i==3) answer.initial_node_hostname = std::string(mini_string);
		} else if (answer.type == JOIN_TRANSFER){
			if (i==1) answer.initial_node = std::atoi(mini_string);
			if (i==2) answer.initial_node_hostname = std::string(mini_string);
		} else if (answer.type == JOIN_RECEIVE){
			if (i==1) answer.files = std::string(mini_string);
		} else if (answer.type == DEPART_RECEIVE){
			if (i==1) answer.files = std::string(mini_string);
		}

		i++;
		mini_string = strtok(NULL,",");
	}
	
	return answer;
};

int search_in_a_vector_of_files(std::vector<std::pair<int,int>> files,int key){
	/*
	* Returns the position of a file in a vector of files or -1 if it doesn't exist
	*/
	for(int i=0; i<files.size(); i++){
		if (files.at(i).first == key) return i;

	}
	return -1;
};

void forward_message(std::string message, std::string hostname, int port){
	/*
	* Client procedure which sends the message message into hostname:port
	*/
	int sd;
	struct hostent *hp;
	struct sockaddr_in sa;
	
#ifdef DEBUG
	printf("Sending[ %s ] to %s:%d ",message,hostname,port);
#endif
	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	if ( !(hp = gethostbyname(hostname.c_str()))) {
		printf("DNS lookup failed for host %s\n", hostname.c_str());
		exit(1);
	}

	while(true){
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));
		if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) == 0) break;
		usleep(200000);
	}

	if (write(sd, message.c_str(), message.length()) != message.length()) perror("write");

	if (close(sd) < 0) perror("close");

#ifdef DEBUG
	printf("Message sent");
#endif

};

char * wait_response_initial_node(int port){
	/*
	*	Server procedure which opens a socket in localhost:port and waits for a connection
	*/
	char buf[300];
	char addrstr[INET_ADDRSTRLEN];
	int sd, newsd;
	ssize_t ret;
	socklen_t len;
	struct sockaddr_in sa;

	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		return NULL;
		exit(1);
	}

	int yes=1;
	if (setsockopt(sd,SOL_SOCKET,SO_REUSEADDR, &yes, sizeof(int))==-1)
		perror("setsockopt");

	again:
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		usleep(200000);
		perror("bind");
		goto again;
	}

	if (listen(sd, TCP_BACKLOG) < 0) {
		perror("listen");
		exit(1);
	}
	len = sizeof(struct sockaddr_in);
	if ((newsd = accept(sd, (struct sockaddr *)&sa, &len)) < 0) {
		perror("accept");
		exit(1);
	}
	if (!inet_ntop(AF_INET, &sa.sin_addr, addrstr, sizeof(addrstr))) {
		perror("could not format IP address");
		exit(1);
	}

	ret = read(newsd,buf,sizeof(buf)-1);
	buf[ret]='\0';

	char * buffer = (char *)malloc(ret*sizeof(char)+1);
	if (buffer==NULL){
		printf("Out of memory\n");
		exit(1);
	}
	strcpy(buffer,buf);
	buffer[ret]='\0';
	if (close(newsd)<0)
		perror("close");
	if (close(sd)<0)
		perror("close");

	return buffer;
};

void print_node_files(std::vector<std::pair<int,int>> files){
	/*
	* Prints the files of a node usually
	*/
	for(int i=0; i<files.size(); i++)
		printf("Key: %d Value:%d\n",files.at(i).first,files.at(i).second);
};
