// KosarajuStronglyConnectedComponentsGraph.cpp : Defines the entry point for the console application.
/*The file contains the edges of a directed graph. Vertices are labeled as positive integers from 1 to 875714. Every row indicates an edge, the vertex label in first column is the tail and the vertex label in second column is the head (recall the graph is directed, and the edges are directed from the first column vertex to the second column vertex). So for example, the 11th row looks liks : "2 47646". This just means that the vertex with label 2 has an outgoing edge to the vertex with label 47646

Your task is to code up the algorithm from the video lectures for computing strongly connected components (SCCs), and to run this algorithm on the given graph.

Output Format: You should output the sizes of the 5 largest SCCs in the given graph, in decreasing order of sizes, separated by commas (avoid any spaces). So if your algorithm computes the sizes of the five largest SCCs to be 500, 400, 300, 200 and 100, then your answer should be "500,400,300,200,100" (without the quotes). If your algorithm finds less than 5 SCCs, then write 0 for the remaining terms. Thus, if your algorithm computes only 3 SCCs whose sizes are 400, 300, and 100, then your answer should be "400,300,100,0,0" (without the quotes). (Note also that your answer should not have any spaces in it.)

WARNING: This is the most challenging programming assignment of the course. Because of the size of the graph you may have to manage memory carefully. The best way to do this depends on your programming language and environment, and we strongly suggest that you exchange tips for doing this on the discussion forums.
*/

#include "stdafx.h"
#define _CRTDBG_MAP_ALLOC  
#include<stdlib.h>
#include<assert.h>
#include <crtdbg.h>  

#define NUM_OF_NODES 875714
#define NUM_OF_SCC 5

/*Graph DS and Functions ==================================================================================
===========================================================================================================*/
struct ST_Edge {
	struct ST_Vertex *Adjvertex;
	bool outgoing; //This will be helpful in reverse graph
	struct ST_Edge *next;
};
struct ST_Vertex {
	int vertexId;
	//int finishingTime;
	//int leaderVertexId;
	bool visited;
	struct ST_Edge *edgePtr;
};
struct ST_Graph {
	int numberOfVertex;
	struct ST_Vertex **vertexes; //Array of vertices
};

void InitGraph(struct ST_Graph *graphPtr, int numOfVertex) {
	int i;
	graphPtr->numberOfVertex = numOfVertex;
	graphPtr->vertexes = (struct ST_Vertex**)malloc(sizeof(struct ST_Vertex)*numOfVertex);
	//memset(graphPtr->vertexes, 0, sizeof(struct ST_Vertex)*numOfVertex);
	for (i = 0; i < numOfVertex; i++) {
		graphPtr->vertexes[i] = (struct ST_Vertex*)malloc(sizeof(struct ST_Vertex));
		graphPtr->vertexes[i]->vertexId = i + 1;
		graphPtr->vertexes[i]->visited = false;
		graphPtr->vertexes[i]->edgePtr = NULL; //Initially NULL pointer!! will populate later
	}

}

//This will be called after InitGraph
void PopulateAdjacencyList(struct ST_Graph *graphPtr, int vertex, int edge, bool outgoing) {
	struct ST_Edge *temp = (struct ST_Edge*)malloc(sizeof(struct ST_Edge));
	temp->Adjvertex = graphPtr->vertexes[edge - 1];
	temp->outgoing = outgoing;  //Set true if outgoing edge
	temp->next = NULL;
	if (graphPtr->vertexes[vertex - 1]->edgePtr == NULL) {
		graphPtr->vertexes[vertex - 1]->edgePtr = temp;
	}
	else {
		struct ST_Edge *temp1 = graphPtr->vertexes[vertex - 1]->edgePtr;
		while (temp1->next) {
			temp1 = temp1->next;
		}
		temp1->next = temp;
	}
	
}

//Free memory
void FreeGraph(struct ST_Graph *graphPtr) {
	int vertices = graphPtr->numberOfVertex;
	struct ST_Edge *temp1, *temp2;
	int i;
	for (i = 0; i < vertices; i++) {
		temp1 = graphPtr->vertexes[i]->edgePtr;
		temp2 = temp1->next;
		while (temp1) {
			free(temp1);
			temp1 = temp2;
			if (temp2) {
				temp2 = temp2->next;
			}
		}
		free(graphPtr->vertexes[i]);
	}
	free(graphPtr->vertexes);
	graphPtr->vertexes = NULL;
}

void PrintGraph(struct ST_Graph *graphPtr) {
	int numOfVertex = graphPtr->numberOfVertex;
	int i;
	struct ST_Edge *temp;
	for (i = 0; i < graphPtr->numberOfVertex; i++) {
		printf("\n%d", graphPtr->vertexes[i]->vertexId);
		temp = graphPtr->vertexes[i]->edgePtr;
		while (temp) {
			if (temp->outgoing) {
				printf("--->");
			}
			else {
				printf("<---");
			}
			printf("%d", temp->Adjvertex->vertexId);
			temp = temp->next;
		}
	}
}

