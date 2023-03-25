#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include <fstream>

#include "error.hpp"
#include "kmeans.hpp"
//#include "utilities.hpp"

using namespace std;

Kmeans::Kmeans(unsigned int k_medians, string method, int L_hashtables, int k_hash_functions, int max_points, int probes, int cube_dimensions, bool complete) : k(k_medians), method(method), L_hashtables(L_hashtables), k_hash_functions(k_hash_functions), max_points(max_points), probes(probes), cube_dimensions(cube_dimensions), complete_print_mode(complete){
	this->clusters = new vector<Cluster *>();
	this->all_points = new map<item *, ItemInfo *>();
	this->points_reassigned = 0;
	this->centroids_changed	= 0;
}


Kmeans::~Kmeans(){
	for(auto const &i : *(this->all_points)){
		delete i.first;
		delete i.second;
	}
	delete this->all_points;

	for(auto const &i : *(this->clusters))
		delete i;
	delete this->clusters;
}

///////////////////////////////////////////////////////			INITIALIZATION OF CENTROIDS			///////////////////////////////////////////////////////

void Kmeans::initialize_centroids(){
	if(this->all_points->empty()){
		error_handler(CLUSTER_EMPTY_SET, "Cluster initialization");
		return;
	}

	/* choose first centroid randomly */
	srand(time(NULL));
	map<item *, ItemInfo *>::iterator i = this->all_points->begin();
	advance(i, rand() % (int) this->all_points->size());
	vector<float> *centr_elements = new vector<float>(*(i->first->get_elements()));
	item *centroid = new item(centr_elements->size(), centr_elements, "");

	this->clusters->push_back(new Cluster(centroid));



	for(unsigned int i = 0; i < this->k - 1; i++){

		/* calculate min distance for all items */
		vector<pair<item *, float>> *distances = new vector<pair<item *, float>>();
		this->calculate_distance("", distances);


		/* choose next centroid with weighted randomness */
		item *new_centroid = this->choose_centroid(distances);


		/* add the new cluster in the set */
		Cluster *new_cluster = new Cluster(new_centroid);
		this->clusters->push_back(new_cluster);





		delete distances;
	}

	if(this->clusters->size() != this->k){
		cout << "Cluster init =>wrong clusters size" << endl;
		error_handler(CLUSTER_INIT_WRONG_SIZE, "Cluster initialization");
	}
}



item *Kmeans::choose_centroid(vector<pair<item *, float>> *min_distances){

	/* find max D(i) in min_distances */
	float max_distance = 0.0;
	for(auto const &i : *min_distances){
		if(i.second > max_distance)
			max_distance = i.second;
	}

	/* calculate P(i) from D(i) in min_distances */
	vector<float> P_values;

	for(long unsigned int i = 0; i < min_distances->size(); i++){
		float p_i;
		if(i == 0){
			p_i = min_distances->at(i).second / max_distance;
			// cout << "p(i) of item " << min_distances->at(i).first->get_id() << ": " << p_i << endl;
			P_values.push_back(p_i);

		} else {
			p_i = (min_distances->at(i).second / max_distance) + P_values.at(i - 1);
			// cout << "p(i) of item " << min_distances->at(i).first->get_id() << ": " << p_i << endl;
			P_values.push_back(p_i);
		}
	}

	/* choose randomly a number in [0.0, max(P(i))] */
	float random_value = uniform_random_generator(P_values.back());

	/* find which point corresponds to random number and return it */
	vector<float>::iterator result = upper_bound(P_values.begin(), P_values.end(), random_value);
	int result_index = result - P_values.begin();
	vector<float> *new_point = new vector<float>(*(min_distances->at(result_index).first->get_elements()));
	item *new_item = new item(new_point->size(), new_point, "");

	return new_item;
}



