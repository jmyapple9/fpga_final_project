// #include "classes.hpp"
#include "util.hpp"

ifstream in_file;
vector<vector<Slot>> Resource;
vector<Slot> clb, ram, dsp;
vector<Instance> instances, bestInstances;
vector<Net> nets;
unordered_map<string, int> inst_name_id;

double bestWL{DBL_MAX}, perturbWL{0.0}, originWL{0.0};
pair<int, int> prev_swap;
int oldtype, oldr1, oldr2;

void archParser(string path)
{
    in_file.open(path);
    if (in_file.fail())
    {
        cout << "Fail opening file: " << path << endl;
        exit(1);
    }
    istringstream iss;
    string line, typeStr, name;
    float x, y;
    int Rid = 0, type;
    while (getline(in_file, line))
    {
        istringstream iss(line);
        iss >> name >> typeStr >> x >> y;

        type = (typeStr != "DSP") ? ((typeStr != "RAM") ? 0 : 1) : 2;
        Resource[type].emplace_back(Slot(Rid++, x, y));
        // if (typeStr == "CLB")
        //     Resource[CLB].emplace_back(Slot(Rid++, x, y));
        // else if (typeStr == "RAM")
        //     Resource[RAM].emplace_back(Slot(Rid++, x, y));
        // else if (typeStr == "DSP")
        //     Resource[DSP].emplace_back(Slot(Rid++, x, y));
        // else
        //     cout << "Error! There's no type \"" << typeStr << "\"\n";
    }

    in_file.close();
}

void instParser(string path)
{

    in_file.open(path);
    if (in_file.fail())
    {
        cout << "Fail opening file: " << path << endl;
        exit(1);
    }

    istringstream iss;
    string line, typeStr, name;
    float x, y;
    int Iid = 0, type = -1;

    while (getline(in_file, line))
    {
        istringstream iss(line);
        iss >> name >> typeStr >> x >> y;

        if (typeStr == "CLB")
            type = 0;
        else if (typeStr == "RAM")
            type = 1;
        else if (typeStr == "DSP")
            type = 2;
        else if (typeStr == "IO")
            type = 3;
        else
            cout << "Error! There's no type \"" << typeStr << "\"\n";

        inst_name_id[name] = Iid;
        // if (type != 3)
        // {
        instances.emplace_back(Instance(Iid++, type, x, y, name));
        // }
    }

    in_file.close();
}

void netParser(string path)
{
    in_file.open(path);
    if (in_file.fail())
    {
        cout << "Fail opening file: " << path << endl;
        exit(1);
    }

    istringstream iss;
    string token, line, instName, netName;
    int netId = 0;
    while (getline(in_file, line))
    {
        istringstream iss(line);
        bool first = true;
        Net net;
        while (iss >> instName)
        {
            if (first)
            {
                net.name = instName;
                first = false;
                continue;
            }
            int instId = inst_name_id[instName];
            net.insts.emplace_back(instId);
            instances[instId].net.emplace_back(netId);
        }
        nets.emplace_back(net);
        ++netId;
    }

    in_file.close();
}

double HPWL()
{

    // cout << "Enter HPWL" << endl;
    // auto start_time = chrono::system_clock::now();
    // compute ttl WL
    double hpwl = 0;
    for (auto net : nets)
    {
        double
            xmax{numeric_limits<double>::lowest()},
            xmin{numeric_limits<double>::max()},
            ymax{numeric_limits<double>::lowest()},
            ymin{numeric_limits<double>::max()};

        for (auto instId : net.insts)
        {
            // cout << "1";
            double x{instances[instId].x}, y{instances[instId].y};
            // cout << "2";
            xmax = max(xmax, x);
            xmin = min(xmin, x);
            ymax = max(ymax, y);
            ymin = min(ymin, y);
        }
        hpwl += ((xmax - xmin) + (ymax - ymin));
    }
    // cout << "hpwl after initPlace: " << hpwl << endl;
    // auto realDuration = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start_time);
    // cout << "duration: " << realDuration.count() << " seconds." << endl;
    // cout << "end HPWL" << endl;
    return hpwl;
}

void initPlace()
{
    // cout << "Enter initPlace" << endl;
    int clbIdx{0}, ramIdx{0}, dspIdx{0};
    for (auto &inst : instances)
    {
        if (inst.type == 0)
        {
            inst.rsrc = clbIdx;
            inst.x = Resource[CLB][clbIdx].x;
            inst.y = Resource[CLB][clbIdx].y;
            Resource[CLB][clbIdx++].stored = inst.Iid;
        }
        else if (inst.type == 1)
        {
            inst.rsrc = ramIdx;
            inst.x = Resource[RAM][clbIdx].x;
            inst.y = Resource[RAM][clbIdx].y;
            Resource[RAM][ramIdx++].stored = inst.Iid;
        }
        else if (inst.type == 2)
        {
            inst.rsrc = dspIdx;
            inst.x = Resource[DSP][clbIdx].x;
            inst.y = Resource[DSP][clbIdx].y;
            Resource[DSP][dspIdx++].stored = inst.Iid;
        }
        else
        {

            ; // don't need to place IO block
        }
    }
    originWL = HPWL();
}