//Parse the input text file and create the Graph
void CreateGraph(struct ST_Graph *graphPtr) {
	FILE *fd;
	char line[100];
	char *tempStr;
	int i = 0;
	int numbersArr[2];
	int numbersCount = 0;
	int j = 0;
	int num;
	int edge;
	memset(numbersArr, -1, 2);
	
	//We know that the number of nodes is 875714
	InitGraph(graphPtr, NUM_OF_NODES);
	//InitGraph(graphPtr, 9);

	//Open File and Store it as Adjacaency List
	fopen_s(&fd, "SCC.txt", "r+");
	while (fgets(line, 100, fd) != NULL) {
		i = 0;
		j = 0;
		numbersCount = 0;
		while (line[i] != '\n') {
			if (line[i] == ' ') {
				tempStr = (char*)malloc(sizeof(char)*(i - j + 1));
				memcpy(tempStr, (line + j), (i - j));
				tempStr[i - j] = '\0';
				num = atoi(tempStr);
				numbersArr[numbersCount] = num;
				numbersCount++;
				j = i + 1;
				free(tempStr);
			}
			i++;
			if (i > 99) {
				break;
			}
		}
		//Now we have our array, lets store in Graph
		num = numbersArr[0]; //Vertex ID(base zeroed!!)
		edge = numbersArr[1]; //Outgoing Edge
		//No self loops
		if (num != edge) {
			//Insert Adjacency list outgoing to vertex
			PopulateAdjacencyList(graphPtr, num, edge, true);
			//Insert Adjacency List, incoming vertex, this will help in reverse graph
			PopulateAdjacencyList(graphPtr, edge, num, false);
		}
		
	}
	fclose(fd);
	i = 0;
	j = 0;
}
/*Graph DS and Functions End ==============================================================================
===========================================================================================================*/

/*=========================================================================================================
								DFS FUNCTIONS
===========================================================================================================*/
void DFS(struct ST_Graph *graphPtr, int vertexId, bool runRevGraph, int *finishingTime, int *finishingTimeArr) {
	//Mark node as explored
	graphPtr->vertexes[vertexId]->visited = true;
	int nextId;
	//Set Leader

	struct ST_Edge *temp = graphPtr->vertexes[vertexId]->edgePtr;
	while (temp) {
		//If the vertex is not visited
		if (!temp->Adjvertex->visited) {
			//Find the outgoing edge!! if reversegrah look for incoming 
			if (runRevGraph) {
				if (!temp->outgoing) {
					//printf("\nRecursing: OriginalVertex: %d", graphPtr->vertexes[vertexId]->vertexId);
					DFS(graphPtr, temp->Adjvertex->vertexId-1, runRevGraph, finishingTime, finishingTimeArr);
				}
			}
			else {
				if (temp->outgoing) {
					DFS(graphPtr, temp->Adjvertex->vertexId-1, runRevGraph, finishingTime, finishingTimeArr);
				}
			}
		}
		temp = temp->next;
		
	}
	
	finishingTimeArr[*finishingTime] = graphPtr->vertexes[vertexId]->vertexId; 
	*finishingTime += 1;
}

void DFS_2(struct ST_Graph *graphPtr, int vertexId, int leader, int *leaderCount, int *leaderArr) {
	//Mark node as explored
	graphPtr->vertexes[vertexId]->visited = true;
	
	//Set Leader
	leaderArr[vertexId] = leader;
	*leaderCount += 1;
	struct ST_Edge *temp = graphPtr->vertexes[vertexId]->edgePtr;
	while (temp) {
		//If the vertex is not visited
		if (!temp->Adjvertex->visited) {
			//Find the outgoing edge!! if reversegrah look for incoming 			
			if (temp->outgoing) {
				DFS_2(graphPtr, temp->Adjvertex->vertexId - 1, leader, leaderCount, leaderArr);
			}			
		}
		temp = temp->next;

	}
}



/*=========================================================================================================
								DFS_END
===========================================================================================================*/


/***********************************************************************************************************
Kosaraju Algo
************************************************************************************************************/
//First Pass ...Run on reverse graph 
void GenerateFinishingTimeOnReverseGraph(struct ST_Graph *graphPtr, int *finishingTimeNodeArr) {
	
	//DFS_LOOP(graphPtr, true, finishingTimeNodeArr);
	int finishingTime = 0;
	int i = graphPtr->numberOfVertex - 1;
	for (i; i >= 0; i--) {
		if (graphPtr->vertexes[i]->edgePtr) {
			//Check if node has not been visited!!
			if (!graphPtr->vertexes[i]->visited) {
				//Now call DFS 
				DFS(graphPtr, i, true, &finishingTime, finishingTimeNodeArr);
			}
		}
	}
	
}