void Kmeans::calculate_distance(string id, vector<pair<item *, float>> *distances_to_calculate){
	if(id == ""){

		for(auto const &i : *(this->all_points)){

			/* make sure this point isn't a centroid already */
			bool in_clusters = false;
			for(auto const &j : *(this->clusters)){
				if(j->centroid == i.first){
					in_clusters = true;
					break;
				}

			}

			/* determine minimum distance from a centroid */
			if(in_clusters == false){
				for(auto const &j : *(this->clusters)){
					float distance_from_centroid = i.first->item_distance_squared(2, j->centroid);

					/* check if there's already a min distance for this point */
					bool already_calculated = false;
					for(long unsigned int k = 0; k < distances_to_calculate->size(); k++){
						if(distances_to_calculate->at(k).first == i.first){
							already_calculated = true;

							if (distance_from_centroid < distances_to_calculate->at(k).second){
								// distances_to_calculate->erase(k.first);
								// distances_to_calculate->insert(pair<float, item *>(distance_from_centroid, i.first));
								distances_to_calculate->at(k).second = distance_from_centroid;
							}
							break;
						}
					}
					if(already_calculated == false)
						distances_to_calculate->push_back(pair<item *, float>(i.first, distance_from_centroid));
				}
			}
		}
	}
}


///////////////////////////////////////////////////////			K-MEANS MAIN METHODS		///////////////////////////////////////////////////////

void Kmeans::insert_item(item *p){
	ItemInfo *info = new ItemInfo();
	info->assigned_to_cluster = false;
	this->all_points->insert(pair<item *, ItemInfo *>(p, info));
}


void Kmeans::create_clusters(string method){

	if(method == "classic" || method == "Classic"){
		cout << "Calling Lloyd" << endl;
		this->lloyd_clustering();

	} else if(method == "lsh" || method == "LSH"){

		LSH * lsh_object = new LSH(this->all_points->size()/8,this->k_hash_functions,this->L_hashtables,1,1000,600);
		for(auto const &i:*(this->all_points)){
			lsh_object->preprocess_item(i.first);
		}
		this->reverse_assignment_lsh(lsh_object);


	} else if(method == "hypercube" || method == "Hypercube"){

		Hypercube * cube_object = new Hypercube(14,1,1000,56,10000,600);
		for(auto const &i:*(this->all_points)){
			cube_object->preprocess_item(i.first);
		}
		this->reverse_assignment_cube(cube_object);

	} else {
		error_handler(CLUSTER_BAD_METHOD, method + " not recognized");
	}
}



void Kmeans::lloyd_clustering(){

    /* repeat clustering until fewer than half the points or fewer than half the centroids change */



    do{

        /* initialize global info */
        this->points_reassigned = 0;
        this->centroids_changed = 0;

        this->assign_points_to_clusters();
        this->update_centroids();


    } while((long unsigned int) this->points_reassigned > this->all_points->size() / 2 ||
            (long unsigned int) this->centroids_changed > this->clusters->size() / 2);

}

