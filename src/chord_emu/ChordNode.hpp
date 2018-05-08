#ifndef __ChordNode_hpp__
#define __ChordNode_hpp__

#include <string>
#include <vector>
#include <utility>

#include "lib.hpp"

/**
	Each chord node corresponds to a class ChordNode. Provide the node's id and the ring size in the constructor.
*/
class ChordNode {

	public:
		ChordNode(int new_id, int new_ring_size): id(new_id), ring_size(new_ring_size){};

		int get_id() const { return id; }

		int get_successor() const { return successor; };
		
		int get_predecessor() const { return predecessor; };

		std::string get_hostname() const { return my_hostname; }

		std::string get_successor_hostname() const { return successor_hostname; };

		std::string get_predecessor_hostname() const { return predecessor_hostname; };

		void set_successor(int succ) { successor = succ; };

		void set_predecessor(int pred) { predecessor = pred; };

		void set_hostname(std::string hostname) { my_hostname = hostname; };
		
		void set_successor_hostname(std::string hostname) { successor_hostname = hostname; };

		void set_predecessor_hostname(std::string hostname) { predecessor_hostname = hostname; };

    	/**
			Insert operation in the node
			@param command The struct with the necessary info
			@return 1 = insertion completed, 0 = not inserted
     	*/
		int insert(Command command);

		/*
			Query opeartion in the node
			@param command The struct with the necessary info
			@return 1 = found, 0 = not found		
		*/
		int query(Command command);

		/*
			Delete operation in the node
			@param command The struct with the necessary info
			@return 1 = deleted, 0 = not deleted
		*/
		int delete_file(Command command);
		
		/**
			Returns the index position of a file in a vector of files
			@param files The vector of files
			@param key The file key - hashvalue of the file to find
			@param -1 = does not exist, else the index
		*/
		int search_in_a_vector_of_files(std::vector<std::pair<int,int>> files,int key);

		/**
			Delete a file with known hash value
			@param hash_value The hash value of the file
			@return 1 = deleted, else -1
 		*/
		int delete_file_from_hash(int hash_value);

		/**
			Insert a file when when known the hash value. Used when files ared moved
			between the nodes
			@param hash_value The hash value of the file
			@param file_value The value of the file
			@return 1 = inserted, -1 not inserted
		*/
		int insert_file_from_hash(int hash_value, int file_value);

		/**
			Starts listening to requests
			@return Returns immidiately when not able to open server socket, else returns
			when the appropriate request is received
		*/
		int listen_incoming_connections();

		bool operator == (const ChordNode& a) {
			return a.get_id() == id;
		}

	private:
		/* The size of the ring */
		int ring_size;
		
		/* Id's for the current the next and the previous nodes */
		int id,successor,predecessor;
		
		/* The hostname of the current the next and the previous node */
		std::string my_hostname, successor_hostname, predecessor_hostname;

		/* Holds the files in this node */
		std::vector<std::pair<int,int>> files;
};


#endif
