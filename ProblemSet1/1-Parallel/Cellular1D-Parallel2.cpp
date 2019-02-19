#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

int main(int argc, char** argv)
{
	double start_time = MPI_Wtime();
	int comm_sz, my_rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	std::string rules = argv[1];
	std::string config = argv[2];
	int iterations = atoi(argv[3]);
	char left, right;
	int inputSize;
	std::map<std::string, char> rulesLookup;
	char* automata;

	if (my_rank == 0)
	{
		std::ifstream ruleFile, configFile;

		ruleFile.open(rules);
		std::string key;
		char value;
		while (ruleFile >> key >> value)
		{
			rulesLookup[key] = value;
		}
		ruleFile.close();
		configFile.open(config);
		int counter;
		configFile >> inputSize;
		char c;
		automata = new char[inputSize];
		counter = 0;

		while (configFile >> c) 
		{
			automata[counter++] = c;
		}
		configFile.close();


		MPI_Request req1;
		for (int i = 1; i < comm_sz; ++i)
		{
			MPI_Send(&inputSize, 1, MPI_FLOAT, i, 6, MPI_COMM_WORLD);
		}

		// SENDING THE RULES TO THE DIFFErENT PROCESSES;
		int sz = rulesLookup.size();

		for (int j = 1; j < comm_sz; ++j)
		{
			MPI_Send(&sz, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
			for (std::map<std::string, char>::value_type& x : rulesLookup)
			{
				MPI_Send(x.first.c_str(), x.first.length(), MPI_BYTE, j, 1, MPI_COMM_WORLD);
				MPI_Send(&x.second, 1, MPI_BYTE, j, 1, MPI_COMM_WORLD);
			}	
		}
		///////////////////////////////////////////////////
	}
	else
	{
		MPI_Recv(&inputSize, 1, MPI_FLOAT, 0, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//RECIEVING RULES;
		int recvs;
		MPI_Recv(&recvs, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		char c;

		for (int i = 0; i < recvs; ++i)
		{
			MPI::Status status;
			MPI::COMM_WORLD.Probe(0, 1, status);
			int l = status.Get_count(MPI_BYTE);
			char *buf = new char[l];
			MPI::COMM_WORLD.Recv(buf, l, MPI_BYTE, 0, 1, status);
			std::string temp(buf, l);
			MPI_Recv(&c, 1, MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			rulesLookup[temp] = c;
		}
		//////////////////////////////////////////////////////////////////////
	}

	int copyInputSize = inputSize;
	int floorOfSizePrProcess = inputSize / comm_sz;
	int *sendcounts = new int[comm_sz];
	int *displacement = new int[comm_sz];

	for (int i = 0; i < comm_sz - 1; ++i)
	{
		sendcounts[i] = floorOfSizePrProcess;
		displacement[i] = i * floorOfSizePrProcess;
		copyInputSize -= floorOfSizePrProcess;
	}

	sendcounts[comm_sz - 1] = copyInputSize;
	displacement[comm_sz - 1] = copyInputSize * (comm_sz - 1);

	char *subConfig = new char[sendcounts[my_rank]];
	char *copySubConfig = new char[sendcounts[my_rank]];
	
	/*
	if (my_rank == 0)
	{
		for (int j = 0; j < inputSize; ++j)
		{
			std::cout << automata[j];			
		}
		std::cout << std::endl;
	}
	*/
	MPI_Scatterv((my_rank == 0) ? automata : NULL, sendcounts, displacement, MPI_CHAR, subConfig, sendcounts[my_rank], MPI_CHAR, 0, MPI_COMM_WORLD);
	for (int i = 0; i < iterations; ++i)
	{
		MPI_Request req;
		MPI_Isend(&subConfig[0], 1, MPI_CHAR, (my_rank + comm_sz - 1) % comm_sz, 5, MPI_COMM_WORLD, &req);
		MPI_Isend(&subConfig[sendcounts[my_rank] - 1], 1, MPI_CHAR, (my_rank + 1) % comm_sz, 4, MPI_COMM_WORLD, &req);
		MPI_Recv(&right, 1, MPI_CHAR, (my_rank + 1) % comm_sz, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&left, 1, MPI_CHAR, (my_rank + comm_sz - 1) % comm_sz, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Request_free(&req);


		for (int j = 0; j < sendcounts[my_rank]; ++j)
		{
			std::string s;
			s += (j==0) ? left : subConfig[j-1];
			s += subConfig[j];
			s += (j==(sendcounts[my_rank] - 1)) ? right : subConfig[j+1];
			copySubConfig[j] = rulesLookup[s];
		}

		//MPI_Gatherv(copySubConfig, sendcounts[my_rank], MPI_CHAR, automata, sendcounts, displacement, MPI_CHAR, 0, MPI_COMM_WORLD);
/*
		if (my_rank == 0)
		{
			for (int j = 0; j < inputSize; ++j)
			{
				std::cout << automata[j];			
			}
			std::cout << std::endl;
		}
		*/	
	}	
	if (my_rank == 0) std::cout << MPI_Wtime() - start_time << std::endl;
	MPI_Finalize();
	return 0;
}
// https://opensource.org/licenses/MIT in