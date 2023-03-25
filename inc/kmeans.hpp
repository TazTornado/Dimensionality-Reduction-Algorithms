#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <list>

#include "item.hpp"
#include "utilities.hpp"


class Cluster{
	public:
		item *centroid;
		// bool changed_centroid;
		std::list<item *> items_in_cluster;

		Cluster(item *centroid){ this->centroid = centroid;  };
		~Cluster() { delete this->centroid;  };

};

class ItemInfo{
	public:
		int cluster;		// index of the cluster the item belongs to
		double distance_from_centroid;	// distance from closest centroid
		int second_closest_centroid;
		bool assigned_to_cluster;		// flag to indicate whether the item is assigned to a cluster

		ItemInfo(){ cluster = -1; distance_from_centroid = 0.0; assigned_to_cluster = false; };
};

class Kmeans{

	private:
		unsigned int k;
		int points_reassigned, centroids_changed;
		std::vector<Cluster *> *clusters;
		std::map<item *, ItemInfo*> *all_points;		// all points and their distance from nearest centroid
		std::string method;

		/* LSH and Hypercube parameters */
		int L_hashtables, k_hash_functions;
		int max_points, probes, cube_dimensions;

		bool complete_print_mode;

	public:

		Kmeans(unsigned int k_medians, std::string method, int L_hashtables = 3, int k_hash_functions = 4, int max_points = 10, int probes = 2, int cube_dimensions = 3, bool complete = false);
		~Kmeans();

		/* Kmeans++ initialization */
		void initialize_centroids();
		item *choose_centroid(std::vector<std::pair<item *, float>> *min_distances);
		void calculate_distance(std::string id, std::vector<std::pair<item *, float>> *distances_to_calculate); // if "" then calculate for all, else only for the id given

		/* Kmeans clustering */
		void insert_item(item *p);
		void create_clusters(std::string method);			// decides whether to use lloyd's algorithm or LSH/Hypercube range search

		void lloyd_clustering();
		void reverse_assignment_lsh(LSH * lsh_object);
		void reverse_assignment_cube(Hypercube * cube_object);
		void assign_points_to_clusters();
		void update_centroids();
		item *calculate_median(Cluster *cluster_to_edit);					// for every iteration, return median point's ID

		void reverse_assignment_lsh();	// algorithm = "lsh" | "hypercube"
		void reverse_assignment_hypercube();

		/* Evaluation and profiling */
		void print_clusters(std::string stream, double init_time, double clustering_time);
		std::vector<double> *silhouette();

};