//Second Pass: Run on graph ..Iterate from max Finish Time vertices and keep count of max leader
int *SetTheConnectedNodeCounts(struct ST_Graph *graphPtr, int *finishingTimeArr, int *leaderArr) {
	int leader = 0;
	int leaderCount = 0;
	int i = graphPtr->numberOfVertex - 1;
	int *localArr = (int*)malloc(sizeof(int) * NUM_OF_SCC);
	memset(localArr, 0, sizeof(int) * NUM_OF_SCC);
	int min;
	int loc;
	for (i; i >= 0; i--) {
		leader = finishingTimeArr[i];
		leaderCount = 0;
		if (!graphPtr->vertexes[finishingTimeArr[i] - 1]->visited) {
			DFS_2(graphPtr, finishingTimeArr[i] - 1, leader, &leaderCount, leaderArr);

			//Check the local arr and replace the min
			min = localArr[0];
			loc = 0;
			for (int j = 1; j < NUM_OF_SCC; j++) {
				if (min > localArr[j]) {
					min = localArr[j];
					loc = j;
				}
			}
			//Now update loc Arr 
			if (leaderCount > min) {
				localArr[loc] = leaderCount;
			}
		}
	}
	
	return localArr;
}
int *Kosaraju_SCC(struct ST_Graph *graphPtr) {
	//This Arr will store the sorted finishing time locations for the vertex
	int *finishingTimeNodeArr = (int*)malloc(sizeof(int)*graphPtr->numberOfVertex);
	memset(finishingTimeNodeArr, 0, sizeof(int)*graphPtr->numberOfVertex);
	
	//Helpful in local drbug...not really needed though for the question asked!!
	int *leaderArr = (int*)malloc(sizeof(int)*graphPtr->numberOfVertex);
	memset(leaderArr, 0, sizeof(int)*graphPtr->numberOfVertex);
	
	//Fist Step ...get the finishing times
	GenerateFinishingTimeOnReverseGraph(graphPtr, finishingTimeNodeArr);
	
	//Now mark all nodes as unvisited
	for (int i = 0; i < graphPtr->numberOfVertex; i++) {
		graphPtr->vertexes[i]->visited = false;
	}
	//Get the Connections
	int *maxArr = SetTheConnectedNodeCounts(graphPtr, finishingTimeNodeArr, leaderArr);
	
	free(leaderArr);
	free(finishingTimeNodeArr);
	return maxArr;
}

/************************************************************************************************************************
Merge Sort Routine
************************************************************************************************************************/
void Merge(int *arr, int left, int middle, int right) {
	int l = middle - left + 1;
	int r = right - middle;
	int *arr1 = (int *)malloc(sizeof(int)*(middle - left + 1));
	int *arr2 = (int*)malloc(sizeof(int)*(right - middle));
	int i, j, k;
	for (i = 0; i < l; i++) {
		arr1[i] = arr[left + i];
	}
	for (i = 0; i < r; i++) {
		arr2[i] = arr[middle + 1 + i];
	}
	i = 0;
	j = 0;
	k = left;
	
	while (i < l && j < r) {
		if (arr1[i] > arr2[j]) {
			arr[k] = arr1[i];
			i++;
		}
		else {
			arr[k] = arr2[j];
			j++;
		}
		k++;
	}
	while (i < l) {
		arr[k] = arr1[i];
		i++;
		k++;
	}
	while (j < r) {
		arr[k] = arr2[j];
		j++;
		k++;
	}
	free(arr1);
	free(arr2);
}
void MergeSort(int *arr, int left, int right) {
	if (left < right) {
		int mid = (right + left) / 2;
		MergeSort(arr, left, mid);
		MergeSort(arr, mid + 1, right);
		Merge(arr, left, mid, right);
	}
}

/**********************************************************************************************************/
int main()
{
	//VS debug flags to catch mem leak
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	struct ST_Graph *myGraph = (struct ST_Graph*)malloc(sizeof(struct ST_Graph));
	CreateGraph(myGraph);
	
	int *maxArr = Kosaraju_SCC(myGraph);
	
	//Sort Array in Decreasing Order
	MergeSort(maxArr, 0, NUM_OF_SCC-1);
	printf("\n\nMax Arr: ");
	for (int i = 0; i < NUM_OF_SCC; i++) {
		printf("%d\t", maxArr[i]);
	}
	FreeGraph(myGraph);
	free(myGraph);
	free(maxArr);
    return 0;
}

