#include <iostream>
#include <map>
#include <string>
#include <unistd.h>

#include "kmeans.hpp"
#include "utilities.hpp"

using namespace std;


int main(int argc, char *argv[]){

	clock_t start = clock();
	char pid_string[8];
	sprintf(&(pid_string[0]), "%d", getpid());


	/*
	 	expects K-medians, input file, output file, complete flag, method,
		LSH parameters: L, k,
		Hypercube parameters: m, probes
	*/
	map<string, string> *parameters = get_program_parameters(argc, argv, "clustering");
	string input_path;
	string output_path;
	string method;
	int num_clusters;
	bool complete_print = false;

	if(parameters->find("input_file") != parameters->end())
		input_path = parameters->at("input_file");

	if(parameters->find("output_file") != parameters->end()){
		output_path = parameters->at("output_file");
	}

	if(parameters->find("K_medians") != parameters->end())
		num_clusters = stoi(parameters->at("K_medians"));

	if(parameters->find("complete") != parameters->end() && parameters->at("complete") == "yes")
		complete_print = true;

	if(parameters->find("method") != parameters->end())
		method = parameters->at("method");

	int lsh_L = 3, lsh_k = 4;
	int cube_m = 10, cube_k = 3, cube_probes = 2;
	if(method == "LSH" || method == "lsh"){

		// get parameters for LSH
		if (parameters->find("L") != parameters->end())
			lsh_L = stoi(parameters->at("L"));

		if (parameters->find("L") != parameters->end())
			lsh_k = stoi(parameters->at("k_hash_functions"));

	} else if(method == "Hypercube" || method == "hypercube"){

		// get parameters for hypercube
		if (parameters->find("max_m_cube") != parameters->end())
			cube_m = stoi(parameters->at("max_m_cube"));

		if (parameters->find("probes") != parameters->end())
			cube_probes = stoi(parameters->at("probes"));

		if (parameters->find("k_cube_size") != parameters->end())
			cube_k = stoi(parameters->at("k_cube_size"));
	}

	delete parameters;

	cout << "Process #" << getpid() << " running with following parameters:" << endl;
	cout << "------------------------------------------------" << endl << endl;
	cout << "Parameters have the following final values: " << endl;
	cout << "input file path: " << input_path << endl;
	cout << "output file path: " << output_path << endl;
	cout << "number of clusters: " << num_clusters << endl;
	cout << "method: " << method << endl;
	cout << "print format: ";
	if(complete_print == false)
		cout << "incomplete" << endl;
	else
		cout << "complete"<< endl;
	cout << "lsh parameters: L = " << lsh_L << ", k = " << lsh_k << endl;
	cout << "hypercube parameters: max_m: " << cube_m << ", probes: " << cube_probes << ", size k: " << cube_k << endl;

	Kmeans *kmeans_object = new Kmeans(num_clusters, method, lsh_L, lsh_k, cube_m, cube_probes, cube_k, complete_print);

	parse_file(input_path, "", kmeans_object, "clustering");
	cout << "parsed input" << endl;

	clock_t init_start = clock();
	kmeans_object->initialize_centroids();
	clock_t init_end = clock();

	cout << "initialization done" << endl;

	clock_t cluster_start = clock();
	kmeans_object->create_clusters(method);
	clock_t cluster_end = clock();

	cout << "clustering done" << endl;

	kmeans_object->print_clusters(output_path, (double)(init_end - init_start) / CLOCKS_PER_SEC, (double)(cluster_end - cluster_start) / CLOCKS_PER_SEC);

	// clock_t update_start = clock();
	// kmeans_object->update_centroids();
	// clock_t update_end = clock();

	// cout << "reassigning points" << endl;
	// kmeans_object->assign_points_to_clusters();
	// cout << "update done" << endl;


	// cout << "kmeans num of clusters: " << kmeans_object->clusters->size() << endl;
	// cout << "kmeans num of points: " << kmeans_object->all_points->size() << endl;
	// cout << "kmeans cluster 1 size: " << kmeans_object->clusters->at(0)->items_in_cluster.size() << endl;
	// cout << "kmeans cluster 2 size: " << kmeans_object->clusters->at(1)->items_in_cluster.size() << endl;
	// cout << "kmeans cluster 3 size: " << kmeans_object->clusters->at(2)->items_in_cluster.size() << endl;
	// kmeans_object->print_clusters(output_path, (double)(init_end - init_start) / CLOCKS_PER_SEC, (double)(assign_end - assign_start) / CLOCKS_PER_SEC);

	delete kmeans_object;
	clock_t end = clock();
	double execution_time = (double)(end - start) / CLOCKS_PER_SEC;
	cout << "Program completed. Execution time " << execution_time << "sec\n";
}