void Kmeans::reverse_assignment_lsh(LSH *lsh_object)
{
	double min_distance_between_centroids;
	double temp_comparison;

	// we get min_distance_between_centroids between any 2 centroids

	int changed_points;
	double radius;
	int total_points = 0;

	double temp_distance;
	std::set<item *> *temp;
	std::vector<std::set<item *> *> cluster_checks;
	char **in_range = (char **)malloc(clusters->size() * sizeof(char *));

	total_points = this->all_points->size();
	do
	{ // while((long unsigned int) this->centroids_changed > this->clusters->size() / 2);
		min_distance_between_centroids = this->clusters->at(0)->centroid->item_distance(2, this->clusters->at(1)->centroid);
		for (long unsigned int a = 0; a < clusters->size() - 1; a++)
		{
			for (long unsigned int b = a + 1; b < clusters->size(); b++)
			{
				temp_comparison = this->clusters->at(a)->centroid->item_distance(2, this->clusters->at(b)->centroid);
				if (temp_comparison < min_distance_between_centroids)
					min_distance_between_centroids = temp_comparison;
			}
		}
		// changed_points=0;
		radius = min_distance_between_centroids / 2;
		// int tt = 0;
		for (long unsigned int i = 0; i < clusters->size(); i++)
		{
			temp = lsh_object->range_search_items(this->clusters->at(i)->centroid);
			cluster_checks.push_back(temp);
			in_range[i] = (char *)calloc(temp->size(), sizeof(char));

			// printf("%d temp_size\n",temp->size() );
		}
		do
		{
			changed_points = 0;

			for (long unsigned int i = 0; i < clusters->size(); i++)
			{
				temp = cluster_checks[i];
				ItemInfo *tempItemInfo;
				int index = 0;
				for (auto const &a : *temp)
				{
					if (in_range[i][index] == 0)
					{
						temp_distance = a->item_distance(2, this->clusters->at(i)->centroid);
					}
					else
					{
						index++;
						continue;
					}
					if (temp_distance < radius)
					{
						tempItemInfo = this->all_points->at(a);
						if (tempItemInfo->assigned_to_cluster)
						{

							if (tempItemInfo->distance_from_centroid > temp_distance)
							{
								this->clusters->at(tempItemInfo->cluster)->items_in_cluster.remove(a);
								this->clusters->at(i)->items_in_cluster.push_back(a);
								tempItemInfo->distance_from_centroid = temp_distance;
								tempItemInfo->second_closest_centroid = tempItemInfo->cluster;
								tempItemInfo->cluster = i;
								changed_points++;
							}
						}
						else
						{
							// tt++;

							this->clusters->at(i)->items_in_cluster.push_back(a);
							tempItemInfo->distance_from_centroid = temp_distance;
							tempItemInfo->cluster = i;
							tempItemInfo->second_closest_centroid = tempItemInfo->cluster;
							tempItemInfo->assigned_to_cluster = true;
							changed_points++;
						}
						in_range[i][index] = 1;
					}

					index++;
				}
			}

			radius = radius * 7;
			// printf("%d total changed_points \n",changed_points );
		} while (changed_points > total_points * 0.5);
		// printf("%d total \n",tt );

		double min_distance;
		int closest_cluster;
		// int dd=0;
		for (auto const &a : *(this->all_points))
		{
			if (!(a.second->assigned_to_cluster))
			{
				// dd++;

				min_distance = clusters->at(0)->centroid->item_distance(2, a.first);
				closest_cluster = 0;
				for (long unsigned int b = 1; b < clusters->size(); b++)
				{
					temp_distance = clusters->at(b)->centroid->item_distance(2, a.first);
					if (temp_distance < min_distance)
					{
						min_distance = temp_distance;
						closest_cluster = b;
					}
				}

				a.second->distance_from_centroid = min_distance;
				a.second->cluster = closest_cluster;
				a.second->second_closest_centroid = closest_cluster;
				this->clusters->at(closest_cluster)->items_in_cluster.push_back(a.first);
			}
			a.second->assigned_to_cluster = false;
		}
		// printf("%d ddtotal \n",dd );
		for (long unsigned int i = 0; i < clusters->size(); i++)
		{
			delete cluster_checks[i];
			// free(in_range[i]);
		}

		cluster_checks.clear();
		this->centroids_changed = 0;
		this->update_centroids();
		if ((long unsigned int)this->centroids_changed > this->clusters->size() / 2)
		{
			for (long unsigned int i = 0; i < clusters->size(); i++)
			{
				for (auto const &c : *(this->clusters))
				{
					c->items_in_cluster.clear();
				}
			}
		}
	} while ((long unsigned int)this->centroids_changed > this->clusters->size() / 2);
	free(in_range);
}

