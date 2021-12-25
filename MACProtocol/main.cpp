/*
* Author: Mohamed Yassin
* File name: main.cpp
*/
#include <iostream>
#include <math.h>
#include <fstream> //for file IO
#include <cstring> //for memcpy when i split my array in two
using namespace std;

/*
I use this function to fill my array with 0 or 1s depending on if the customer has an item at that slot.
So if K = 3, my array is 8 slots big, and if the customer only has items labeled 5,6, my array would be
0 0 0 0 0 1 1 0
as well, i do size(sizeof(int)) because sizeof an array in C is usually multiplied by 4 or 8 depending on what your unit is, and for ints,
its multiplied by 4, so i had to divide by sizeof(int) which is 4
*/
void fillArray(int arr[], int sizeArr, int customerArr[], int sizeCustomerArr)
{
    for (int i = 0; i < sizeArr/(sizeof(int)); i++) {
            arr[i] = 0; //fill 0 initially
    }
    for (int j = 0; j < sizeCustomerArr/(sizeof(int)); j++)
    {
        int index = customerArr[j];
        arr[index] = 1; //put the item's label at the correct slot in array
    }
}
/*
this is a simple loop to print my array, just helps in testing to visualize how my array is being manipulated.
*/
void printArray(int arr[], int sizeArr)
{
    for (int i = 0; i < sizeArr/(sizeof(int)); i++){
        cout << arr[i] << " "; //add a space between elements
    }
    cout << endl; //new line when array is printed
}
/*
helper function for RootLevelScan, this is used to calculate the sum of the contents of an array, in the RootLevelScan,
I split the array in two halves repeatedly and call this function.
*/
int sumOfArray(int arr[], int sizeArr)
{
    int sum = 0;
    for (int i = 0; i < sizeArr/(sizeof(int)); i++)
    {
        sum += arr[i]; //keep adding on to sum
    }
    return sum;
}
//Global variables
double probes = 0;
double idles = 0;
double collisions = 0;
double success = 0;
/*
This is my recursive root level scan function, the comments inside the code help explain how it works
arr[] is the array we filled with 0s or 1s depending on the customers cart.
*/
int RootLevelScan(int arr[], int K)
{
    int range = pow(2,K); //range of values inside array is 2^K
    int firstHalf[range/2]; //split array in 2
    int secondHalf[range/2]; //split array in 2
    int *dst = &arr[0]; //point to first half of array
    int *dst2 = &arr[range/2]; //point to second half of array
    memcpy(firstHalf, dst ,sizeof(int)*(range/2)); //use memcpy and dst to copy first half of array
    memcpy(secondHalf, dst2 ,sizeof(int)*(range/2)); //use memcpy and dst2 to copy second half of array

    //Base Cases:
    //If the sum of the halves of the array is equal to exactly one, it means u have a success. return from here
    if ((sumOfArray(firstHalf, sizeof(firstHalf)) + sumOfArray(secondHalf, sizeof(secondHalf))) == 1)
    {
        probes++;
        success++;
        return 0;
    }
    //if the sum of the two halves of the array is equal to exactly 0, it means you have an idle, because
    //the "child nodes" have nothing in that case. return from here
    if ((sumOfArray(firstHalf, sizeof(firstHalf)) + sumOfArray(secondHalf, sizeof(secondHalf))) == 0)
    {
        probes++;
        idles++;
        return 0;
    }
    //When you are right above leaf level (k==1) and both child nodes have an item, that is a collison, but it also
    //has two successes and 3 probes total, since the two child nodes left dont have any more children, so we add those and return,
    //Even though our array is not a binary tree representation, it still behaves like one, that is why i am using terms like node and leaf level.
    if (((sumOfArray(firstHalf, sizeof(firstHalf)) + sumOfArray(secondHalf, sizeof(secondHalf))) > 1) && K == 1)
    {
        collisions++;
        probes += 3;
        success += 2;
        return 0;
    }
    //Recursive Case:

    //Otherwise when there are more than 1 elements in the halves of the array, that is a collision, and repeat RootLevelScan for left half and right
    //half using K-1 to make the array half the size, until we reach any of the base cases.
    if ((sumOfArray(firstHalf, sizeof(firstHalf)) + sumOfArray(secondHalf, sizeof(secondHalf))) > 1)
    {
        probes++;
        collisions++;
        RootLevelScan(firstHalf, K-1);
        RootLevelScan(secondHalf, K-1);
    }
}
/*
This is the leaf level scan function, it is very simple, iterate through the array using the given range depending on K's input,
and if there's a value == 1 there, it means we have a success, if there is a value == 0, it is an idle.
Number of total probes/ total time is always 2^K.
*/
void LeafLevelScan(int arr[], int K)
{
    double totalSuccess = 0;
    double totalFailure = 0;
    double totalIdle = 0;
    double totalProbes = 0;
    int range = pow(2,K); //range is 2^K
    for (int i = 0; i < range; i++)
    {
        if (arr[i] == 1){ //if there is an item
             totalProbes++;
             totalSuccess++;
        }
        else { //if there is no item
            totalProbes++;
            totalIdle++;
        }
    }
    //Print results
    cout <<"Total Successful Scans: " << totalSuccess << endl;
    cout <<"Total Idle Scans: " << totalIdle << endl;
    cout << "Total Efficiency : " << ((totalSuccess-totalFailure)/range)*100  << "%" << endl;
    cout <<"Total Time: " << totalProbes << endl;
}

