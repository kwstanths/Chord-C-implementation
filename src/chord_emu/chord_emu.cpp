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

#include "ChordNode.hpp"

/**
	Function to compare twn chord nodes with their id's for sorting purposes
	@param first The first node
	@param second The second node
	@return True if first has lower id than second
*/
bool compare(const ChordNode& first, const ChordNode& second){
	return first.get_id() < second.get_id();
}

/**
	Pretty print the list of chord nodes for debugging purposes
	@param nodes The list of chord nodes
*/
void print_list(std::list<ChordNode> nodes){

	std::list<ChordNode>::iterator iterator;
	for(iterator = nodes.begin(); iterator != nodes.end(); ++iterator){
		printf("Node: %d Name: %s\n\tSuccessor:%d\t Predecessor:%d\n\tName: %s\t Name:%s\n\n",
			PORT_BASE+(*iterator).get_id(),
			(*iterator).get_hostname().c_str(),
			(*iterator).get_successor(),
			(*iterator).get_predecessor(),
			(*iterator).get_successor_hostname().c_str(),
			(*iterator).get_predecessor_hostname().c_str());
	}
};

/**
	A function that sets all the id's of the nodes(successor's and predecessors') 
	and the hostnames (successor's and predecessors's as well). Usually called after a sorting.
	@param nodes The chord nodes list
*/
void set_id_and_hostnames(std::list<ChordNode>* nodes){

	std::list<ChordNode>::iterator iterator, prev_node, next_node;
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

/**
	Check if a node already exists inside inside the node list
	@param nodes The nodes list
	@param nodehash The nodehash of a node
	@param 1 = exists, 0 = does not exist 
*/
int check_node_existence(std::list<ChordNode>* nodes, int nodehash){
		for(std::list<ChordNode>::iterator iterator=nodes->begin(); iterator != nodes->end(); ++iterator){
		if ((*iterator).get_id() == nodehash) return 1;
	}
	return 0;
}

/**
	Get the chord node object with a given node hash
	@param nodes The nodes list
	@param nodehash The nodehash of a node
	@return A chord node object
*/
ChordNode return_node(std::list<ChordNode>* nodes, int nodehash){
		for(std::list<ChordNode>::iterator iterator=nodes->begin(); iterator != nodes->end(); ++iterator){
		if ((*iterator).get_id() == nodehash) return *iterator;
	}

}


int main(int argc, char** argv){

	if (argc < 3){
		std::cout << "Usage:" << "./chord_emu <Number of initial nodes> <Ring size log>" << std::endl;
		return 0;
	}

	int number_of_nodes, M, ring_size;

	number_of_nodes = std::atoi(argv[1]);
	M = std::atoi(argv[2]);

	ring_size = std::pow(2, M);
	std::cout << "Nodes: " << number_of_nodes << std::endl;
	std::cout << "Ring size: " << ring_size << std::endl;

	/*
	 * Create a list of Chord_node elements and then sort it.
	 */

	std::list<ChordNode> nodes;
	for(int i=0; i<number_of_nodes; i++){
		ChordNode newNode = ChordNode(compute_sha1_hash(i, ring_size), ring_size);
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

	std::vector<std::thread> threads;
	for(std::list<ChordNode>::iterator iterator=nodes.begin(); iterator != nodes.end(); ++iterator){
		threads.push_back(std::thread(&ChordNode::listen_incoming_connections, iterator));
	}

	std::string line;
	int nodeid, nodehash;
	while(1){
		/*
		* Iterate forever accepting joins and departs
		*/
		std::cout << "Commands[JOIN, DEPART] > ";
		std::cin >> line;
		if (line == "JOIN") {
			std::cout << "New node ID: " ;
			std::cin >> nodeid;
			nodehash = compute_sha1_hash(nodeid, ring_size);
			std::cout << "Inserting node: " << nodeid << " with hash: " << nodehash << std::endl;

			if (check_node_existence(&nodes, nodehash)) std::cout << "Node already exists..." << std::endl;
			else {
				ChordNode newNode = ChordNode(nodehash, ring_size);
				newNode.set_hostname("localhost");
				nodes.push_back(newNode);
				nodes.sort(compare);
				set_id_and_hostnames(&nodes);
				threads.push_back(std::thread(&ChordNode::listen_incoming_connections, return_node(&nodes,nodehash)));
				print_list(nodes);
				/* set_id_and_hostnames changes the nodes, the local variable newNode is the new node anynmore */
				newNode = return_node(&nodes, nodehash);
				std::string message = "JOIN TRANSFER," + std::to_string(newNode.get_id()) + "," + newNode.get_hostname();
				forward_message(message, newNode.get_successor_hostname(), PORT_BASE + newNode.get_successor());
				usleep(2000000);
			}
		} else if (line == "DEPART"){
			std::cout << "Node port: " ;
			std::cin >> nodeid;
			nodehash = nodeid - PORT_BASE;
			std::cout << "Removing node with hash: " << nodehash << std::endl;

			if (check_node_existence(&nodes, nodehash) == 0) std::cout << "Node doesn't exist..." << std::endl;
			else {
				ChordNode node_to_remove = return_node(&nodes, nodehash);
				std::string message = "DEPART TRANSFER";
				forward_message(message, node_to_remove.get_hostname(), PORT_BASE + node_to_remove.get_id());
				usleep(2000000);

				nodes.remove(node_to_remove);
				set_id_and_hostnames(&nodes);
				print_list(nodes);
			}
		}
	}


	for(int i =0; i<nodes.size(); i++)
		threads.at(i).join();
}

