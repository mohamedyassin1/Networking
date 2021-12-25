/*
    File Name: main.cpp
    Author: Mohamed Yassin
*/
#include <iostream> //Needed for many C++ things like cout, cin
#include <vector>   //Needed because I modeled my graph with vectors
#include <string>
#include <algorithm> //Don't need it but kept just in case since I used it earlier in the code.
#include <fstream>   //For reading data from a txt file
#include <climits>  //Included this so my code compiles on CPSC servers
using namespace std;
/*  This struct is what I used to model my vertices, since each vertice has a Name, in our case a city
    and also had a distance, distance is 0 from the city to itself, but as you can see later on, when we add
    edges to our adjacency list, each city would have a distance that represents its distance from the original vertice
*/
int hops = 0;  //Used to count number of hops
struct Cities{
    string cityName;
    int distance;
};
/*
    This method returns the index of a vertice in my adjacency list(graph), for example if my list looks something like this:
        YYC,0-->YEG,157-->YQR,440-->YXE,365-->YVR,720-->YXX,666-->YXS,514-->
        YEG,0-->YYC,157-->YXE,322-->YVR,735-->YXY,1297-->YZF,904-->YFB,1680-->YXS,468-->
        YQR,0-->YYC,440-->YWG,362-->YXE,164-->
    And the airportCode is YQR, the index returned would be 2.

*/
int returnIndexOfCity(vector<vector<Cities>> graph, string airportCode)
{
    for (int i = 0; i < graph.size(); i++){
        if (graph.at(i).begin()->cityName == airportCode) //if first element of vector has a cityName equal to airportCode, return i
        {
            return i;
        }
    }
    return -1; //else return -1 if that airport code doesn't exist.
}
/*
    This method is used to add edges vertices in my adjacency list, there are further explanations inside the method
    It takes in a reference graph so that I can directly modify it.
*/
void addEdge(vector<vector<Cities>> &graph, string city1, string city2, int distance)
{
    bool containsVertice = false; //Initially, we assume the vertices don't exist in the graph (for example city1 and city2 don't have vertices yet)
    bool containsVertice2 = false;
    for (int i = 0; i < graph.size(); i++)
    {
        if (graph[i].front().cityName == city1)
        {
            containsVertice = true; //If the first city exists in the graph, then we change containsVertice to true since it contains that vertice
            Cities details;
            details.cityName = city2;
            details.distance = distance;    //We made a new struct and added city2 and the distance to our vertice, since city2 is the other vertice
                                            //that forms an edge with city1
            graph[i].push_back(details);    //and at the vector[i] inside graph, we push back that struct and that is how we add edges between vertices
        }
        if (graph[i].front().cityName == city2) //If the second city exists in the graph, then we change containsVertice2 to true since it contains that vertice
        {
            containsVertice2 = true;
            Cities details;
            details.cityName = city1;      //We made a new struct and added city1 and the distance to our vertice, since city1 is the other vertice
            details.distance = distance;   //that forms an edge with city1
            graph[i].push_back(details);   //and at the vector[i] inside graph, we push back that struct and that is how we add edges between vertices
        }
    }
    if (containsVertice == false)        //otherwise if the vertice doesn't exist(city1 doesn't have an index in our list), add it to our adjacent list
    {
        vector<Cities> addingCity;
        Cities first;
        Cities second;
        first.cityName = city1;
        first.distance = 0;
        second.cityName = city2;
        second.distance = distance;
        addingCity.push_back(first);     //since city1 doesn't exist, push that back first so it is the index of the vector
        addingCity.push_back(second);   //then push city2
        graph.push_back(addingCity);
    }
    if (containsVertice2 == false) //Same as the above if statement but in this case if city2 doesn't have an index
    {
        vector<Cities> addingCity;
        Cities first;
        Cities second;
        first.cityName = city2;
        first.distance = 0;
        second.cityName = city1;
        second.distance = distance;
        addingCity.push_back(first);
        addingCity.push_back(second);
        graph.push_back(addingCity);
    }

}
//This is a method to print my adjacency list/graph , code is somewhat inspired from programiz which I referenced above.
void printGraph(vector<vector<Cities>> graph)
{
      for (int d = 0; d < graph.size(); ++d) {
    for (auto x : graph[d]) { //for every element inside graph[d]
      cout << x.cityName << "," << x.distance << "-->"; //print the element
    }
    printf("\n"); //print new line after every array
  }
}
//Used to find the minimum distance between current vertice from our original vertice
int minDistance(int dist[], bool visited[], int graphSize)
{
    int Min = INT_MAX; //We set Min to MAX initially so that if anything smaller than Min is the new smallest
    int index = 0;
    for (int i = 0; i < graphSize; i++)
    {
        if (dist[i] <= Min && visited[i] == false) //if the dist at I is smallest than Min at I and it's not visited yet, we set Min to dist[i]
        {
            Min = dist[i];  //Min is the distance at i now. Then we would loop again for all indices in the array
            index = i; //set index to be i, which is the shortest distance
        }
    }
    return index;
}
//This function is to print the path and the number of hops it took to reach our target destination
//This logic is referenced from the website I referenced above for printing paths
void printPath(int parent[], int target,vector<vector<Cities>>graph)
{

    // Base Case : If target is source
    if (parent[target] == - 1){
        cout << graph[target][0].cityName;//printing the parent
        return;
    }
    hops++; //Incrementing number of hops everytime we travel to a new destination
    printPath(parent, parent[target], graph);

    cout << "-->" << graph[target][0].cityName;
}

