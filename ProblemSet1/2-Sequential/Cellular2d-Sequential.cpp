/*
Copyright 2019 André Berge

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include<string>
#include<fstream>
#include<iostream>
#include<vector>
#include<map>

int main(int argc, char** argv)
{

	std::vector<std::vector<std::string>> automata;
	std::ifstream ruleFile, configFile;
	std::string rules = argv[1];
	std::string config = argv[2];
	int iterations = atoi(argv[3]);
	std::map<std::string, std::string> rulesLookup;
	std::string key, value;
	int line = 0;
	
	ruleFile.open(rules);
	while (ruleFile >> key >> value)
	{
		rulesLookup[key] = value;
		line++;

	}
	

	ruleFile.close();
	configFile.open(config);

	int configSize;
	configFile >> configSize;
	char c;
	int counter = 0;
	
	std::vector<std::string> v;

	while (configFile >> c)
	{
		v.push_back(std::string(1,c));
		
		if (++counter == configSize)
		{
			counter = 0;
			automata.push_back(v);
			v.clear();
		}
	}
	configFile.close();

	std::vector<std::vector<std::string>> copymata(automata);


	for (int i = 0; i < iterations; ++i)
	{
		
		for (int j = 0; j < automata.size(); ++j)
		{
			//std::cout << automata.size() << " " << automata[j].size() << std::endl;

			for (int k = 0; k < automata[j].size(); ++k)
			{
				std::string s;
				int up = (j + configSize - 1) % configSize;
				int down = (j + 1) % configSize;
				int left = (k + configSize - 1) % configSize;
				int right = (k + 1) % configSize;

				s = (automata.at(up).at(left) + automata.at(up).at(k) + automata.at(up).at(right) +
				automata.at(j).at(left) + automata.at(j).at(k) + automata.at(j).at(right) +
				automata.at(down).at(left) + automata.at(down).at(k) + automata.at(down).at(right));

				copymata[j][k] = rulesLookup[s]; 
				//std::cout << s << ", " << rulesLookup[s] << rulesLookup.size() << std::endl;
			}
		}
		automata = copymata;
		for (int j = 0; j < automata.size(); ++j)
		{
			for (int k = 0; k < automata[j].size(); ++k)
			{
				std::cout << automata[j][k];

			}
			std::cout << std::endl;
		}
		std::cout << "---------------------------------------------------------" << std::endl;
	}

	return 0;
}

 // https://opensource.org/licenses/MIT in
