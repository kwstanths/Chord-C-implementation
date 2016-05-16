/*
*	Chord emulator:
* Author: Constantinos Kazatzhs kon.kazatis@gmail.com
*/

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
#include <list>

#include "lib.h"
#include "defines.h"

using namespace std;

struct command {
	/*
	* The use of this struct is explained in the file lib.cpp
	*/
	int type;
	std::pair<char *,int> key_and_value;
	int initial_node;
	char * initial_node_hostname, * files;
};

class Chord_node{
	/*
	* Each node corresponds to a class Chord_node. Provide the node's id and the ring size in the constructor
	* In the end of this class you can see the private fields of the node
	*/

	public:
		Chord_node(int new_id,int new_ring_size): id(new_id), ring_size(new_ring_size){};

		/*
		 * Methods for getting and returning private data. Description for those is not necessary cause they do
		 * what their name says.
		 */

		int get_id() const{
			return id;
		}
		int get_successor(){
			return successor;
		};
		int get_predecessor(){
			return predecessor;
		};
		char * get_hostname(){
			return my_hostname;
		}
		char * get_successor_hostname(){
			return successor_hostname;
		};
		char * get_predecessor_hostname(){
			return predecessor_hostname;
		};

		void set_successor(int succ){
			successor = succ;
		};
		void set_predecessor(int pred){
			predecessor = pred;
		};
		void set_hostname(const char * buffer){
			my_hostname = (char *)malloc(strlen(buffer)*sizeof(char)+1);
			if (my_hostname==NULL){
				printf("Out of memory\n");
				exit(1);
			}
			strcpy(my_hostname,buffer);
			my_hostname[strlen(my_hostname)+1]='\0';
		};
		void set_successor_hostname(const char * buffer){
			successor_hostname = (char *)malloc(strlen(buffer)*sizeof(char)+1);
			if (successor_hostname==NULL){
				printf("Out of memory\n");
				exit(1);
			}
			strcpy(successor_hostname,buffer);
			successor_hostname[strlen(successor_hostname)+1]='\0';
		};
		void set_predecessor_hostname(const char * buffer){
			predecessor_hostname = (char *)malloc(strlen(buffer)*sizeof(char)+1);
			if (predecessor_hostname==NULL){
				printf("Out of memory\n");
				exit(1);
			}
			strcpy(predecessor_hostname,buffer);
			predecessor_hostname[strlen(predecessor_hostname)+1]='\0';
		};

		int insert(struct command *my_command){
			/*
			* This is the insert opeartion. Input is a command struct. First if case is to check if this file is
			* between the first and the last node. Second if case is the typical case.
			*/
			int hash_value = compute_sha1_hash(my_command->key_and_value.first,ring_size);
			int position;
#ifdef DEBUG
			cout << "Hash value! :" << hash_value << endl;
#endif
			/*
			 * Return 1 if you did the insertion, or 0 if you didn't
			 */
			if (id < predecessor && ((hash_value > id && hash_value > predecessor)||hash_value < id) ){
				position = search_in_a_vector_of_files(files,hash_value);
				if(position == -1 ){
					files.push_back(make_pair(hash_value,my_command->key_and_value.second));
				}else{
					files.at(position).second = my_command->key_and_value.second;
				}
				return 1;
			}else if (hash_value < id && hash_value > predecessor) {
				position = search_in_a_vector_of_files(files,hash_value);
				if(position == -1 ){
					files.push_back(make_pair(hash_value,my_command->key_and_value.second));
				}else{
					files.at(position).second = my_command->key_and_value.second;
				}
				return 1;
			}else return 0;


		};

		int query(struct command *my_command){
			/*
			* This is the query opeartion. Input is a command struct. First if case is to check if this file is
			* between the first and the last node. Second if case is the typical case.
			*/
			int hash_value = compute_sha1_hash(my_command->key_and_value.first,ring_size);
			int position;

			/*
			 * Return 1 if you found the key, or 0 if you didn't
			 */

			if (id < predecessor && ((hash_value > id && hash_value > predecessor)||hash_value < id) ){
				position = search_in_a_vector_of_files(files,hash_value);
				if(position == -1 ){
					return 0;
				}else{
					return files.at(position).second;
				}
			}else if (hash_value < id && hash_value > predecessor) {
				position = search_in_a_vector_of_files(files,hash_value);
				if(position == -1 ){
					return 0;
				}else{
					return files.at(position).second;
				}
			}else return 0;
		}