void Kmeans::reverse_assignment_cube(Hypercube *cube_object)
{
	double min_distance_between_centroids;
	double temp_comparison;

	// we get min_distance_between_centroids between any 2 centroids

	int changed_points;
	double radius;
	int total_points = 0;

	double temp_distance;
	std::set<item *> *temp;
	std::vector<std::set<item *> *> cluster_checks;
	char **in_range = (char **)malloc(clusters->size() * sizeof(char *));

	total_points = this->all_points->size();
	do
	{ // while((long unsigned int) this->centroids_changed > this->clusters->size() / 2);
		min_distance_between_centroids = this->clusters->at(0)->centroid->item_distance(2, this->clusters->at(1)->centroid);
		for (long unsigned int a = 0; a < clusters->size() - 1; a++)
		{
			for (long unsigned int b = a + 1; b < clusters->size(); b++)
			{
				temp_comparison = this->clusters->at(a)->centroid->item_distance(2, this->clusters->at(b)->centroid);
				if (temp_comparison < min_distance_between_centroids)
					min_distance_between_centroids = temp_comparison;
			}
		}
		// changed_points=0;
		radius = min_distance_between_centroids / 2;
		// int tt = 0;
		for (long unsigned int i = 0; i < clusters->size(); i++)
		{
			temp = cube_object->range_search_items(this->clusters->at(i)->centroid);
			cluster_checks.push_back(temp);
			in_range[i] = (char *)calloc(temp->size(), sizeof(char));
			// printf("%d temp_size\n",temp->size() );
		}
		do
		{
			changed_points = 0;

			for (long unsigned int i = 0; i < clusters->size(); i++)
			{
				temp = cluster_checks[i];
				ItemInfo *tempItemInfo;
				int index = 0;
				for (auto const &a : *temp)
				{
					if (in_range[i][index] == 0)
					{
						temp_distance = a->item_distance(2, this->clusters->at(i)->centroid);
					}
					else
					{
						index++;
						continue;
					}
					if (temp_distance < radius)
					{
						tempItemInfo = this->all_points->at(a);
						if (tempItemInfo->assigned_to_cluster)
						{

							if (tempItemInfo->distance_from_centroid > temp_distance)
							{
								this->clusters->at(tempItemInfo->cluster)->items_in_cluster.remove(a);
								this->clusters->at(i)->items_in_cluster.push_back(a);
								tempItemInfo->distance_from_centroid = temp_distance;
								tempItemInfo->second_closest_centroid = tempItemInfo->cluster;
								tempItemInfo->cluster = i;
								changed_points++;
							}
						}
						else
						{
							// tt++;

							this->clusters->at(i)->items_in_cluster.push_back(a);
							tempItemInfo->distance_from_centroid = temp_distance;
							tempItemInfo->cluster = i;
							tempItemInfo->second_closest_centroid = tempItemInfo->cluster;
							tempItemInfo->assigned_to_cluster = true;
							changed_points++;
						}
						in_range[i][index] = 1;
					}

					index++;
				}
			}

			radius = radius * 7;
			// printf("%d total changed_points \n",changed_points );
		} while (changed_points > total_points * 0.5);
		// printf("%d total \n",tt );

		double min_distance;
		int closest_cluster;
		// int dd=0;
		for (auto const &a : *(this->all_points))
		{
			if (!(a.second->assigned_to_cluster))
			{
				// dd++;

				min_distance = clusters->at(0)->centroid->item_distance(2, a.first);
				closest_cluster = 0;
				for (long unsigned int b = 1; b < clusters->size(); b++)
				{
					temp_distance = clusters->at(b)->centroid->item_distance(2, a.first);
					if (temp_distance < min_distance)
					{
						min_distance = temp_distance;
						closest_cluster = b;
					}
				}

				a.second->distance_from_centroid = min_distance;
				a.second->cluster = closest_cluster;
				a.second->second_closest_centroid = closest_cluster;
				this->clusters->at(closest_cluster)->items_in_cluster.push_back(a.first);
			}
			a.second->assigned_to_cluster = false;
		}
		// printf("%d ddtotal \n",dd );
		for (long unsigned int i = 0; i < clusters->size(); i++)
		{
			delete cluster_checks[i];
			delete in_range[i];
		}

		cluster_checks.clear();
		this->centroids_changed = 0;
		this->update_centroids();
		if ((long unsigned int)this->centroids_changed > this->clusters->size() / 2)
		{
			for (long unsigned int i = 0; i < clusters->size(); i++)
			{
				for (auto const &c : *(this->clusters))
				{
					c->items_in_cluster.clear();
				}
			}
		}
	} while ((long unsigned int)this->centroids_changed > this->clusters->size() / 2);
}

