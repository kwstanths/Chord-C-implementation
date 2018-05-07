#ifndef __ChordNode_hpp__
#define __ChordNode_hpp__

#include <string>
#include <vector>
#include <utility>

#include "lib.hpp"

class ChordNode {
	/*
	* Each node corresponds to a class Chord_node. Provide the node's id and the ring size in the constructor
	* In the end of this class you can see the private fields of the node
	*/

	public:
		ChordNode(int new_id, int new_ring_size): id(new_id), ring_size(new_ring_size){};

		/*
		 * Methods for getting and returning private data. Description for those is not necessary cause they do
		 * what their name says.
		 */

		int get_id() const { return id; }
		
		int get_successor() const { return successor; };

		int get_predecessor() const { return predecessor; };

		std::string get_hostname() const { return my_hostname; }

		std::string get_successor_hostname() const { return successor_hostname; };

		std::string get_predecessor_hostname() const { return predecessor_hostname; };

		void set_successor(int succ) { successor = succ; };

		void set_predecessor(int pred) { predecessor = pred; };

		void set_hostname(std::string hostname){
			my_hostname = hostname;
		};
		
		void set_successor_hostname(std::string hostname){
			successor_hostname = hostname;
		};

		void set_predecessor_hostname(std::string hostname) {
			predecessor_hostname = hostname;
		};

		int insert(Command command);

		int query(Command command);

		int delete_file(Command command);

		/*
		* These two function are of great help when i need to insert or delete a file when i only know its hash
		*/
		int delete_file_from_hash(int hash_value);

		int insert_file_from_hash(int hash_value, int file_value);

		int listen_incoming_connections();

		bool operator == (const ChordNode& a) {
			return a.get_id() == id;
		}

	private:
		/*
		 * Hold the size of the ring, the id's and hostnames of the successor and the predecessor
		 * Also hold a vector of files
		 */
		int ring_size;
		int id,successor,predecessor;
		std::string my_hostname, successor_hostname, predecessor_hostname;
		std::vector<std::pair<int,int>> files;

};


#endif