		int delete_file(struct command *my_command){
			/*
			* This is the delete opeartion. Input is a command struct. First if case is to check if this file is
			* between the first and the last node. Second if case is the typical case.
			*/
			int hash_value = compute_sha1_hash(my_command->key_and_value.first,ring_size);
			int position;

			/*
			 * Return 1 if you deleted the key, or 0 if you didn't
			 */

			if (id < predecessor && ((hash_value > id && hash_value > predecessor)||hash_value < id) ){
				position = search_in_a_vector_of_files(files,hash_value);
				if(position == -1 ){
					return 0;
				}else{
					files.erase(files.begin() + position);
					return 1;
				}
			}else if (hash_value < id && hash_value > predecessor) {
				position = search_in_a_vector_of_files(files,hash_value);
				if(position == -1 ){
					return 0;
				}else{
					files.erase(files.begin() + position);
					return 1;
				}
			}else return 0;
		}

		/*
		* These two function are of great help when i need to insert or delete a file when i only know its hash
		*/
		int delete_file_from_hash(int hash_value){
			int position;

			position = search_in_a_vector_of_files(files,hash_value);
			if(position == -1 ){
					printf("Internal error\n");
			}else{
				files.erase(files.begin() + position);
				return 1;
			}
		}

		int insert_file_from_hash(int hash_value, int file_value){
			int position;

			position = search_in_a_vector_of_files(files,hash_value);
			if(position == -1 ){
					files.push_back(make_pair(hash_value,file_value));
					return 1;
			}else{
				printf("Internal error\n");
			}
		}

		int listen_incoming_connections(){
			/*
			* This is the funtion each node-thread executes. Here is where all the magic happens.
			* Basically you setup a socket in the PORT_BASE + id port and wait. After receiving
			*	a message go to the switch clause and do what you need to do (implement the routing protocol)
			*/
			int sd,newsd,return_value;
			int file,value;
			struct sockaddr_in sa;
			socklen_t len = sizeof(struct sockaddr_in);
			char addrstr[INET_ADDRSTRLEN];
			char temp_buffer[50];
			char buffer[900];
			char buffer_answer[300];
			char * temp_string;
			vector<int> delete_vector;

			ssize_t ret;
			struct command* answer;

			if ((sd=socket(PF_INET, SOCK_STREAM,0)) < 0){
				perror("socket");
				return 1;
			}

			int yes=1;
			if (setsockopt(sd,SOL_SOCKET,SO_REUSEADDR, &yes, sizeof(int))==-1)
				perror("setsockopt");
#ifdef DEBUG
			printf("Node: %d listening to port: %d\n",id,PORT_BASE+id);
#endif
		  try_again:
			memset(&sa, 0, sizeof(sa));
			sa.sin_family = AF_INET;
			sa.sin_port = htons(PORT_BASE+id);
			sa.sin_addr.s_addr = htonl(INADDR_ANY);

			if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
				usleep(800000);
				perror("bind");
				printf("port: %d\n",PORT_BASE+id);
				goto try_again;
			}

			if (listen(sd, TCP_BACKLOG) < 0){
				perror("listen");
				return 1;
			}

