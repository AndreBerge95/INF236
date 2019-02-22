/*
Copyright 2019 André Berge

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include<string>
#include<fstream>
#include<iostream>
#include<vector>
#include<map>
#include<cmath>
#include<mpi.h>

int main(int argc, char** argv)
{
	double startTime = MPI_Wtime();
	int comm_sz, my_rank; // use 10 processes.

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	std::string rules = argv[1];
	std::string config = argv[2];
	int iterations = atoi(argv[3]);
	std::ifstream ruleFile, configFile;
	std::map<std::string, char> rulesLookup;
	std::vector<char> vec, copymata;
	std::string key;
	char value;
	char *configArray;
	int gridSize;

	if (my_rank == 0)
	{
		ruleFile.open(rules);
		while (ruleFile >> key >> value)
		{
			rulesLookup[key] = value;
		}

		configFile.open(config);
		configFile >> gridSize;
		configArray = new char[gridSize * gridSize];
		MPI_Request req1;
		for (int i = 0; i < comm_sz; ++i)
		{
			MPI_Send(&gridSize, 1, MPI_FLOAT, i, 1, MPI_COMM_WORLD);
		}
		
		char c;
		int counter = 0;
		while (configFile >> c)
		{
			configArray[counter++] = c; 
		}

		//SENDING RULES
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
		///////////////////////////////////////////////////////////////////////
	}
	else
	{
		MPI_Recv(&gridSize, 1, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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
	int copyGridSize = gridSize;
	int floorOfSizePrProcess = copyGridSize/comm_sz;
	bool perfect = (gridSize % comm_sz == 0) ? true : false;
	int *sendcounts = new int[comm_sz];
	int *displacement = new int[comm_sz];

	for (int i = 1; i < comm_sz; ++i) copyGridSize -= floorOfSizePrProcess;

	sendcounts[0] = copyGridSize*gridSize;
	displacement[0] = 0;

	for (int i = 1; i < comm_sz; ++i)
	{
		sendcounts[i] = floorOfSizePrProcess*gridSize;
		displacement[i] = displacement[i-1] + sendcounts[i-1];
	}
	

	// ITERATIONS ////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////
	char* subConfig = new char[sendcounts[my_rank]];
	MPI_Scatterv((my_rank == 0) ? configArray : NULL, sendcounts, displacement, MPI_CHAR, subConfig, sendcounts[my_rank], MPI_CHAR, 0, MPI_COMM_WORLD);
	for (int i = 0; i < iterations; ++i)
	{
		// MAKING SURE TO ONLY SEND THE UPPERMOST ROW TO THE PROCESS ABOVE, AND VICE VERSA FOR LOWER MOST ROW.
		//char* upperMostRow = new char[gridSize];
		//char* lowerMostRow = new char[gridSize];
		/*
		if (my_rank == 0)
		{
			for (int j = 0; j < gridSize; ++j)
			{
				upperMostRow[j] = subConfig[j];
				if (perfect) lowerMostRow[j] = subConfig[((floorOfSizePrProcess - 1) * gridSize) + j];
				else lowerMostRow[j] = subConfig[(floorOfSizePrProcess * gridSize) + j];
			}	
		}
		else
		{
			for (int j = 0; j < gridSize; ++j)
			{
				upperMostRow[j] = subConfig[j];
				lowerMostRow[j] = subConfig[((floorOfSizePrProcess - 1) * gridSize) + j];
			}
		}
		*/
		int lowest = sendcounts[my_rank] - gridSize;
		
		///////////////////////////////////////////////////////////////////////////////////////////////7
		MPI_Request req;
		MPI_Isend(&subConfig[0], gridSize, MPI_CHAR, (my_rank + 1) % comm_sz, 1, MPI_COMM_WORLD, &req);
		MPI_Isend(&subConfig[lowest], gridSize, MPI_CHAR, (my_rank + comm_sz - 1) % comm_sz, 1, MPI_COMM_WORLD, &req);
		char* arrayUp = new char[gridSize];
		char* arrayDown = new char[gridSize];
		MPI_Recv(arrayUp, gridSize, MPI_CHAR, (my_rank + comm_sz - 1) % comm_sz, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(arrayDown, gridSize, MPI_CHAR, (my_rank + 1) % comm_sz, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Request_free(&req);

		char* copySubConfig = new char[sendcounts[my_rank]];
		
		// BUILDING THE STRING OF NEIGHBOURS, INCLUDING CELL IN QUESTION, TO USE INF RULES-LOOKUP.
		for (int j = 0; j < sendcounts[my_rank]; ++j)
		{
			int left = ((j + gridSize - 1) % gridSize) + ((j/gridSize) * gridSize);
			int right = ((j + 1) % gridSize) + ((j/gridSize) * gridSize);
			std::string s = "";
			if (j - gridSize < 0)
			{
				s += arrayUp[(j + gridSize - 1) % gridSize];
				s += arrayUp[j % gridSize];
				s += arrayUp[(j + 1) % gridSize];
				s += subConfig[left];
				s += subConfig[j];
				s += subConfig[right];
			}
			else
			{
				s += subConfig[left - gridSize];
				s += subConfig[j - gridSize];
				s += subConfig[right - gridSize];
				s += subConfig[left];
				s += subConfig[j];
				s += subConfig[right];
			}
			if (j + gridSize >= sendcounts[my_rank])
			{
				s += arrayDown[(j + gridSize - 1) % gridSize];
				s += arrayDown[j % gridSize];
				s += arrayDown[(j + 1) % gridSize];
			}
			else
			{
				s += subConfig[left + gridSize];
				s += subConfig[j + gridSize];
				s += subConfig[right + gridSize]; 
			}
			copySubConfig[j] = rulesLookup[s];
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		for (int j = 0; j < sendcounts[my_rank]; ++j) subConfig[j] = copySubConfig[j];	
	}
	MPI_Gatherv(subConfig, sendcounts[my_rank], MPI_CHAR, (my_rank == 0) ? configArray : NULL, sendcounts, displacement, MPI_CHAR, 0, MPI_COMM_WORLD);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//PRINTING THE FINAL CONFIGURATION OF GAME OF LIFE.
	
	/*
	
	if (my_rank == 0)
	{
		for (int i = 0; i < gridSize; ++i)
		{
			for (int j = 0; j < gridSize; ++j)
			{
				std::cout << configArray[j + (gridSize * i)] << " ";
			}
			std::cout << std::endl;
		}
	}
	*/
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	double endTime = MPI_Wtime();
	if (my_rank == 0) std::cout << endTime - startTime << std::endl;
	MPI_Finalize();
	return 0;
}

//  https://opensource.org/licenses/MIT in
