#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <stdlib.h>
#include "utilities.hpp"
#include "hypercube.hpp"

using namespace std;


int main(int argc, char *argv[]){

	clock_t start = clock();

	map<string, string> *parameters = get_program_parameters(argc, argv, "hypercube");
	if(parameters == NULL){
		cout << "Main was unable to acquire parameters. Exiting program..." << endl;
		return 1;
	}

	string input_path;
	if (parameters->find("input_file") != parameters->end()){
		input_path = parameters->at("input_file");

	} else {

		cout << "Couldn't get input path. Exiting..." << endl;
		delete parameters;
		return 1;
	}


	string output_path;

	if (parameters->find("output_file") != parameters->end()){
		output_path = parameters->at("output_file");

	} else {

		cout << "Couldn't get output path. Exiting..." << endl;
		delete parameters;
		return 1;
	}


	string query_path;
	if (parameters->find("query_file") != parameters->end()){
		query_path = parameters->at("query_file");

	} else {

		cout << "Couldn't get query path. Exiting..." << endl;
		delete parameters;
		return 1;
	}

	// int hashtable_size = (int)(count_lines_in_file(input_path) / 8);
	int k = 14, N = 1, m = 10, probes = 2, R = 10000,w = 600;

	cout << "Hypercube running with following parameters:" << endl;
	cout << "------------------------------------------------" << endl << endl;
	cout << "Default parameters: k = " << k << ", N = " << N << ", m = " << m << "probes = " << probes << ", R = " << R << ", w " << w << endl;

	if (parameters->find("k") != parameters->end()) k = std::stoi((*parameters)["k"]);
	if (parameters->find("nearest") != parameters->end()) N = std::stoi((*parameters)["nearest"]);
	if (parameters->find("m") != parameters->end()) m = std::stoi((*parameters)["m"]);
	if (parameters->find("probes") != parameters->end()) probes = std::stoi((*parameters)["probes"]);
	if (parameters->find("radius") != parameters->end()) R = std::stof((*parameters)["radius"]);
	if (parameters->find("w") != parameters->end()) w = std::stof((*parameters)["w"]);

	cout << "Parameters have the following values: k = " << k << ", N = " << N << ", m = " << m << "probes = " << probes << ", R = " << R << ", w " << w << endl;



	delete parameters;
	Hypercube *cube = new Hypercube(k, N, m, probes, R,w);
	int execution_option = 4;
	bool run_again = false;


	do{
		cout << "input file path: " << input_path << endl;
		cout << "output file path: " << output_path << endl;
		cout << "query file path: " << query_path << endl;

		if(run_again == true)
			start = clock();

		/* parse input only if there is an input set to parse */
		if(execution_option != 2){
			parse_file(input_path, "", cube, "hypercube");
			cout << "parsed input file" << endl;
		}

		parse_file(query_path, output_path, cube, "hypercube");
		cout << "parsed query file" << endl;

		clock_t end = clock();
		double execution_time = (double)(end - start) / CLOCKS_PER_SEC;
		cout << "Program completed. Execution time " << execution_time << "sec\n";

		/* Give menu of options */
		cout << "Continue with one of the following options: " << endl;
		cout << "Option 1: Exit program" << endl;
		cout << "Option 2: Rerun program with new query file" << endl;
		cout << "Option 3: Rerun program with new input file" << endl;
		cout << "Option 4: Rerun program with new input file and new query file" << endl << endl;
		cout << "What would you like to do now? [Enter 1 or 2 or 3 or 4]" << endl << "Option: ";
		cin >> execution_option;
		cout << endl;

		switch(execution_option){
			default:
				cout << "Unrecognized option" << endl;

			case 1:
				cout << "Exiting program..." << endl;
				delete cube;
				return 0;

			case 2:
				cout << "Please give path of new query file: ";
				cin >> query_path;
				cout << endl;
				run_again = true;
				break;

			case 3:
				cout << "Please give path of new input file: ";
				cin >> input_path;
				cout << endl;

				delete cube;
				cube = new Hypercube(k, N, m, probes, R, w);
				run_again = true;
				break;

			case 4:
				cout << "Please give path of new input file: ";
				cin >> input_path;
				cout << endl;

				cout << "Please give path of new query file: ";
				cin >> query_path;
				cout << endl;

				delete cube;
				cube = new Hypercube(k, N, m, probes, R, w);
				run_again = true;
				break;
		}

	} while(run_again == true);


	delete cube;
	return 0;
}
