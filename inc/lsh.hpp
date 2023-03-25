#pragma once

#include <climits>
#include "item.hpp"
#include "hashtable.hpp"
#include "tree.hpp"


#define M (UINT_MAX - 5)

class LSH{
	private:


		HashTable **hashtables;
		std::list<item *> *all_items;

	public:
		std::vector <int> r;
		int k, L, N;
		float R;

		LSH(int hashtable_size, int k = 4, int L = 5, int N = 1, float R = 10000.0,int w = 600);
		~LSH();
		void preprocess_item(item *i);

		std::set<item *> * range_search_items(item * query){
			std::set<item *> * return_set= new std::set<item *>();
			for(int i = 0 ; i < this->L ; i++){
				hashtables[i]->range_search_items_lsh(query,k,&r,return_set);
			}
			return return_set;
		}


		std::set<item *> * range_search(item * query,int ldistance,float radius){
			std::set<item *> * return_set = new std::set<item *>();
			for(int i = 0 ; i<k ; i++){
				hashtables[i]->range_search_lsh(query,this->k,&(this->r),ldistance,radius,return_set);
			}
			return return_set;
		}
		void execute_query(int ldistance,item * query,std::string outputfile);
		//void print_all_tables();


};
