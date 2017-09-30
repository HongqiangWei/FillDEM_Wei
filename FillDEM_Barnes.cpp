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
#include <stack>
#include <unordered_map>
using namespace std;

typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;
//The implementation of the Priority-Flood algorithm in Barnes et al. (2014)
int FillDEM_Barnes(char* inputFile, char* outputFilledPath)
{
	CDEM dem;
	double geoTransformArgs[6];
	cout<<"Reading tiff file..."<<endl;
	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs))
	{
		printf("Error occurred while reading GeoTIFF file!\n");
		return 0;
	}

	int width = dem.Get_NX();
	int height = dem.Get_NY();
	cout<<"DEM Width:"<<  width<<"  Height:"<<height<<endl;

	cout<<"Using Barnes et al. (2014) method to fill DEM"<<endl;

	Flag flag;
	if (!flag.Init(width,height)) {
		printf("Failed to allocate memory!\n");
		return 0;
	}

	cout<<"\nStart filling depressions..."<<endl;
	time_t timeStart, timeEnd;
	timeStart = time(NULL);

	PriorityQueue queue;
	std::queue<Node> pitque;
	int validElementsCount = 0;
	// push border cells into the PQ
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			Node tmpNode;
			if (!dem.is_NoData(row, col))
			{
				validElementsCount++;
				for (int i = 0; i < 8; i++)
				{
					int iRow, iCol;
					iRow = Get_rowTo(i, row);
					iCol = Get_colTo(i, col);
					if (!dem.is_InGrid(iRow, iCol) || dem.is_NoData(iRow, iCol))
					{
						tmpNode.col = col;
						tmpNode.row = row;
						tmpNode.spill = dem.asFloat(row, col);
						queue.push(tmpNode);
						flag.SetFlag(row,col);
						break;
					}
				}
			}
			else {
				flag.SetFlag(row,col);
			}
		}
	}
	int percentFive = validElementsCount / 20;

	int count = 0;
	Node tmpNode;
	int iRow, iCol;
	float iSpill;
	int i;
	while (!queue.empty() || ! pitque.empty())
	{
		count++;
		if (count % percentFive == 0)
		{
			int percentNum = count / percentFive;
			cout<<"Progress:"<<percentNum * 5 <<"%\r";
		}
		if (!pitque.empty()) {
			tmpNode = pitque.front();
			pitque.pop();
		}
		else
		{
			tmpNode = queue.top();
			queue.pop();
		}
		int row = tmpNode.row;
		int col = tmpNode.col;
		float spill = tmpNode.spill;


		for (i = 0; i < 8; i++)
		{	iRow = Get_rowTo(i, row);
		iCol = Get_colTo(i, col);
		if (!flag.IsProcessed(iRow,iCol))
		{
			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill <= spill)
			{
				dem.Set_Value(iRow, iCol, spill);
				flag.SetFlag(iRow,iCol);						

				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = spill;
				pitque.push(tmpNode);
			}
			else
			{
				dem.Set_Value(iRow, iCol, iSpill);
				flag.SetFlag(iRow,iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = iSpill;
				queue.push(tmpNode);
			}

		}

		}
	}
	timeEnd = time(NULL);
	double consumeTime = difftime(timeEnd, timeStart);
	cout<<"\nTime used:"<<consumeTime<<" seconds"<<endl;

	double min, max, mean, stdDev;
	calculateStatistics(dem, &min, &max, &mean, &stdDev);

	CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(), 
		(void *)dem.getDEMdata(),GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);
	return true;
}