			while(1){
				/*
				* Iterate for ever accepting and answering until there is a DEPART request.
				*/
				if ((newsd = accept(sd, (struct sockaddr *)&sa, &len)) < 0){
					perror("accept");
					return 1;
				}

				if (!inet_ntop(AF_INET, &sa.sin_addr, addrstr, sizeof(addrstr))) {
					perror("could not format client's IP address");
					return 1;
				}

				ret = read(newsd,buffer,sizeof(buffer)-1);
				if (ret <= 0) continue;
				if (buffer[ret-1] == '\n') buffer[ret-1]='\0';
				else buffer[ret]= '\0';
#ifdef DEBUG
				printf("Node: %d Message: %s\n",id,buffer);
#endif
				answer = parse_the_command(buffer);
				if (answer->type == INSERT_TYPE && (answer->key_and_value.first==NULL || answer->key_and_value.second == -1)) {
					if (close(newsd) < 0)
						perror("close");
					continue;
				}
				if (answer->type == QUERY_TYPE && answer->key_and_value.first ==NULL) {
					if (close(newsd) < 0)
						perror("close");
					continue;
				}
				if (answer->type == DELETE_TYPE && answer->key_and_value.first ==NULL){
					if (close(newsd) < 0)
						perror("close");
					continue;
				}
				if (answer->type == -1) continue;
				/*
				 * Buffer holds the initial message
				 */
				switch (answer->type){
					case INSERT_TYPE :
#ifdef DEBUG
						printf("Node: %d Type: %d Key: %s Value: %d Initial node: %d Initial node name:%s\n",id,answer->type,answer->key_and_value.first,answer->key_and_value.second,answer->initial_node,answer->initial_node_hostname);
#endif
						if (insert(answer)){
							if (answer->initial_node!=-1){
								/*
								 * Case: There is initial node and i did the insertion
								 * Purpose: Answer to the initial node and he will answer to the client
								 */
#ifdef DEBUG
								printf("Node: %d Case 1\n",id);
#endif
								sprintf(buffer_answer,"INSERT DONE AT NODE %s PORT %d ID %d\n",my_hostname,PORT_BASE+id,id);
								forward_message(buffer_answer,answer->initial_node_hostname,PORT_BASE + ring_size);
							}else{
								/*
								 * Case: There is no initial node and i did the insertion
								 * Purpose: Answer directly to the client
								 */
#ifdef DEBUG
								printf("Node: %d Case 2\n",id);
#endif
								sprintf(buffer_answer,"INSERT DONE AT NODE %s PORT %d ID %d\n",my_hostname,PORT_BASE+id,id);
								if (write(newsd,buffer_answer,strlen(buffer_answer)) != strlen(buffer_answer))
									perror("write");
							}
						}else{
							if (answer->initial_node!=-1){
								/*
								 * Case: There is initial node and i didn't do the insertion
								 * Purpose: Just forward the message
								 */
#ifdef DEBUG
								printf("Node: %d Case 3\n",id);
								printf("Forwarding to: %d\n",PORT_BASE+successor);
#endif
								forward_message(buffer,successor_hostname,PORT_BASE+successor);
							}else {
								/*
								 * Case: There is no initial node and i didn't do the insertion
								 * Purpose: Forward the message to the successor adding the initial node (me)
								 * 			and wait in port PORT_BASE + ring_size for the node that did the insertion
								 */
#ifdef DEBUG
								printf("Node: %d Case 4\n",id);
#endif
								char * new_message = (char *)malloc(strlen(buffer)*sizeof(char)+sizeof(char)+int_length(id)*sizeof(int)+strlen(my_hostname)*sizeof(char));
								if (new_message == NULL){
									printf("out of memory");
									exit(1);
								}
								sprintf(new_message,"%s,%d,%s",buffer,id,my_hostname);
#ifdef DEBUG
								printf("Forwarding to: %d\n",PORT_BASE+successor);
#endif
								forward_message(new_message,successor_hostname,PORT_BASE+successor);

								char * answer_buffer;
								answer_buffer = wait_response_initial_node(PORT_BASE+ ring_size);
								if (write(newsd,answer_buffer,strlen(answer_buffer)) != strlen(answer_buffer))
									perror("write");

								//free(new_message);
								//free(answer_buffer);
							}
						}
						break;
					case QUERY_TYPE :
#ifdef DEBUG
						printf("Node: %d Type: %d Key: %s Initial node: %d Initial node name:%s\n",id,answer->type,answer->key_and_value.first,answer->initial_node,answer->initial_node_hostname);
#endif
						int value;
						if (value = query(answer)){
							if (answer->initial_node != -1){
								/*
								 * Case: There is initial node and i found the key
								 * Purpose: Answer to the initial node and he will answer to the client
								 */
#ifdef DEBUG
								printf("Node: %d Case 1\n",id);
#endif
								sprintf(buffer_answer,"KEY FOUND AT NODE %s PORT %d ID %d Value: %d\n",my_hostname,PORT_BASE+id,id,value);
								forward_message(buffer_answer,answer->initial_node_hostname,PORT_BASE + ring_size);
							}else{
								/*
								 * Case: There is no initial node and i found the key
								 * Purpose: Answer to the client
								 */
#ifdef DEBUG
								printf("Node: %d Case 2\n",id);
#endif
								sprintf(buffer_answer,"KEY FOUND AT NODE %s PORT %d ID %d Value: %d\n",my_hostname,PORT_BASE+id,id,value);
								if (write(newsd,buffer_answer,strlen(buffer_answer)) != strlen(buffer_answer))
									perror("write");
							}
						}else{
							if (answer->initial_node != -1){
								/*
								 * Case: There is initial node and i didn't find the key
								 * Purpose: Check if the successor is the initial node. If he isn't just
								 * 			forward the message. If he is send him failure to find message
								 */
#ifdef DEBUG
								printf("Node: %d Case 3\n",id);
#endif
								if (answer->initial_node == successor){
									sprintf(buffer_answer,"KEY NOT FOUND :(\n");
									forward_message(buffer_answer,answer->initial_node_hostname,PORT_BASE + ring_size);
								}else{
									forward_message(buffer,successor_hostname,PORT_BASE+successor);
								}
							}else {
								/*
								 * Case: There is no initial node and i didn't find the key
								 * Purpose: Forward the message to the successor adding the initial node (me)
								 * 			and wait in port PORT_BASE + ring_size for the node who found or not the element
								 */
#ifdef DEBUG
								printf("Node: %d Case 4\n",id);
#endif
								char * new_message = (char *)malloc(strlen(buffer)*sizeof(char)+sizeof(char)+int_length(id)*sizeof(int)+sizeof(char)+strlen(my_hostname)*sizeof(char));
								if (new_message == NULL){
									printf("out of memory");
									exit(1);
								}
								sprintf(new_message,"%s,%d,%s",buffer,id,my_hostname);
								forward_message(new_message,successor_hostname,PORT_BASE+successor);

								char * answer_buffer;
								answer_buffer = wait_response_initial_node(PORT_BASE+ ring_size);
								if (write(newsd,answer_buffer,strlen(answer_buffer)) != strlen(answer_buffer))
									perror("write");

								//free(new_message);
								//free(answer_buffer);
							}
						}
						break;
					case DELETE_TYPE :
#ifdef DEBUG
						printf("Node: %d Type: %d Key: %s Initial node: %d Initial node name:%s\n",id,answer->type,answer->key_and_value.first,answer->initial_node,answer->initial_node_hostname);
#endif
						if (value = delete_file(answer)){
							if (answer->initial_node != -1){
								/*
								 * Case: There is initial node and i deleted the key
								 * Purpose: Answer to the initial node and he will answer to the client
								 */
#ifdef DEBUG
								printf("Node: %d Case 1\n",id);
#endif
								sprintf(buffer_answer,"KEY DELETED AT NODE %s PORT %d ID %d\n",my_hostname,PORT_BASE+id,id);
								forward_message(buffer_answer,answer->initial_node_hostname,PORT_BASE + ring_size);
							}else{
								/*
								 * Case: There is no initial node and i deleted the key
								 * Purpose: Answer to the client
								 */
#ifdef DEBUG
								printf("Node: %d Case 2\n",id);
#endif
								sprintf(buffer_answer,"KEY DELETED AT NODE %s PORT %d ID %d\n",my_hostname,PORT_BASE+id,id);
								if (write(newsd,buffer_answer,strlen(buffer_answer)) != strlen(buffer_answer))
									perror("write");
							}
						}else{
							if (answer->initial_node != -1){
								/*
								 * Case: There is initial node and i didn't delete the key
								 * Purpose: Check if the successor is the initial node. If he isn't just
								 * 			forward the message. If he is send him failure to delete message
								 */
#ifdef DEBUG
								printf("Node: %d Case 3\n",id);
#endif
								if (answer->initial_node == successor){
									sprintf(buffer_answer,"KEY NOT FOUND :(\n");
									forward_message(buffer_answer,answer->initial_node_hostname,PORT_BASE + ring_size);
								}else{
									forward_message(buffer,successor_hostname,PORT_BASE+successor);
								}
							}else {
								/*
								 * Case: There is no initial node and i didn't delete the key
								 * Purpose: Forward the message to the successor adding the initial node (me)
								 * 			and wait in port PORT_BASE + ring_size for the node who deleted or not the element
								 */
#ifdef DEBUG
								printf("Node: %d Case 4\n",id);
#endif
								char * new_message = (char *)malloc(strlen(buffer)*sizeof(char)+sizeof(char)+int_length(id)*sizeof(int)+sizeof(char)+strlen(my_hostname)*sizeof(char));
								if (new_message == NULL){
									printf("out of memory");
									exit(1);
								}
								sprintf(new_message,"%s,%d,%s",buffer,id,my_hostname);
								forward_message(new_message,successor_hostname,PORT_BASE+successor);

								char * answer_buffer;
								answer_buffer = wait_response_initial_node(PORT_BASE+ ring_size);
								if (write(newsd,answer_buffer,strlen(answer_buffer)) != strlen(answer_buffer))
									perror("write");

								//free(new_message);
								//free(answer_buffer);
							}
						}
						break;
					case QUERY_STAR :
#ifdef DEBUG
						printf("Node: %d Type: %d\n",id,answer->type);
#endif
						/*
						* Just put all your files in a buffer and send it
						*/
						sprintf(buffer,"Node %d files:\nFile Hash\tValue\n",id);
						for(int i=0; i< files.size(); i++){
							sprintf(temp_buffer,"%d\t\t%d\n",files.at(i).first,files.at(i).second);
							if (i==files.size()-1) temp_buffer[strlen(temp_buffer)]='\0';
							strcat(buffer,temp_buffer);
						}
						if (write(newsd,buffer,strlen(buffer)) != strlen(buffer))
							perror("write");

						break;
					case JOIN_TRANSFER :
#ifdef DEBUG
						printf("Node: %d Type: %d New node: %d New node name: %s\n",id,answer->type,answer->initial_node,answer->initial_node_hostname);
#endif
						delete_vector.clear();
						sprintf(buffer,"JOIN_RECEIVE,");
						/*
						* Two cases: For a node that is smaller than me but has to also take the file at the end of the ring
						*	and for the typical case
						*/
						if (answer->initial_node < id){
#ifdef DEBUG
							printf("1st case\n");
#endif
							for (int i=0; i<files.size(); i++){
								if (files.at(i).first < answer->initial_node || files.at(i).first > id) {
									sprintf(temp_buffer,"%d-%d-",files.at(i).first,files.at(i).second);
									strcat(buffer,temp_buffer);
									delete_vector.push_back(files.at(i).first);
								}
							}
						}else{
#ifdef DEBUG
							printf("2nd case\n");
#endif
							for (int i=0; i<files.size(); i++){
								if ((files.at(i).first > id)&&(files.at(i).first < answer->initial_node)){
									printf("%d ",files.at(i).first);
									sprintf(temp_buffer,"%d-%d-",files.at(i).first,files.at(i).second);
									strcat(buffer,temp_buffer);
									delete_vector.push_back(files.at(i).first);
								}
							}
						}
						for (int i=0; i < delete_vector.size(); i++) delete_file_from_hash(delete_vector.at(i));
						forward_message(buffer,answer->initial_node_hostname,PORT_BASE+answer->initial_node);

						break;
					case JOIN_RECEIVE :
#ifdef DEBUG
						printf("Node: %d Type: %d Receiving those files: %s\n",id,answer->type,answer->files);
#endif
						/*
						* Inserts all the new files. This is the first thing that a joining node executes!
						*/
						if (answer->files == NULL) break;
						strcpy(buffer,answer->files);
						temp_string = strtok(buffer,"-");
						while(temp_string != NULL){
							file= atoi(temp_string);
							temp_string = strtok(NULL,"-");
							value = atoi(temp_string);
							temp_string = strtok(NULL,"-");

							insert_file_from_hash(file,value);
						}

						break;
					case DEPART_TRANSFER :
#ifdef DEBUG
						printf("Node: %d Type: %d Removing ths node.... :(\n",id,answer->type);
#endif
						/*
						* No cases here. Just send everything away.
						*/
						sprintf(buffer,"DEPART_RECEIVE,");
						for (int i=0; i<files.size(); i++){
							sprintf(temp_buffer,"%d-%d-",files.at(i).first,files.at(i).second);
							strcat(buffer,temp_buffer);
						}
						forward_message(buffer,successor_hostname,PORT_BASE+successor);

						if (close(sd) < 0)
							perror("close");
						return 1;

						break;
					case DEPART_RECEIVE :
#ifdef DEBUG
						printf("Node: %d Type: %d Receiving those files: %s\n",id,answer->type,answer->files);
#endif
						/*
						* And here just insert everyting
						*/
						if (answer->files == NULL) break;
						strcpy(buffer,answer->files);
						temp_string = strtok(buffer,"-");
						while(temp_string != NULL){
							file= atoi(temp_string);
							temp_string = strtok(NULL,"-");
							value = atoi(temp_string);
							temp_string = strtok(NULL,"-");
							insert_file_from_hash(file,value);
						}

					break;
				}

				if (close(newsd) < 0)
					perror("close");
				/*
				   if (answer->key_and_value.first != NULL)
				   free(answer->key_and_value.first);
				   if (answer->initial_node_hostname != NULL)
				   free(answer->initial_node_hostname);
				   if (answer != NULL)
				   free(answer);
				 */
			}
		};

		bool operator == (const Chord_node& a) {
			return a.get_id() == id;
		}

	private:
		/*
		 * Hold the size of the ring, the id's and hostnames of the successor and the predecessor
		 * Also hold a vector of files
		 */
		int ring_size;
		int id,successor,predecessor;
		char *my_hostname, *successor_hostname, *predecessor_hostname;
		vector<pair<int,int>> files;

};

