#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>
#include "dem.h"
#include "Node.h"
#include "utils.h"
#include <time.h>
#include <list>
#include <unordered_map>

using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::getline;
using std::fstream;
using std::ifstream;
using std::priority_queue;
using std::binary_function;


typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;

int FillDEM_Zhou(char* inputFile, char* outputFilledPath);
int FillDEM_Wang(char* inputFile, char* outputFilledPath);
int FillDEM_Barnes(char* inputFile, char* outputFilledPath);
int FillDEM_Wei(char* inputFile, char* outputFilledPath);
//compute stats for a DEM
void calculateStatistics(const CDEM& dem, double* min, double* max, double* mean, double* stdDev)
{
	int width = dem.Get_NX();
	int height = dem.Get_NY();

	int validElements = 0;
	double minValue, maxValue;
	double sum = 0.0;
	double sumSqurVal = 0.0;
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (!dem.is_NoData(row, col))
			{
				double value = dem.asFloat(row, col);
				
				if (validElements == 0)
				{
					minValue = maxValue = value;
				}
				validElements++;
				if (minValue > value)
				{
					minValue = value;
				}
				if (maxValue < value)
				{
					maxValue = value;
				}

				sum += value;
				sumSqurVal += (value * value);
			}
		}
	}

	double meanValue = sum / validElements;
	double stdDevValue = sqrt((sumSqurVal / validElements) - (meanValue * meanValue));
	*min = minValue;
	*max = maxValue;
	*mean = meanValue;
	*stdDev = stdDevValue;
}
int main(int argc, char* argv[])
{
	
	if (argc < 4){
		cout<<"Fill DEM usage: FillDEM fillingMethod inputfileName outputfileName"<<endl;
		cout<<"wang: using the method in Wang and Liu (2006)"<<endl;
		cout<<"barnes:using the method in Barnes et al. (2014) method"<<endl;
		cout<<"zhou: using the method in Zhou et al. (2016) method"<<endl;
		cout<<"wei:using the implementation in the manuscript"<<endl;
		cout<<"\nFor example, FillDEM wang f:\\dem\\dem.tif f:\\dem\\dem_filled_wang.tif"<<endl;
		return 1;
	}
	
	string path(argv[2]);
	string outputFilledPath(argv[3]);
	size_t index = path.find(".tif");
	if (index ==string::npos) {
		cout<<"Input file name should have an extension of '.tif'"<<endl;
		return 1;
	}
	char* method=argv[1];
	string strFolder = path.substr(0, index);
	int Result;
	if (strcmp(method,"wang")==0)
	{
		Result=FillDEM_Wang(&path[0], &outputFilledPath[0]); //wang 2006
	}
	else if (strcmp(method,"barnes")==0)
	{
		Result=FillDEM_Barnes(&path[0], &outputFilledPath[0]); //barnes 2014
	}
	else if (strcmp(method,"zhou")==0)
	{
		Result=FillDEM_Zhou(&path[0], &outputFilledPath[0]); //zhou 2016
	}
	else if (strcmp(method,"wei")==0)
	{
		Result=FillDEM_Wei(&path[0], &outputFilledPath[0]); //wei
	}
	else 
	{
		cout<<"Unknown filling method"<<endl;
	}
	
	std::cout<<"\nPress any key to exit ..."<<endl;
	getchar();
	return 0;
}