// void Kmeans::reverse_assignment_lsh(LSH * lsh_object){
// 	float min_distance_between_centroids;
// 	min_distance_between_centroids = this->clusters->at(0)->centroid->item_distance(2,this->clusters->at(1)->centroid);
// 	float temp_comparison;
// 	for(long unsigned int a=0;a<clusters->size()-1;a++){
// 		for(long unsigned int b=a+1;b<clusters->size();b++){
// 			temp_comparison = this->clusters->at(a)->centroid->item_distance(2,this->clusters->at(b)->centroid);
// 			if(temp_comparison < min_distance_between_centroids)
// 				min_distance_between_centroids = temp_comparison;
// 		}
// 	}
// 	//we get min_distance_between_centroids between any 2 centroids


// 	int changed_points=0;

// 	int total_points=0;
// 	float radius = min_distance_between_centroids/2;
// 	float temp_distance;
// 	std::set<item *> * temp;
// 	std::vector<std::set<item *> *> cluster_checks;
// 	for (long unsigned int i = 0; i < clusters->size();i++) {
// 		cluster_checks.push_back(lsh_object->range_search_items(this->clusters->at(i)->centroid));
// 	}
// 	total_points = this->all_points->size();
// 	do{
// 		do{
// 			changed_points = 0;

// 			for (long unsigned int i = 0; i < clusters->size();i++) {

// 				temp = cluster_checks[i];
// 				ItemInfo * tempItemInfo;
// 				for(auto const &a:*temp){
// 					if(this->all_points->find(a) != this->all_points->end()){
// 						temp_distance = a->item_distance(2,this->clusters->at(i)->centroid);
// 						if(temp_distance < radius){
// 							tempItemInfo = this->all_points->at(a);
// 							if(tempItemInfo->assigned_to_cluster){

// 								if(tempItemInfo->distance_from_centroid > temp_distance){
// 									this->clusters->at(tempItemInfo->cluster)->items_in_cluster.remove(a);
// 									this->clusters->at(i)->items_in_cluster.push_back(a);
// 									tempItemInfo->distance_from_centroid = temp_distance;
// 									tempItemInfo->second_closest_centroid =tempItemInfo->cluster;
// 									tempItemInfo->cluster = i;
// 									changed_points++;
// 								}
// 							}
// 							else{
// 								this->clusters->at(i)->items_in_cluster.push_back(a);
// 								tempItemInfo->distance_from_centroid = temp_distance;
// 								tempItemInfo->cluster = i;
// 								tempItemInfo->second_closest_centroid =tempItemInfo->cluster;
// 								tempItemInfo->assigned_to_cluster = true;
// 								changed_points++;
// 							}
// 						}
// 					}
// 					else{
// 						cout<< "COULD NOT FIND POINT" << std::endl;
// 					}
// 					temp->erase(a);
// 				}

// 			}

// 			radius = radius*2;
// 		}while(changed_points > total_points/2);
// 		float min_distance;
// 		int closest_cluster;
// 		for(auto const &a:*(this->all_points)){
// 			if(!(a.second->assigned_to_cluster)){
// 				min_distance = clusters->at(0)->centroid->item_distance(2,a.first);
// 				closest_cluster = 0;
// 				for(long unsigned int b = 1 ; b < clusters->size(); b++){
// 					temp_distance = clusters->at(b)->centroid->item_distance(2,a.first);
// 					if(temp_distance<min_distance){
// 						min_distance = temp_distance;
// 						closest_cluster = b;
// 					}
// 				}

