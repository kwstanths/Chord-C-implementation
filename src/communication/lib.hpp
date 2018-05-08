#ifndef __lib_hpp__
#define __lib_hpp__

#include <string>
#include <vector>

/*
	Holds the necessary info for a request
*/
struct Command {
	int type;
	std::pair<std::string, int> key_and_value;
	int initial_node;
	std::string initial_node_hostname, files;
};

void convert_int_to_char_array(unsigned char * buffer,int number);

int int_length(int number);

int collect_hash_output(unsigned char* buffer,int ring_size);

int compute_sha1_hash(int not_hashed, int ring_size);

int compute_sha1_hash(const char * not_hashed,int ring_size);

Command parse_the_command(char * buffer);

void forward_message(std::string message, std::string hostname, int port);

char * wait_response_initial_node(int port);

void print_node_files(std::vector<std::pair<int,int>>);

#endif
