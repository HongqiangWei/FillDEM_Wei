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
//The implementation of algorithm in the manuscript
int FillDEM_Wei(char* inputFile, char* outputFilledPath)
{
	CDEM DEM;
	double geoTransformArgs[6];
	cout<<"Reading tiff file..."<<endl;
	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, DEM, geoTransformArgs)){
		cout<<"error!"<<endl;
		return 0;
	}
	int width = DEM.Get_NX();
	int height = DEM.Get_NY();
	cout<<"DEM Width:"<<width<<"  Height:"<<height<<endl;

	// Spatial distribution of altitude for a transient surface converging to filled DEM
	CDEM W;
	W.SetHeight(height);
	W.SetWidth(width);
	if (!W.Allocate()){
		printf("Failed to allocate memory!\n");
		return 0;
	}
	//a matrix of Boolean values of the size of DEM
	//Initialize Flag as false
	Flag flag;
	if (!flag.Init(width,height)) {
		printf("Failed to allocate memory!\n");
		return 0;
	}
	time_t timeStart, timeEnd;
	timeStart = time(NULL);
	//Stage 1:Initialization of the surface to infinite altitudes
	
	int row,col;
	int i,iRow,iCol;
	queue<Node> P,Q;
	float HeadSpill;
	Node TempN,HeadN;
	int HeadNRow,HeadNCol;
	const float M=20000.0f;
	for (row=0;row<height;row++){
		for (col=0;col<width;col++){
			if (flag.IsProcessedDirect(row,col)) continue;
			if (DEM.is_NoData(row,col)){
				flag.SetFlag(row,col);
				for(i=0;i<8;i++){
					iRow=Get_rowTo(i,row);
					iCol=Get_colTo(i,col);
					if(flag.IsProcessed(iRow,iCol)) continue;
					if (DEM.is_InGrid(iRow, iCol)&&!DEM.is_NoData(iRow,iCol)){
						W.Set_Value(iRow, iCol,DEM.asFloat(iRow, iCol));
						TempN.row=iRow;
						TempN.col=iCol;
						TempN.spill=DEM.asFloat(iRow, iCol);
						P.push(TempN);
						flag.SetFlag(iRow, iCol);
					}
				}
			}
			else if(row==0 || row==height-1 || col==0 || col==width-1){
				W.Set_Value(row, col,DEM.asFloat(row, col));
				TempN.row=row;
				TempN.col=col;
				TempN.spill=DEM.asFloat(row, col);
				P.push(TempN);
				flag.SetFlag(row, col);
			}
			else{
				W.Set_Value(row,col,M);
			}
		}
	}
	//Stage 2:Removal of excess water
	while(!P.empty()||!Q.empty()){
		if (!P.empty()){
			HeadN=P.front();
			P.pop();
		}
		else{
			HeadN=Q.front();
			Q.pop();
		}
		HeadNRow=HeadN.row;
		HeadNCol=HeadN.col;
		HeadSpill=W.asFloat(HeadNRow,HeadNCol);
		for(i=0;i<8;i++){
			iRow=Get_rowTo(i,HeadNRow);
			iCol=Get_colTo(i,HeadNCol);
			if (flag.IsProcessed(iRow,iCol)) continue;
			if (DEM.asFloat(iRow,iCol)>=HeadSpill){
				TempN.row=iRow;
				TempN.col=iCol;
				TempN.spill=DEM.asFloat(iRow,iCol);
				W.Set_Value(iRow,iCol,DEM.asFloat(iRow,iCol));
				P.push(TempN);
				flag.SetFlag(iRow,iCol);
			}
			else if (W.asFloat(iRow,iCol)>HeadSpill){
				TempN.row=iRow;
				TempN.col=iCol;
				TempN.spill=HeadSpill;
				W.Set_Value(iRow,iCol,HeadSpill);
				Q.push(TempN);
			}
		}
	}

	timeEnd = time(NULL);
	double consumeTime = difftime(timeEnd, timeStart);
	cout<<"Time used:"<<consumeTime<<" seconds"<<endl;

	double min, max, mean, stdDev;
	calculateStatistics(W, &min, &max, &mean, &stdDev);

	CreateGeoTIFF(outputFilledPath, W.Get_NY(), W.Get_NX(), 
		(void *)W.getDEMdata(),GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);
	return 1;
}