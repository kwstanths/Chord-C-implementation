#include "ChordNode.hpp"

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
#include <cmath>

#include "lib.hpp"
#include "defines.hpp"


int ChordNode::insert(Command command){
    /*
     * This is the insert opeartion. Input is a command struct. First if case is to check if this file is
     * between the first and the last node. Second if case is the typical case.
     */
    int hash_value = compute_sha1_hash(command.key_and_value.first.c_str(), ring_size);
    int position;
#ifdef DEBUG
    cout << "Hash value! :" << hash_value << endl;
#endif
    /*
     * Return 1 if you did the insertion, or 0 if you didn't
     */
    if (id < predecessor && ((hash_value > id && hash_value > predecessor)||hash_value < id) ){
        position = search_in_a_vector_of_files(files, hash_value);
        if(position == -1 ){
            files.push_back(std::make_pair(hash_value, command.key_and_value.second));
        }else{
            files.at(position).second = command.key_and_value.second;
        }
        return 1;
    }else if (hash_value < id && hash_value > predecessor) {
        position = search_in_a_vector_of_files(files, hash_value);
        if(position == -1 ){
            files.push_back(std::make_pair(hash_value, command.key_and_value.second));
        }else{
            files.at(position).second = command.key_and_value.second;
        }
        return 1;
    }else return 0;


};

