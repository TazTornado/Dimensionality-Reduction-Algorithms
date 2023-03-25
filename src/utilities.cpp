#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <random>
#include <chrono>
#include "utilities.hpp"
#include "error.hpp"
#include "item.hpp"
#include "lsh.hpp"
#include "hypercube.hpp"
#include "kmeans.hpp"

using namespace std;

double distance_max = 0.0, distance_mean = 0.0;
string max_dist_query = "";

double time_max = 0.0, time_mean = 0.0;
string max_time_query = "";

int bad_dist_counter = 0, bad_time_counter = 0;

void error_handler(int error_code, const string &message){

	switch (error_code){


		case SYSTEM_ERROR:
			perror("");
			break;

		case UTIL_EUC_MOD:
			cerr << "Error in Euclidean Modulo method: divident is equal to zero." << endl;
			break;

		case LSH_SET_BAD_NAME:
			cerr << "Error in Set method: unrecognised member name." << endl;
			break;

		case CLUSTER_EMPTY_SET:
			cerr << "Cluster error: set of points is empty." << endl;
			break;

		case PROG_PARAMS:
		case CLUSTER_CONFIG_PARAMS:
			cerr << "Error in program parameters." << endl;
			break;

		default:
			cerr << "Something went wrong. Unrecognised error code." << endl;
	}

	if(message != "") cerr << message << endl;
}


int euclidean_modulo(int num1, int num2)
{
	if (num2 == 0)
	{
		error_handler(UTIL_EUC_MOD, "");
		return -1;
	}
	int result = num1 % num2;
	return result >= 0 ? result : result + std::abs(num2);
}



float gaussian_random_generator()
{
	/* generate random (real) vi ~ N(0, 1) */

	unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::normal_distribution<float> distr(0.0, 1.0);

	return distr(generator);
}


float uniform_random_generator(int upper_bound){
	/* generate random (real) n ~ U(0, window) */

	unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<float> distr(0.0, upper_bound);

	return distr(generator);
}

float uniform_random_generator(float upper_bound){
	/* generate random (real) n ~ U(0, window) */

	unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<float> distr(0.0, upper_bound);

	return distr(generator);
}


void read_configuration(map<string, string> *parameters, string file_to_read){

	bool Kflag = false, Lflag = false, k_hash_flag = false, mflag = false, k_cube_flag = false, probes_flag = false;

	string line;
	ifstream file;
	file.open(file_to_read, ifstream::in);

	if (file.is_open()){
		string token;

		while(getline(file, line)){

			stringstream line_to_split;
			line_to_split << line;

			getline(line_to_split, token, ' ');

			if(token == "number_of_clusters:"){
				if(Kflag == true){
					error_handler(CLUSTER_CONFIG_PARAMS, "Configuration: number_of_clusters, was given twice.");
					break;
				}
				getline(line_to_split, token, ' ');
				parameters->emplace("K_medians", token);

				Kflag = true;

			} else if(token == "number_of_vector_hash_tables:"){
				if(Lflag == true){
					error_handler(CLUSTER_CONFIG_PARAMS, "Configuration: number_of_vector_hash_tables, was given twice.");
					break;
				}
				getline(line_to_split, token, ' ');
				parameters->emplace("L", token);

			} else if(token == "number_of_vector_hash_functions:"){
				if(k_hash_flag == true){
					error_handler(CLUSTER_CONFIG_PARAMS, "Configuration: number_of_vector_hash_functions, was given twice.");
					break;
				}
				getline(line_to_split, token, ' ');
				parameters->emplace("k_hash_functions", token);

				k_hash_flag = true;

			} else if (token == "max_number_M_hypercube:"){
				if(mflag == true){
					error_handler(CLUSTER_CONFIG_PARAMS, "Configuration: max_number_M_hypercube, was given twice.");
					break;
				}
				getline(line_to_split, token, ' ');
				parameters->emplace("max_m_cube", token);

				mflag = true;

			} else if (token == "number_of_hypercube_dimensions:"){
				if(k_cube_flag == true){
					error_handler(CLUSTER_CONFIG_PARAMS, "Configuration: number_of_hypercube_dimensions, was given twice.");
					break;
				}
				getline(line_to_split, token, ' ');
				parameters->emplace("k_cube_size", token);

				k_cube_flag = true;

			} else if(token == "number_of_probes:"){
				if(probes_flag == true){
					error_handler(CLUSTER_CONFIG_PARAMS, "Configuration: number_of_probes, was given twice.");
					break;
				}
				getline(line_to_split, token, ' ');
				parameters->emplace("probes", token);

				probes_flag = true;

			} else {
				error_handler(CLUSTER_CONFIG_PARAMS, "Configuration: unrecognized parameter " + token);
			}
		}

		if(Kflag == false)
			error_handler(CLUSTER_CONFIG_PARAMS, "Configuration: number_of_clusters was _not_ given but is mandatory.");

		file.close();
	}
}



