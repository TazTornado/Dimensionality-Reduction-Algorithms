#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <set>
#include <list>
#include <map>
#include <iterator>
#include <fstream>
#include "error.hpp"
#include "hashtable.hpp"
#include "lsh.hpp"
#include "utilities.hpp"



LSH::LSH(int hashtable_size, int k, int L, int N, float R,int w) : k(k), L(L), N(N), R(R){

	this->hashtables = (HashTable **)malloc(L * sizeof(HashTable *));
	for (int i = 0; i < L; i++){
		this->hashtables[i] = new HashTable(hashtable_size, this->k,w);
	}


	for(int i = 0; i < k; i++){
		this->r.push_back((int)euclidean_modulo(uniform_random_generator(DEFAULT_WINDOW), M));
	}

	this->all_items = new list<item *>();
}



LSH::~LSH(){
	for(int i = 0; i < this->L; i++){
		delete hashtables[i];
	}
	free(hashtables);

	// this->all_items->clear();
	for(auto const &i : *(this->all_items)){
		delete i;
	}
	delete this->all_items;
}







void LSH::preprocess_item(item *p){

	for(int i = 0; i < this->L; i++){
		this->hashtables[i]->initialize_v((unsigned int)p->get_length(), this->k);
		hashtables[i]->insert_element(p,this->k, &(this->r));
	}
	this->all_items->push_back(p);
}



void LSH::execute_query(int ldistance,item * query, std::string outputfile){

	clock_t all_items_start = clock();
	std::map<float, std::string> true_neighbors;
	float temp_distance;
	for(auto const &i : *(this->all_items)){
		if(true_neighbors.size() < (long unsigned int) N){
			true_neighbors.insert(std::pair<float,std::string>(query->item_distance(2,i),i->get_id()));
		}
		else{
			temp_distance = query->item_distance(2,i);
			if(true_neighbors.rbegin()->first < temp_distance)
				continue;
			true_neighbors.insert(std::pair<float,std::string>(temp_distance,i->get_id()));
			true_neighbors.erase(true_neighbors.rbegin()->first);
		}
	}

	clock_t all_items_end = clock();
	double all_items_seconds_passed = (double)(all_items_end - all_items_start) / CLOCKS_PER_SEC;


	std::multimap <float,item*> * nearest_neighbors = new std::multimap <float,item*>();
	std::set<std::string> * avoid_duplicates_in_nn = new std::set<std::string>();
	std::set<item *> * in_range = new std::set<item*>();

	// clock_t start = clock();
	double nn_time = 0.0;
	double table_time;
	for(int a = 0 ; a< this->L; a++){

		hashtables[a]->execute_query(this->N, this->k, &(this->r), ldistance, this->R, query,  nearest_neighbors, avoid_duplicates_in_nn, in_range, &table_time);
		nn_time += table_time;
	}



	std::ofstream outfile;
	outfile.open(outputfile, std::ios_base::app);
	outfile << "Query: " << query->get_id() << std::endl;

	std::multimap<float, item *>::iterator i;
	std::map<float, string >::iterator j;
	int count = 0;

	i = nearest_neighbors->begin();
	j = true_neighbors.begin();


	while(i != nearest_neighbors->end()){
		count++;
		outfile << "Nearest neighbor - " << count << ": " << i->second->get_id() << std::endl;
		outfile << "DistanceLSH: " << i->first << std::endl;
		outfile << "DistanceTrue: " << j->first << std::endl;

		double dist_eval = (double) (i->first / j->first);
		if (dist_eval > distance_max){
			distance_max = dist_eval;
			max_dist_query = query->get_id();
		}
		if(dist_eval > 2.0)
			bad_dist_counter++;

		distance_mean += dist_eval;

		i++;
		j++;
	}

	double time_eval = nn_time / all_items_seconds_passed;
	if (time_eval > time_max){
		time_max = time_eval;
		max_time_query = query->get_id();
	}
	if(time_eval > 1.0)
		bad_time_counter++;

	time_mean += time_eval;


	outfile << "tLSH: " << nn_time << std::endl;
	outfile << "tTrue: " << all_items_seconds_passed << std::endl;
	std::set<item *>::iterator z;
	outfile << "R-near neighbors: " << std::endl;
	for(z = in_range->begin(); z !=in_range->end(); z++){
		outfile << (*z)->get_id() << std::endl;
	}

	delete nearest_neighbors;
	delete avoid_duplicates_in_nn;
	delete in_range;

	outfile.close();
}


// void LSH::print_all_tables(){
//
// 	for(int i = 0; i < this->L; i++){
// 		cout << "Printing HashTable " << i << endl;
// 		cout << "=========================" << endl;
// 		this->hashtables[i]->print_table();
// 		cout << endl << "===================================================================" << endl << endl;
// 	}
// }