// 				a.second->distance_from_centroid = min_distance;
// 				a.second->cluster = closest_cluster;
// 				a.second->second_closest_centroid = closest_cluster;
// 				this->clusters->at(closest_cluster)->items_in_cluster.push_back(a.first);
// 			}
// 			a.second->assigned_to_cluster = false;
// 		}
// 		for (long unsigned int i = 0; i < clusters->size();i++) {
// 			delete cluster_checks[i];
// 		}
// 		cluster_checks.clear();
// 		this->centroids_changed = 0;
// 		this->update_centroids();
// 	}while((long unsigned int) this->centroids_changed > this->clusters->size() / 2);
// }

// void Kmeans::reverse_assignment_cube(Hypercube * cube_object){
// 	float min_distance_between_centroids;
// 	min_distance_between_centroids = clusters->at(0)->centroid->item_distance(2,clusters->at(1)->centroid);
// 	float temp_comparison;
// 	for(long unsigned int a=0;a<clusters->size()-1;a++){
// 		for(long unsigned int b=a+1;b<clusters->size();b++){
// 			temp_comparison = this->clusters->at(a)->centroid->item_distance(2,clusters->at(b)->centroid);
// 			if(temp_comparison < min_distance_between_centroids)
// 				min_distance_between_centroids = temp_comparison;
// 		}
// 	}
// 	//we get min_distance_between_centroids between any 2 centroids


// 	int changed_points=0;
// 	int total_points = this->all_points->size();
// 	float radius = min_distance_between_centroids/2;
// 	float temp_distance;
// 	std::set<item *> * temp;
// 	std::vector<std::set<item *> *> cluster_checks;
// 	for (long unsigned int i = 0; i < clusters->size();i++) {
// 		cluster_checks.push_back(cube_object->range_search_items(this->clusters->at(i)->centroid));
// 	}


// 	//we get min_distance_between_centroids between any 2 centroids



// 	total_points = this->all_points->size();
// 	do{
// 		do{
// 			changed_points = 0;

// 			for (long unsigned int i = 0; i < clusters->size();i++) {

// 				temp = cluster_checks[i];
// 				ItemInfo * tempItemInfo;
// 				for(auto const &a:*temp){
// 					if(this->all_points->find(a) != this->all_points->end()){
// 						temp_distance = a->item_distance(2,this->clusters->at(i)->centroid);
// 						if(temp_distance < radius){
// 							tempItemInfo = this->all_points->at(a);
// 							if(tempItemInfo->assigned_to_cluster){

// 								if(tempItemInfo->distance_from_centroid > temp_distance){
// 									this->clusters->at(tempItemInfo->cluster)->items_in_cluster.remove(a);
// 									this->clusters->at(i)->items_in_cluster.push_back(a);
// 									tempItemInfo->distance_from_centroid = temp_distance;
// 									tempItemInfo->second_closest_centroid =tempItemInfo->cluster;
// 									tempItemInfo->cluster = i;
// 									changed_points++;
// 								}
// 							}
// 							else{
// 								this->clusters->at(i)->items_in_cluster.push_back(a);
// 								tempItemInfo->distance_from_centroid = temp_distance;
// 								tempItemInfo->cluster = i;
// 								tempItemInfo->second_closest_centroid =tempItemInfo->cluster;
// 								tempItemInfo->assigned_to_cluster = true;
// 								changed_points++;
// 							}
// 						}
// 					}
// 					else{
// 						cout<< "COULD NOT FIND POINT" << std::endl;
// 					}
// 					temp->erase(a);
// 				}
// 				cout<<this->clusters->at(i)->items_in_cluster.size()<< "fjjfjfj" <<endl;
// 			}

// 			radius *= 2;
// 		}while(changed_points > total_points/2);
// 		float min_distance;
// 		int closest_cluster;
// 		for(auto const &a:*(this->all_points)){
// 			if(!(a.second->assigned_to_cluster)){
// 				min_distance = clusters->at(0)->centroid->item_distance(2,a.first);
// 				closest_cluster = 0;
// 				for(long unsigned int b = 1 ; b < clusters->size(); b++){
// 					temp_distance = clusters->at(b)->centroid->item_distance(2,a.first);
// 					if(temp_distance<min_distance){
// 						min_distance = temp_distance;
// 						closest_cluster = b;
// 					}
// 				}