map<string, string> *get_program_parameters(int argc, char *argv[], string behavior){

	map<string, string> *parameters = new map<string, string>();

	/* common flags */
	bool iflag = false, qflag = false, oflag = false, kflag = false, Nflag = false, Rflag = false,Wflag = false;

	/* lsh-specific flags */
	bool Lflag = false;

	/* hypercube-specific flags */
	bool mflag = false, pflag = false;

	/* clustering-specific flags */
	bool config_flag = false, complete_flag = false, method_flag = false;

	while (argc--){


		if(strcmp(*argv, "-i") == 0){
			if(iflag == true){
				error_handler(PROG_PARAMS, "Input file path given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("input_file", string(*(argv + 1)));
			iflag = true;

		} else if ((behavior != "clustering") && (strcmp(*argv, "-q") == 0)){

			if (qflag == true){
				error_handler(PROG_PARAMS, "Query file path given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("query_file", string(*(argv + 1)));

			qflag = true;

		} else if(strcmp(*argv,"-o") == 0){


			if (oflag == true) {
				error_handler(PROG_PARAMS, "Output file path given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("output_file", string(*(argv + 1)));

			oflag = true;

		} else if (strcmp(*argv, "-k") == 0){
			if (kflag == true){
				error_handler(PROG_PARAMS, "Parameter k given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("k", string(*(argv + 1)));

			kflag = true;

		} else if(strcmp(*argv,"-N")== 0){

			if (Nflag == true){
				error_handler(PROG_PARAMS, "Parameter number of nearest given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("nearest", string(*(argv + 1)));

			Nflag = true;

		} else if(strcmp(*argv,"-R")==0){

			if (Rflag == true){
				error_handler(PROG_PARAMS, "Parameter radius given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("radius", string(*(argv + 1)));

			Rflag = true;

		}else if(strcmp(*argv, "-w") == 0){
			if(Wflag == true){
				error_handler(PROG_PARAMS, "Parameter w given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("w", string(*(argv + 1)));

			Wflag = true;
		}


		 else if ((behavior == "lsh") && (strcmp(*argv, "-L") == 0)){

			if (Lflag == true){
				error_handler(PROG_PARAMS, "Parameter L given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("L", string(*(argv + 1)));

			Lflag = true;

		} else if((behavior == "hypercube") && (strcmp(*argv, "-M") == 0)){

			if(mflag == true){
				error_handler(PROG_PARAMS, "Parameter M given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("m", string(*(argv + 1)));

			mflag = true;

		} else if((behavior == "hypercube") && (strcmp(*argv, "-probes") == 0)){

			if(pflag == true){
				error_handler(PROG_PARAMS, "Parameter probes given twice.");
				delete parameters;
				return NULL;
			}

			parameters->emplace("probes", string(*(argv + 1)));

			pflag = true;

		} else if(behavior == "clustering"){
			if(strcmp(*argv, "-c") == 0){

				if(config_flag == true){
					error_handler(PROG_PARAMS, "Configuration file path given twice.");
					delete parameters;
					return NULL;
				}

				parameters->emplace("config_file", string(*(argv + 1)));

				config_flag = true;

			} else if(strcmp(*argv, "-m") == 0){

				if(method_flag == true){
					error_handler(PROG_PARAMS, "Parameter method given twice.");
					delete parameters;
					return NULL;
				}

				if (strcmp(*(argv + 1), "Classic") != 0 && strcmp(*(argv + 1), "classic") != 0 &&
					strcmp(*(argv + 1), "LSH") != 0 && strcmp(*(argv + 1), "lsh") != 0 &&
					strcmp(*(argv + 1), "Hypercube") != 0 && strcmp(*(argv + 1), "hypercube") != 0){

					error_handler(PROG_PARAMS, "Parameter method not recognised.\n(options: Classic, LSH, Hypercube");
					delete parameters;
					return NULL;
				}

				parameters->emplace("method", string(*(argv + 1)));

				method_flag = true;

			} else if (strcmp(*argv, "-complete") == 0){

				if(complete_flag == true){
					error_handler(PROG_PARAMS, "Optional parameter complete given twice.");
					delete parameters;
					return NULL;
				}

				parameters->emplace("complete", "yes");

				complete_flag = true;
			}
		}

		argv++;
	}

	if(iflag == false){
		string input_path;
		cout << "Please give input file path." << endl;
		cin >> input_path;
		parameters->emplace("input_file", input_path);
	}

	if (behavior != "clustering" && qflag == false){
		string query_path;
		cout << "Please give query file path." << endl;
		cin >> query_path;
		parameters->emplace("query_file", query_path);
	}

	if (oflag == false){
		string output_path;
		cout << "Please give output file path." << endl;
		cin >> output_path;
		parameters->emplace("output_file", output_path);
	}

	if(behavior == "clustering"){
		if(config_flag == false){
			string config_path;
			cout << "Please give path to configuration file with clustering parameters." << endl;
			cin >> config_path;
			parameters->emplace("config_file", config_path);
		}

		if(complete_flag == false)
			parameters->emplace("complete", "no");

		read_configuration(parameters, parameters->at("config_file"));
	}

	return parameters;
}



int count_lines_in_file(const std::string &filepath){

	std::string command;
	command = "wc -l " + filepath;

	int lines;
	FILE *f = popen(command.c_str(), "r");
	fscanf(f, "%d", &lines);
	pclose(f);

	return lines;
}



void parse_file(string file_to_read, string file_to_write, void *problem, string problem_type){

	string line;
	ifstream file;
	file.open(file_to_read, ifstream::in);

	if (file.is_open()){
		string token;
		int item_counter = 0;

		while(getline(file, line)){
			stringstream line_to_split;
			line_to_split << line;

			getline(line_to_split, token, ' ');
			string id = token;
			vector<float> *p = new vector<float>();
			int token_count = 0;
			while(getline(line_to_split, token, ' ')){
				if (token == "\n" || token == "\r" || token == "\r\n")
					break;

				p->push_back(stof(token.c_str()));
				token_count++;
			}
			item *p_item = new item(token_count, p, id);

			if(problem_type == "lsh"){
				LSH *lsh_object = (LSH *) problem;
				if(file_to_write == ""){
					lsh_object->preprocess_item(p_item);

				} else {
					lsh_object->execute_query(2,p_item,file_to_write);
					delete p_item;
				}

			} else if(problem_type == "hypercube"){

				Hypercube *cube_object = (Hypercube *) problem;
				if(file_to_write == ""){
					cube_object->preprocess_item(p_item);

				} else {
					cube_object->execute_query(p_item,file_to_write);
					delete p_item;
				}
			} else {
				Kmeans *cluster_object = (Kmeans *) problem;
				if(file_to_write == ""){
					cluster_object->insert_item(p_item);

				}

			}

			item_counter++;
		}



		if(file_to_write != ""){

			if(problem_type == "lsh"){
				LSH *lsh_object = (LSH *) problem;
				distance_mean = distance_mean / (item_counter * lsh_object->N);

			} else if (problem_type == "hypercube") {
				Hypercube *cube_object = (Hypercube *) problem;
				distance_mean = distance_mean / (item_counter * cube_object->N);
			}

			time_mean = time_mean / item_counter;


			std::ofstream outfile;
			outfile.open(file_to_write, std::ios_base::app);
			outfile << endl << endl;
			outfile << "==========================================================================================================" << endl;
			outfile << "Evaluation of Algorithm:" << problem_type << endl << "------------------" << endl;

			outfile << "max distanceApprox / distanceTrue : " << distance_max << " for query with ID " << max_dist_query << endl;
			outfile << "mean distanceApprox / distanceTrue : " << distance_mean << endl;
			outfile << "bad distance frequency: " << bad_dist_counter << " queries" << endl << endl;

			outfile << "max timeApprox / timeTrue : " << time_max << " for query with ID " << max_time_query << endl;
			outfile << "mean timeApprox / timeTrue : " << time_mean << endl;
			outfile << "bad time frequency: " << bad_time_counter << " queries" << endl;

			outfile.close();
		}

		file.close();
	}


}