int main()
{
    //While(1) so that the program doesnt close everytime you input a test case.
    while (1)
    {
        //Getting user value for K
        cout << endl;
        int K;
        cout << "Enter a value for K: ";
        cin >> K;
        cout << K << endl;

        int sizeOfArr = pow(2,K); //Size of array is 2^K, dynamic based on user input
        int arr[sizeOfArr]; //Create array with size 2^K

        int numberOfItems = 0;
        string name; //name of file
        cout << "Enter the name of the file you wish to read, example is Customer1.txt or Customer2.txt : ";
        cin >> name;
        ifstream myfile(name);
        string line;
        if (myfile.is_open()) //File IO to open file
        {
            while (getline(myfile, line))
            {
                numberOfItems++; //Calculating the total number of items in the customers shopping cart
            }
            myfile.close();
        }
        int customerArr[numberOfItems]; //we use the number of items in the shopping cart to make a customer array of the correct size
        cout << "Number Of Items in Customer's Bag: " << numberOfItems << endl;

        ifstream myfile2(name);
        if (myfile2.is_open()) //File IO to open file
        {
            int i = 0;
            while (getline(myfile2, line))
            {
                customerArr[i] = stoi(line); //adding the integer value of the customer's cart items into the array we previously made
                i++;
            }
            myfile2.close();
        }
       // printArray(customerArr, sizeof(customerArr));

        fillArray(arr, sizeof(arr), customerArr, sizeof(customerArr)); //now, we use our fill function to properly model our array so that we can run functions on it.

        //printArray(arr, sizeof(arr));
        cout << sumOfArray(arr, sizeof(arr)) << endl; //just for testing
        cout << "Do you want to do root level or leaf level scan? write root or leaf" << endl;
        string mode;
        cin >> mode; //Letting users pick root or leaf level
        if (mode == "root") //If user wants to scan from root level
        {
            //Resetting global variables
            probes = 0;
            idles = 0;
            collisions = 0;
            success = 0;
            RootLevelScan(arr, K); //call leaf level scan with user's K value and the array we got from the input
            //Print results
            cout <<"Total Probes: " << probes << endl;
            cout <<"Total Collisions: " << collisions << endl;
            cout <<"Total Successful Scans: " << success << endl;
            cout <<"Total Idle Scans: " << idles << endl;
            cout << "Total Efficiency : " << ((success)/probes) *100  << "%" << endl;
        }
        if (mode == "leaf") //else if user wants to scan from leaf level
        {
            LeafLevelScan(arr, K);
        }
    }
}

