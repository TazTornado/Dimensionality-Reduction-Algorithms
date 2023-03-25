#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "utilities.hpp"
#include "hypercube.hpp"




Hypercube::Hypercube(int k, int N, int m, int probes, int R,int w) : k(k), N(N), m(m), probes(probes), R(R) {

	this->k = k;
	this->N = N;
	this->m = m;
	this->probes = probes;
	this->R = R;
	this->table = new HashTable(k,w);

	this->all_items = new std::list<item *>();


}


Hypercube::~Hypercube(){
	delete this->table;

	for(auto const &i : *(this->all_items))
		delete i;
	delete this->all_items;
}

void Hypercube::preprocess_item(item *p){

	this->table->initialize_v((unsigned int) p->get_length(), this->k);
	table->insert_element(p,this->k);
	this->all_items->push_back(p);
}



void Hypercube::execute_query(item *q, std::string outputfile){


	std::set<std::string> * checked_positions	= new std::set<std::string>();
	clock_t all_items_start = clock();
	std::map<float, std::string> true_neighbors;
	float temp_distance;
	for(auto const &i : *(this->all_items)){
		if(true_neighbors.size() < (long unsigned int) N){
			true_neighbors.insert(std::pair<float,std::string>(q->item_distance(2,i),i->get_id()));
		}
		else{
			temp_distance = q->item_distance(2,i);
			if(true_neighbors.rbegin()->first < temp_distance)
				continue;
			true_neighbors.insert(std::pair<float,std::string>(temp_distance,i->get_id()));
			true_neighbors.erase(true_neighbors.rbegin()->first);
		}
	}

	clock_t all_items_end = clock();
	double all_items_seconds_passed = (double)(all_items_end - all_items_start) / CLOCKS_PER_SEC;

	//this just calls hashtable to do the dirty work
	std::multimap <float,item*> * nearest_neighbors = new std::multimap <float,item*>();
	std::set<item *> * in_range;



	double nn_time;

	table->execute_query(this->N, this->k, this->m, this->probes, this->R, q, nearest_neighbors,checked_positions,&nn_time); //maybe we need all items?



	std::ofstream outfile;
	outfile.open(outputfile, std::ios_base::app);
	outfile << "Query: " << q->get_id() << std::endl;

	std::multimap<float, item *>::iterator i;
	std::map<float, std::string >::iterator j;
	int count = 0;

	i = nearest_neighbors->begin();
	j = true_neighbors.begin();


	while(i != nearest_neighbors->end()){
		count++;
		outfile << "Nearest neighbor - " << count << ": " << i->second->get_id() << std::endl;
		outfile << "DistanceHypercube: " << i->first << std::endl;
		outfile << "DistanceTrue: " << j->first << std::endl;

		double dist_eval = (double) (i->first / j->first);
		if (dist_eval > distance_max){
			distance_max = dist_eval;
			max_dist_query = q->get_id();
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
		max_time_query = q->get_id();
	}
	if(time_eval > 1.0)
		bad_time_counter++;

	time_mean += time_eval;


	outfile << "tHypercube: " << nn_time << std::endl;
	outfile << "tTrue: " << all_items_seconds_passed << std::endl;
	outfile << "R-near neighbors: " << std::endl;

	in_range = table->positions_checked_range_search(q,this->m,this->R,checked_positions);
	std::set<item *>::iterator z;

	for(z = in_range->begin(); z !=in_range->end(); z++){
		outfile << (*z)->get_id() << std::endl;
	}


	delete nearest_neighbors;
	delete in_range;
	delete checked_positions;

}