bool compare(const Chord_node& first, const Chord_node& second){
	return first.get_id() < second.get_id();
}

void print_list(list<Chord_node> nodes){

	list<Chord_node>::iterator iterator;
	for(iterator = nodes.begin(); iterator != nodes.end(); ++iterator){
		printf("Node: %d Name: %s\n\tSuccessor:%d\t Predecessor:%d\n\tName: %s\t Name:%s\n\n",PORT_BASE+(*iterator).get_id(),(*iterator).get_hostname(),(*iterator).get_successor(),(*iterator).get_predecessor(),(*iterator).get_successor_hostname(),(*iterator).get_predecessor_hostname());
	}
};

void set_id_and_hostnames(list<Chord_node>* nodes);
int check_node_existence(list<Chord_node>* nodes, int nodehash);
Chord_node return_node(std::list<Chord_node>* nodes, int nodehash);

int main(int argc, char** argv){

	if (argc < 3){
		cout << "Usage:" << "./chord_emu <Number of initial nodes> <Ring size log>" << endl;
		return 0;
	}

	int number_of_nodes, M, ring_size;

	number_of_nodes = atoi(argv[1]);
	M = atoi(argv[2]);

	ring_size = pow(2,M);
	cout << "Nodes: " << number_of_nodes << endl;
	cout << "Ring size: " << ring_size << endl;

	/*
	 * Create a list of Chord_node elements and then sort it.
	 */

	list<Chord_node> nodes;
	for(int i=0; i<number_of_nodes; i++){
		Chord_node newNode = Chord_node(compute_sha1_hash(i,ring_size),ring_size);
		newNode.set_hostname("localhost");
		nodes.push_back(newNode);
	}

	nodes.sort(compare);

	/*
	 * For every node in the vector find his position in the circle
	 * and set the proper successor and predecessor
	 */

	set_id_and_hostnames(&nodes);

	/*
	 * Print the nodes as an "interface" for sending the messages  and
	 * spawn a thread for each node listening for incoming messages
	 */
	print_list(nodes);
#ifdef DEBUG
	printf("---> Debugging is on! Remove it for measurements! <---\n");
#endif

	vector<thread> threads;
	for(list<Chord_node>::iterator iterator=nodes.begin(); iterator != nodes.end(); ++iterator){
		threads.push_back(thread(&Chord_node::listen_incoming_connections, iterator));
	}

	char buffer[50];
	int nodeid, nodehash;
	while(1){
		/*
		* Iterate forever accepting joins and departs
		*/
		cin >> buffer;
		if (strcmp(buffer,"JOIN")==0) {
			cout << "New node ID: " ;
			cin >> nodeid;
			nodehash =compute_sha1_hash(nodeid,ring_size);
			cout << "About to insert the node: " << nodeid << " with hash: " << nodehash <<endl;

			if (check_node_existence(&nodes,nodehash)) cout << "Node already exists..." << endl;
			else {
				Chord_node newNode = Chord_node(nodehash,ring_size);
				newNode.set_hostname("localhost");
				nodes.push_back(newNode);
				nodes.sort(compare);
				set_id_and_hostnames(&nodes);
				threads.push_back(thread(&Chord_node::listen_incoming_connections, return_node(&nodes,nodehash)));
				print_list(nodes);
				newNode = return_node(&nodes,nodehash);
				sprintf(buffer,"JOIN_TRANSFER,%d,%s",newNode.get_id(),newNode.get_hostname());
				forward_message(buffer,newNode.get_successor_hostname(),PORT_BASE+newNode.get_successor());
				usleep(2000000);

			}
		}else if (strcmp(buffer,"DEPART")==0){
			cout << "Node ID: " ;
			cin >> nodeid;
			nodehash = nodeid - PORT_BASE;
			cout <<"About to remove the node with hash: " << nodehash << endl;

			if (check_node_existence(&nodes,nodehash)==0) cout << "Node doesn't exist..." << endl;
			else {
				Chord_node node_to_remove = return_node(&nodes,nodehash);
				printf("Removing node : %d\n",node_to_remove.get_id());
				sprintf(buffer,"DEPART_TRANSFER");
				forward_message(buffer,node_to_remove.get_hostname(),PORT_BASE+node_to_remove.get_id());

				usleep(2000000);
				nodes.remove(node_to_remove);
				set_id_and_hostnames(&nodes);
				print_list(nodes);

			}
		}
	}


/*
	for(int i =0; i<nodes.size(); i++)
		threads.at(i).join();

	for(int i =0; i<nodes.size(); i++)
		close(PORT_BASE+nodes.at(i).get_id());
*/
}