bool accept(double cost, int T, double baselineWL)
{
    double random = static_cast<double>(rand()) / RAND_MAX;
    bool ac = exp(-cost*10000 / T ) > random;
    // cout << (ac ? "accept" : "notAccept") << ": " << exp(-cost*1000 / T )  << endl;

    return ac;
}
// 46815
struct ascendingX
{
    inline bool operator()(const int &instId1, const int &instId2)
    {
        return (instances[instId1].x < instances[instId2].x);
    }
};

struct ascendingY
{
    inline bool operator()(const int &instId1, const int &instId2)
    {
        return (instances[instId1].y < instances[instId2].y);
    }
};
void Swap(int type, int r1, int r2, bool reverting)
{
    // if (reverting)
    //     cout << "r: " << type << ", " << r1 << ", " << r2 << endl;
    // else
    //     cout << "s: " << type << ", " << r1 << ", " << r2 << endl;

    // cout << "enter swap: " << type<<", " << r1 << ", " << r2 << endl;
    Slot &slot1 = Resource[type][r1];
    Slot &slot2 = Resource[type][r2];
    Instance &inst1 = instances[slot1.stored];

    if (slot1.stored == -1)
        cout << "Error! slot1 should never be empty" << endl;

    if (slot2.stored == -1)
    {
        slot2.stored = inst1.Iid;
        slot1.stored = -1;

        inst1.rsrc = r2;
        inst1.x = slot2.x;
        inst1.y = slot2.y;
    }
    else
    {
        // cout << "swap 2" << endl;
        Instance &inst2 = instances[slot2.stored];
        // Instance tmp1 = instances[slot1.stored];
        if (inst1.type != inst2.type)
            cout << "Error! try swapping instances in different type!" << endl;

        slot1.stored = inst2.Iid;
        slot2.stored = inst1.Iid;

        inst1.x = slot2.x;
        inst1.y = slot2.y;
        inst1.rsrc = r2;
        // inst1.rsrc = slot2.Rid;

        inst2.x = slot1.x;
        inst2.y = slot1.y;
        inst2.rsrc = r1;
        // inst2.rsrc = slot1.Rid;
    }
    // cout << "exit swap" << endl;
}

void perturb()
{
    // cout << "Enter perturb" << endl;
    int instID, ResID;

    do // prevent get IO instance
    {
        instID = rand() % instances.size();
    } while (instances[instID].type == 3);

    Instance &randInst = instances[instID];

    ResID = rand() % Resource[randInst.type].size();
    // Slot &slot = Resource[randInst.type][ResID];
    oldtype = randInst.type, oldr1 = randInst.rsrc, oldr2 = ResID;
    Swap(randInst.type, randInst.rsrc, ResID, false);

    // int cnt = 0;
    // for (size_t i = 0; i < Resource[0].size(); i++)
    //     if (Resource[0][i].stored != -1)
    //         cnt++;
    // cout << cnt << " clb in total" << endl;
}

void revert()
{
}

void SA()
{
    cout << "Enter SA()" << endl;
    double reject, reduceRatio = 0.9999;
    int nAns, uphill, T = 1000000, N = 70; // N: number of answer in T
    bestWL = originWL;
    do
    {
        reject = nAns = uphill = 0.;

        while (uphill < N and nAns < 2 * N)
        {
            // int act = rand() % 4;
            perturb();
            ++nAns;
            perturbWL = HPWL();

            double deltaCost = perturbWL - originWL;
            if (deltaCost <= 0/*  or accept(deltaCost, T, originWL) */)
            {
                if (deltaCost > 0)
                    ++uphill;

                originWL = perturbWL;
                if (originWL < bestWL)
                {
                    bestInstances = instances;
                    bestWL = originWL;
                }
            }
            else
            {
                ++reject;
                Swap(oldtype, oldr2, oldr1, true);
            }
        }

        T *= reduceRatio;
        if (T % 100 == 0)
            cout << "T: " << T << ", bestWL: " << bestWL << endl;
    } while (/* reject / nAns <= 0.95 and */ T > 10);
    cout << "end SA" << endl;
}

int main(int argc, char *argv[])
{
    srand(1);
    Resource.resize(3); // 3: Resource[CLB], Resource[RAM], Resource[DSP]
    Arg arg(argc, argv);
    archParser(arg.archPath);
    instParser(arg.instPath);
    netParser(arg.netPath);

    // checkParsers(instances, nets, Resource);
    cout << "Given input's HPWL: " << HPWL() << endl;
    initPlace();
    cout << "After initPlace HPWL: " << HPWL() << endl;

    // cout << Resource[0][i].stored << endl;
    SA();

    updateResourceToBest(bestInstances, Resource);
    if (checkValid(bestInstances, Resource))
        cout << "Successfully !" << endl;
    else
        cout << "Failed to make placement legal... " << endl;

    output(arg.outPath, bestInstances, Resource);
}
