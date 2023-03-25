#pragma once

#include <iostream>

#include "hashtable.hpp"
#include "item.hpp"




class Hypercube{

	private:

		HashTable * table;
		std::list<item *> *all_items;

	public:
		int k;	// dimension of projection - kD
		int N; 	// number of nearest neighbors to find
		int m;	// number of checked vectors to stop search
		int probes;	// number of checked neighbor nodes to stop search
		float R;	// search radius


		Hypercube(int k = 14, int N = 1, int m = 10, int probes = 2, int R = 10000,int w = 600);
		~Hypercube();
		void testprint(){
			table->print_table_cube();
		}

		void preprocess_item(item *p);
		void execute_query(item *q, std::string output_file);
		std::set<item *> * range_search(item * q,int radius){
			return table->range_search(q,this->N,this->k,this->m,this->probes,radius);
		};

		std::set<item *> * range_search_items(item * q){
			return table->range_search_items(q,this->k,this->m,this->probes);
		};
};