int check_node_existence(std::list<Chord_node>* nodes, int nodehash){
	for(list<Chord_node>::iterator iterator=nodes->begin(); iterator != nodes->end(); ++iterator){
		if ((*iterator).get_id() == nodehash) return 1;
	}
	return 0;
}

Chord_node return_node(std::list<Chord_node>* nodes, int nodehash){
	for(list<Chord_node>::iterator iterator=nodes->begin(); iterator != nodes->end(); ++iterator){
		if ((*iterator).get_id() == nodehash) return *iterator;
	}

}

void set_id_and_hostnames(list<Chord_node>* nodes){
	/*
	* A function that sets all the id's of the nodes(successor's and predecessors') and the hostnames
	*	(successor's and predecessors's as well). Usually called after a sorting.
	*/
	list<Chord_node>::iterator iterator, prev_node, next_node;
	for(iterator=nodes->begin(); iterator != nodes->end(); ++iterator){
		iterator++; next_node = iterator;
		iterator--; iterator--; prev_node=iterator++;
		if ((*iterator).get_id() == nodes->front().get_id()){
			(*iterator).set_successor((*next_node).get_id());
			(*iterator).set_predecessor(nodes->back().get_id());

			(*iterator).set_successor_hostname((*next_node).get_hostname());
			(*iterator).set_predecessor_hostname(nodes->back().get_hostname());
		}else if ((*iterator).get_id() == nodes->back().get_id()){
			(*iterator).set_successor(nodes->front().get_id());
			(*iterator).set_predecessor((*prev_node).get_id());

			(*iterator).set_successor_hostname(nodes->back().get_hostname());
			(*iterator).set_predecessor_hostname((*prev_node).get_hostname());
		}else{
			(*iterator).set_successor((*next_node).get_id());
			(*iterator).set_predecessor((*prev_node).get_id());

			(*iterator).set_successor_hostname((*next_node).get_hostname());
			(*iterator).set_predecessor_hostname((*prev_node).get_hostname());
		}
	}
}