//This is my dijkstra algorithm, I will explain how it works step by step in the code
//It is inspired from a youtube video I listed above and I also modified the logic to work with my data structure
void dijkstra(vector<vector<Cities>>graph, int starting, int target)
{
    int dist[graph.size()]; //An array of all distances of each vertex from source
    bool visited[graph.size()] = {}; //array of visited vertices, we set each array element to true once we compute minimum distance for the vertice from source later in the function
    int parent[graph.size()];
    for (int i = 0; i < graph.size(); i++)
    {
        parent[i] = -1;
        dist[i] = INT_MAX; //Distance to other vertices from starting is currently unknown, therefore we give a very high number
    }
    dist[starting] = 0;   //Distance of the starting vertex from our starting vertix is 0
    for (int i = 0; i < (graph.size() -1); i++) //Don't need to check last index because all of its neighbors will already be checked, thus, we use graph.size() - 1
    {
        int u = minDistance(dist, visited, graph.size()); //we return the index that contains minimum distance from starting to any other vertex
        visited[u] = true; //set u to be visited since we already checked that index that we returned from minDistance
        for (Cities c: graph[u]){
            int x = returnIndexOfCity(graph, c.cityName); //get index of the city C in the graph, so if it's the third element in the vector, look for its index in the 2D vector (the cityName should be the first element of one of the vectors)
            if (visited[x] == false && dist[u] != INT_MAX && dist[x] > (dist[u] + c.distance))
            {
                parent[x] = u;
                dist[x] = dist[u] + c.distance; //If the index of the neighbor has not yet been visited AND the distance at that element is not INT_MAX AND
                                                //if the previous path from dist[x] to dist[u] is greater than the distance to dist[u], then set dist[x] to be the distance at dist[u]
            }
        }
    }
    cout << dist[target] << "  "; //print the distance to the target
    hops = 0; //reset hops to 0 before printing path again
    printPath(parent,target,graph);
    cout << "  " << hops << "hops";
    cout << endl;
}
int main()
{
    vector<vector<Cities>> graph; //Creating our graph, it is a vector of vectors
    //First, we create a Graph
    string line;
    string city1;
    string city2;
    int distance; //These string and
    ifstream myfile ("MapOfCanada.txt"); //Reading the txt file "MapOfCanda.txt"
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            city1 = line.substr(0,3);
            city2 = line.substr(4,3);
            distance = stoi((line.substr(8)));
            cout << city1 << " " << city2 << " " << distance << endl; //This is to print my cities just to check if I read the file in correctly
            addEdge(graph,city1,city2,distance); //we add edges to make our adjacent list at this point using the parsed strings
        }
        myfile.close();
    }
    else cout << "Unable to open file";
    printf("\n");
    printGraph(graph); //Testing our adjacent list
    printf("\n\nResults: \n");
    dijkstra(graph,returnIndexOfCity(graph,"YYC"), returnIndexOfCity(graph,"YYZ")); //CAN1
    dijkstra(graph,returnIndexOfCity(graph,"YYC"), returnIndexOfCity(graph,"YOW")); //CAN2
    dijkstra(graph,returnIndexOfCity(graph,"YYC"), returnIndexOfCity(graph,"YVR")); //CAN3
    return 0;
}
