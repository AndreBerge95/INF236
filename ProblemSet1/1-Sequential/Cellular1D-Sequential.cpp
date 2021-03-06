/*
Copyright 2019 André Berge

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

int main(int argc, char** argv)
{


	// Input file, file,  int: nr of iterations
	std::vector<std::string> automata;
	std::ifstream ruleFile, configFile;
	std::string rules = argv[1];
	std::string config = argv[2];
	int iterations = atoi(argv[3]);
	std::map<std::string, std::string> rulesLookup;

	ruleFile.open(rules);

	std::string key;
	std::string value;
	while (ruleFile >> key >> value)
	{
		rulesLookup[key] = value;
	}

	for (std::map<std::string, std::string>::value_type& x : rulesLookup)
	{
		std::cout << x.first << "," << x.second << std::endl;
	}

	ruleFile.close();
	configFile.open(config);
	int nr;
	configFile >> nr;
	char c;
	while (configFile >> c) 
	{
		automata.push_back(std::string(1,c));
	}

	std::vector<std::string> copymata(automata);

	configFile.close();

	std::cout << "Hello" << std::endl;
	std::ofstream formattingFile;
	std::vector<std::vector<std::string>> vectorTwoDim;

	for (int i = 0; i < iterations; ++i) 
	{
		for (int j = 0; j < automata.size(); ++j)
		{
			std::string s;
			if (j == 0) {
				s = automata.at(automata.size()-1) + automata.at(0) + automata.at(1);
				copymata.at(0) = rulesLookup[s]; 
			} else {
				s = automata.at(j-1) + automata.at(j) + automata.at((j+1) % automata.size());
				copymata.at(j) = rulesLookup[s];
			}
			
		}
		vectorTwoDim.push_back(copymata);
		automata = copymata;

		for (std::vector<std::string>::iterator i = automata.begin(); i != automata.end(); ++i)
		{
			std::cout << *i << " ";
		}

		std::cout << std::endl;
	}

	formattingFile.open("formatting.txt");
	int counter = 0;
	for (std::vector<std::vector<std::string>>::iterator i = vectorTwoDim.begin(); i != vectorTwoDim.end(); ++i)
	{
		for (std::vector<std::string>::iterator j = vectorTwoDim.at(counter).begin(); j != vectorTwoDim.at(counter).end(); ++j)
		{
			formattingFile << *j;
			
		}
		counter++;
		formattingFile << std::endl;
		
	}
	formattingFile.close();




	return 0;
}

 // https://opensource.org/licenses/MIT in