// 				a.second->distance_from_centroid = min_distance;
// 				a.second->cluster = closest_cluster;
// 				a.second->second_closest_centroid = closest_cluster;
// 				this->clusters->at(closest_cluster)->items_in_cluster.push_back(a.first);
// 			}
// 			a.second->assigned_to_cluster = false;
// 		}
// 		for (long unsigned int i = 0; i < clusters->size();i++) {
// 			delete cluster_checks[i];
// 		}
// 		cluster_checks.clear();
// 		this->centroids_changed = 0;
// 		this->update_centroids();
// 	}while((long unsigned int) this->centroids_changed > this->clusters->size() / 2);
// }

void Kmeans::assign_points_to_clusters(){
	for (auto const &i : *(this->all_points)){


		float distance_from_centroid;
		int cluster_index = 0;
		bool changed_cluster = false;
		for (auto const &j : *(this->clusters)){


			distance_from_centroid = i.first->item_distance(2, j->centroid);

			if (i.second->assigned_to_cluster == false){

				/* fill in point's info */
				i.second->cluster = cluster_index;
				i.second->second_closest_centroid = cluster_index;
				i.second->distance_from_centroid = distance_from_centroid;
				i.second->assigned_to_cluster = true;

				changed_cluster = true;
			}
			else if (distance_from_centroid < i.second->distance_from_centroid){

				/* check if new cluster is the same as the current cluster */
				if (i.second->cluster != cluster_index){

					/* remove from current cluster */
					this->clusters->at(i.second->cluster)->items_in_cluster.remove(i.first);

					/* update point's info */
					i.second->second_closest_centroid = i.second->cluster;
					i.second->cluster = cluster_index;
					i.second->distance_from_centroid = distance_from_centroid;
					i.second->assigned_to_cluster = true;


					changed_cluster = true;
				}
			}

			cluster_index++;
		}

		/* update global cluster status */
		if (changed_cluster == true){
			(this->points_reassigned)++;
			this->clusters->at(i.second->cluster)->items_in_cluster.push_back(i.first);
		}
	}
}



void Kmeans::update_centroids(){

	for(auto const &c : *(this->clusters)){
		item *new_centroid = this->calculate_median(c);

		/* check if centroid changed significantly */
		if(c->centroid->item_distance(2, new_centroid) > 10.0)
			(this->centroids_changed)++;


		delete c->centroid;
		c->centroid = new_centroid;
	}
}



item *Kmeans::calculate_median(Cluster *cluster_to_edit){
	vector<float> *new_vector = new vector<float>();
	int point_dimensions = (cluster_to_edit->items_in_cluster.front())->get_length();
	float current_sum;
	int num_cluster_points = cluster_to_edit->items_in_cluster.size();

	for(int i = 0; i < point_dimensions; i++){
		current_sum = 0.0;

		for(auto const &point : cluster_to_edit->items_in_cluster)
			current_sum += point->get_certain_dimenstion(i);

		new_vector->push_back(current_sum / num_cluster_points);
	}

	return new item(point_dimensions, new_vector, "");
}
















///////////////////////////////////////////////////////			EVALUATION - PROFILING			///////////////////////////////////////////////////////


