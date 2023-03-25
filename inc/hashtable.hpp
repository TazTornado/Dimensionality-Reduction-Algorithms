#pragma once

#include <iostream>
#include <map>
#include "tree.hpp"

#define DEFAULT_WINDOW 600


class HashTable{

	private:
		unsigned int size;
		int window;
		std::vector<item *> v;
		std::vector<float> t;
		void *buckets;
		std::vector<std::map<int, unsigned char> *> *f = NULL;

	public:
		HashTable(int size, int k,int w);
		HashTable(int size,int w);
		~HashTable();

		int get_size(){ return this->size; };
		void print_table();
		void print_table_cube();

		int H(item *p, int i);
		int ID(item *p, int k, std::vector<int> *r);
		void initialize_v(unsigned int v_length, int k);




		/////////////////////////////lsh//////////////////////////
		void insert_element(item *p, int k, std::vector<int> *r);

		void execute_query(int N, int k, std::vector<int> *r, int ldistance, float radius, item *query,
			std::multimap<float, item *> *nearest_neighbors,std::set<std::string> * avoid_duplicates_in_nn ,std::set<item *> *neighbors_in_range, double *nn_time);


		void range_search_items_lsh(item * query,int k,std::vector<int> *r,std::set<item *> *return_set);
		void range_search_lsh(item * q, int k, std::vector<int> *r, int ldistance,float radius ,std::set<item *> * neighbors_in_range);
		void bucket_ready_range_search(item * q, int ldistance,float radius,std::set<item *> * in_range , std::list<item *> * bucket);

		////////////////////cube/////////////////////

		void insert_element(item *p, int k);

		void execute_query(int N, int k, int m, int probes, float radius, item *query, std::multimap <float,item*> * nearest_neighbors ,std::set<std::string> * checked_positions, double * nn_time);

		std::set<item *> * range_search(item * q,int N, int k, int m, int probes,int radius);
		std::set<item *> * range_search_items(item * q, int k, int m , int probes);
		std::set<item *> * positions_checked_range_search(item * q,int m,int radius, std::set<std::string> * positions_checked);




};