int ChordNode::query(Command command){
    /*
     * This is the query opeartion. Input is a command struct. First if case is to check if this file is
     * between the first and the last node. Second if case is the typical case.
     */
    int hash_value = compute_sha1_hash(command.key_and_value.first.c_str(), ring_size);
    int position;

    /*
     * Return 1 if you found the key, or 0 if you didn't
     */

    if (id < predecessor && ((hash_value > id && hash_value > predecessor) || hash_value < id) ){
        position = search_in_a_vector_of_files(files, hash_value);
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

int ChordNode::delete_file(Command command){
    /*
     * This is the delete opeartion. Input is a command struct. First if case is to check if this file is
     * between the first and the last node. Second if case is the typical case.
     */
    int hash_value = compute_sha1_hash(command.key_and_value.first.c_str(), ring_size);
    int position;

    /*
     * Return 1 if you deleted the key, or 0 if you didn't
     */

    if (id < predecessor && ((hash_value > id && hash_value > predecessor) || hash_value < id) ){
        position = search_in_a_vector_of_files(files, hash_value);
        if(position == -1 ){
            return 0;
        }else{
            files.erase(files.begin() + position);
            return 1;
        }
    }else if (hash_value < id && hash_value > predecessor) {
        position = search_in_a_vector_of_files(files, hash_value);
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
int ChordNode::delete_file_from_hash(int hash_value){
    int position;

    position = search_in_a_vector_of_files(files,hash_value);
    if(position == -1 ){
        printf("Internal error\n");
    }else{
        files.erase(files.begin() + position);
        return 1;
    }
}

int ChordNode::insert_file_from_hash(int hash_value, int file_value){
    int position;

    position = search_in_a_vector_of_files(files,hash_value);
    if(position == -1 ){
        files.push_back(std::make_pair(hash_value,file_value));
        return 1;
    }else{
        printf("Internal error\n");
    }
}

int ChordNode::listen_incoming_connections(){
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
    std::vector<int> delete_vector;

    ssize_t ret;
    Command answer;

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
        if (answer.type == INSERT_TYPE && (answer.key_and_value.first == "" || answer.key_and_value.second == -1)) {
            if (close(newsd) < 0)
                perror("close");
            continue;
        }
        if (answer.type == QUERY_TYPE && answer.key_and_value.first == "") {
            if (close(newsd) < 0)
                perror("close");
            continue;
        }
        if (answer.type == DELETE_TYPE && answer.key_and_value.first == ""){
            if (close(newsd) < 0)
                perror("close");
            continue;
        }
        if (answer.type == -1) continue;
        /*
         * Buffer holds the initial message
         */
        switch (answer.type){
            case INSERT_TYPE :
#ifdef DEBUG
                printf("Node: %d Type: %d Key: %s Value: %d Initial node: %d Initial node name:%s\n",id,answer->type,answer->key_and_value.first,answer->key_and_value.second,answer->initial_node,answer->initial_node_hostname);
#endif
                if (insert(answer)){
                    if (answer.initial_node!=-1){
                        /*
                         * Case: There is initial node and i did the insertion
                         * Purpose: Answer to the initial node and he will answer to the client
                         */
#ifdef DEBUG
                        printf("Node: %d Case 1\n",id);
#endif
                        sprintf(buffer_answer, "INSERT DONE AT NODE %s PORT %d ID %d\n", my_hostname.c_str(), PORT_BASE+id, id);
                        forward_message(buffer_answer,answer.initial_node_hostname, PORT_BASE + ring_size);
                    }else{
                        /*
                         * Case: There is no initial node and i did the insertion
                         * Purpose: Answer directly to the client
                         */
#ifdef DEBUG
                        printf("Node: %d Case 2\n",id);
#endif
                        sprintf(buffer_answer, "INSERT DONE AT NODE %s PORT %d ID %d\n", my_hostname.c_str(), PORT_BASE+id, id);
                        if (write(newsd,buffer_answer,strlen(buffer_answer)) != strlen(buffer_answer))
                            perror("write");
                    }
                }else{
                    if (answer.initial_node != -1){
                        /*
                         * Case: There is initial node and i didn't do the insertion
                         * Purpose: Just forward the message
                         */
#ifdef DEBUG
                        printf("Node: %d Case 3\n",id);
                        printf("Forwarding to: %d\n",PORT_BASE+successor);
#endif
                        forward_message(buffer, successor_hostname, PORT_BASE+successor);
                    }else {
                        /*
                         * Case: There is no initial node and i didn't do the insertion
                         * Purpose: Forward the message to the successor adding the initial node (me)
                         * 			and wait in port PORT_BASE + ring_size for the node that did the insertion
                         */
#ifdef DEBUG
                        printf("Node: %d Case 4\n",id);
#endif
                        std::string new_message = std::string(buffer) + "," + std::to_string(id) + "," + my_hostname;
#ifdef DEBUG
                        printf("Forwarding to: %d\n",PORT_BASE+successor);
#endif
                        forward_message(new_message, successor_hostname, PORT_BASE + successor);

                        char * answer_buffer;
                        answer_buffer = wait_response_initial_node(PORT_BASE+ ring_size);
                        if (write(newsd,answer_buffer,strlen(answer_buffer)) != strlen(answer_buffer))
                            perror("write");
                    }
                }
                break;
            case QUERY_TYPE :
#ifdef DEBUG
                printf("Node: %d Type: %d Key: %s Initial node: %d Initial node name:%s\n",id,answer->type,answer->key_and_value.first,answer->initial_node,answer->initial_node_hostname);
#endif
                int value;
                if (value = query(answer)){
                    if (answer.initial_node != -1){
                        /*
                         * Case: There is initial node and i found the key
                         * Purpose: Answer to the initial node and he will answer to the client
                         */
#ifdef DEBUG
                        printf("Node: %d Case 1\n",id);
#endif
                        sprintf(buffer_answer,"KEY FOUND AT NODE %s PORT %d ID %d Value: %d\n", my_hostname.c_str(), PORT_BASE+id, id, value);
                        forward_message(buffer_answer, answer.initial_node_hostname, PORT_BASE + ring_size);
                    }else{
                        /*
                         * Case: There is no initial node and i found the key
                         * Purpose: Answer to the client
                         */
#ifdef DEBUG
                        printf("Node: %d Case 2\n",id);
#endif
                        sprintf(buffer_answer,"KEY FOUND AT NODE %s PORT %d ID %d Value: %d\n",my_hostname.c_str(), PORT_BASE+id, id, value);
                        if (write(newsd,buffer_answer,strlen(buffer_answer)) != strlen(buffer_answer))
                            perror("write");
                    }
                }else{
                    if (answer.initial_node != -1){
                        /*
                         * Case: There is initial node and i didn't find the key
                         * Purpose: Check if the successor is the initial node. If he isn't just
                         * 			forward the message. If he is send him failure to find message
                         */
#ifdef DEBUG
                        printf("Node: %d Case 3\n",id);
#endif
                        if (answer.initial_node == successor){
                            sprintf(buffer_answer,"KEY NOT FOUND :(\n");
                            forward_message(buffer_answer, answer.initial_node_hostname, PORT_BASE + ring_size);
                        }else{
                            forward_message(buffer, successor_hostname, PORT_BASE+successor);
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
                        std::string new_message = std::string(buffer) + "," + std::to_string(id) + "," + my_hostname;
                        forward_message(new_message,successor_hostname,PORT_BASE+successor);

                        char * answer_buffer;
                        answer_buffer = wait_response_initial_node(PORT_BASE+ ring_size);
                        if (write(newsd,answer_buffer,strlen(answer_buffer)) != strlen(answer_buffer))
                            perror("write");
                    }
                }
                break;
            case DELETE_TYPE :
#ifdef DEBUG
                printf("Node: %d Type: %d Key: %s Initial node: %d Initial node name:%s\n",id,answer->type,answer->key_and_value.first,answer->initial_node,answer->initial_node_hostname);
#endif
                if (value = delete_file(answer)){
                    if (answer.initial_node != -1){
                        /*
                         * Case: There is initial node and i deleted the key
                         * Purpose: Answer to the initial node and he will answer to the client
                         */
#ifdef DEBUG
                        printf("Node: %d Case 1\n",id);
#endif
                        std::string buffer_answer = "KEY DELETED AT NODE " + my_hostname + " PORT " + std::to_string(PORT_BASE+id) + " ID " + std::to_string(id); 
                        forward_message(buffer_answer, answer.initial_node_hostname, PORT_BASE + ring_size);
                    }else{
                        /*
                         * Case: There is no initial node and i deleted the key
                         * Purpose: Answer to the client
                         */
#ifdef DEBUG
                        printf("Node: %d Case 2\n",id);
#endif
                        std::string buffer_answer = "KEY DELETED AT NODE " + my_hostname + " PORT " + std::to_string(PORT_BASE+id) + " ID " + std::to_string(id); 
                        if (write(newsd, buffer_answer.c_str(), buffer_answer.length()) != buffer_answer.length())
                            perror("write");
                    }
                }else{
                    if (answer.initial_node != -1){
                        /*
                         * Case: There is initial node and i didn't delete the key
                         * Purpose: Check if the successor is the initial node. If he isn't just
                         * 			forward the message. If he is send him failure to delete message
                         */
#ifdef DEBUG
                        printf("Node: %d Case 3\n",id);
#endif
                        if (answer.initial_node == successor){
                            sprintf(buffer_answer, "KEY NOT FOUND :(\n");
                            forward_message(buffer_answer, answer.initial_node_hostname, PORT_BASE + ring_size);
                        }else{
                            forward_message(buffer, successor_hostname, PORT_BASE+successor);
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
                        std::string new_message = std::string(buffer) + "," + std::to_string(id) + "," + my_hostname;
                        forward_message(new_message,successor_hostname,PORT_BASE+successor);

                        char * answer_buffer;
                        answer_buffer = wait_response_initial_node(PORT_BASE+ ring_size);
                        if (write(newsd,answer_buffer,strlen(answer_buffer)) != strlen(answer_buffer))
                            perror("write");
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
                if (answer.initial_node < id){
#ifdef DEBUG
                    printf("1st case\n");
#endif
                    for (int i=0; i<files.size(); i++){
                        if (files.at(i).first < answer.initial_node || files.at(i).first > id) {
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
                        if ((files.at(i).first > id)&&(files.at(i).first < answer.initial_node)){
                            printf("%d ",files.at(i).first);
                            sprintf(temp_buffer,"%d-%d-",files.at(i).first,files.at(i).second);
                            strcat(buffer,temp_buffer);
                            delete_vector.push_back(files.at(i).first);
                        }
                    }
                }
                for (int i=0; i < delete_vector.size(); i++) delete_file_from_hash(delete_vector.at(i));
                forward_message(buffer, answer.initial_node_hostname, PORT_BASE + answer.initial_node);

                break;
            case JOIN_RECEIVE :
#ifdef DEBUG
                printf("Node: %d Type: %d Receiving those files: %s\n",id,answer->type,answer->files);
#endif
                /*
                 * Inserts all the new files. This is the first thing that a joining node executes!
                 */
                if (answer.files == "") break;
                strcpy(buffer, answer.files.c_str());
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
                if (answer.files == "") break;
                strcpy(buffer,answer.files.c_str());
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
