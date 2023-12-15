#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <chrono>
#include <climits>
#include <random>
#include <vector>
#include <ctime>
#include <cmath>
#include <list>
#include <algorithm>
#include <unordered_map>

#define CLB 0
#define RAM 1
#define DSP 2
#define IO 3

using namespace std;

class Arg
{
public:
    Arg(int argc, char *argv[])
    {
        // cout << "argc" << argc << endl;
        if (argc != 5)
        {
            cout << "Input format should be the following." << endl;
            cout << "eg: ./legalizer ./input/testcase1/architecture.txt ./input/testcase1/instance.txt ./input/testcase1/netlist.txt ./output/output1.txt" << endl;
            cout << "exiting..." << endl;
            exit(1);
        }
        archPath = argv[1];
        instPath = argv[2];
        netPath = argv[3];
        outPath = argv[4];

    }

    string archPath, instPath, netPath, outPath;
};

class Resource
{
public:
    Resource(int _Rid, float _x, float _y)
        : Rid(_Rid), x(_x), y(_y)
    {
        used = false;
    }
    int Rid, inst;
    float x, y;
    bool used;
};

class Instance
{
public:
    Instance(int _Iid, int _type, float _x, float _y, string _name)
        : Iid(_Iid), type(_type), x(_x), y(_y), name(_name)
    {
        rsrc = -1;
    }
    int Iid, type, rsrc;
    float x, y;
    string name;
    vector<int> net;
};

class Net
{
public:
    Net() {}
    // int T_Nid, Nid;
    string name;
    vector<int> insts;
};