void Kmeans::print_clusters(string stream, double init_time, double clustering_time){

	ofstream *outfile = NULL;
	if(stream == "cout"){
		cout << "Algorithm: " << this->method << endl;

	} else {
		outfile = new ofstream(stream, ios_base::app);
		*outfile << "Algorithm: " << this->method << endl;
	}

	for(long unsigned int i = 0; i < this->clusters->size(); i++){
		if(outfile == NULL){
			cout << "CLUSTER-" << i+1 << " {";
			cout << "size : " << this->clusters->at(i)->items_in_cluster.size() << ", ";
			cout << "centroid: " << this->clusters->at(i)->centroid->printitem(true) << endl << endl;

		} else {
			*outfile << "CLUSTER-" << i+1 << " {";
			*outfile << "size : " << this->clusters->at(i)->items_in_cluster.size() << ", ";
			*outfile << "centroid: " << this->clusters->at(i)->centroid->printitem(true) << endl << endl;
		}
	}

	if(outfile == NULL){


		cout << "Clustering_time " << clustering_time << " sec" << endl;

		cout << "Silhouette: [";
		vector<double> *sil = this->silhouette();
		for(long unsigned int i = 0; i < sil->size(); i++){
			cout << sil->at(i);

			if(i < sil->size() - 1)
				cout << ", ";
		}
		cout << "]" << endl;


		delete sil;

	} else {
		*outfile << "Clustering_time " << clustering_time << " sec" << endl;

		*outfile << "Silhouette: [";
		vector<double> *sil = this->silhouette();
		for (long unsigned int i = 0; i < sil->size(); i++)
		{
			*outfile << sil->at(i);

			if (i < sil->size() - 1)
				*outfile << ", ";
		}
		*outfile << "]" << endl;

		delete sil;
	}


	 if(this->complete_print_mode){
		if(outfile == NULL){

			cout << endl << endl;
			cout << "Initialization_time " << init_time << " sec" << endl;

			for(long unsigned int i = 0; i < this->clusters->size(); i++){
				cout << "CLUSTER-" << i << " {";
				cout << "centroid , ";

				for(auto const &point: this->clusters->at(i)->items_in_cluster){
					cout << point->get_id();

					if (point != this->clusters->at(i)->items_in_cluster.back())
						cout << ", ";
				}

				cout << "}" << endl << endl;
			}
		} else {
			*outfile << endl << endl;
			*outfile << "Initialization_time " << init_time << " sec" << endl;

			for(long unsigned int i = 0; i < this->clusters->size(); i++){
				*outfile << "CLUSTER-" << i+1 << " {";
				*outfile << "centroid , ";

				for(auto const &point: this->clusters->at(i)->items_in_cluster){
					*outfile << point->get_id();

					if (point != this->clusters->at(i)->items_in_cluster.back())
						*outfile << ", ";
				}

				*outfile << "}" << endl << endl;
			}
		}
	}

	if(outfile != NULL)
		delete outfile;
}


vector<double> *Kmeans::silhouette(){
	vector<double> *overall_results = new vector<double>();
	double mean_silhouette = 0.0;

	for(long unsigned int i = 0; i < this->clusters->size(); i++){
		Cluster *current_cluster = this->clusters->at(i);
		double s_cluster = 0.0;

		/* calculate s(i) */
		for(auto const &current_point : current_cluster->items_in_cluster){

			double a_i = 0.0, b_i = 0.0;

			/* calculate a(i) */
			for(auto const &other_point : current_cluster->items_in_cluster){
				if(other_point->get_id() != current_point->get_id())
					a_i += current_point->item_distance(2, other_point);

			}
			a_i = a_i / (current_cluster->items_in_cluster.size() - 1);

			// cout << "a(i) done" << endl;

			int other_cluster_index = this->all_points->at(current_point)->second_closest_centroid;
			Cluster *other_cluster = this->clusters->at(other_cluster_index);

			/* calculate b(i) */
			for(auto const &other_point : other_cluster->items_in_cluster){
				b_i += current_point->item_distance(2, other_point);
			}
			b_i = b_i / other_cluster->items_in_cluster.size();


			/* add s(i) to mean(s(i)) for this cluster */
			if(a_i < b_i)
				s_cluster += 1 - (a_i/b_i);
			else if(a_i > b_i)
				s_cluster += (b_i/a_i) - 1;
		}


		s_cluster = s_cluster / current_cluster->items_in_cluster.size();
		mean_silhouette += s_cluster;
		overall_results->push_back(s_cluster);
	}

	/* overall silhouette of clusters */
	mean_silhouette = mean_silhouette / this->clusters->size();
	overall_results->push_back(mean_silhouette);

	return overall_results;
}
