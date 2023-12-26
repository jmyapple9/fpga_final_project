// #include "classes.hpp"
#include "util.hpp"

ifstream in_file;
vector<vector<Slot>> Resource;
vector<Slot> clb, ram, dsp;
vector<Instance> instances, bestInstances;
vector<Net> nets;
unordered_map<string, int> inst_name_id;

double bestHPWL{DBL_MAX}, perturbWL{0.0}, originWL{0.0};
pair<int, int> prev_swap;
int oldtype, oldr1, oldr2;

chrono::system_clock::time_point start_time;
chrono::system_clock::time_point end_time;

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
    // TODO:
    // 1. build a net vector for each instance(module) so that we can calculate HPWL faster!
    // 2.

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
            double x{instances[instId].x}, y{instances[instId].y};
            xmax = max(xmax, x);
            xmin = min(xmin, x);
            ymax = max(ymax, y);
            ymin = min(ymin, y);
        }
        hpwl += ((xmax - xmin) + (ymax - ymin));
    }
    // cout << "end HPWL" << endl;
    return hpwl;
}

void initPlace2()
{
    for (auto &inst : instances)
    {
        int type = inst.type;
        int bestSlotID = -1;
        int slotID = 0;
        float shortestDist = numeric_limits<float>::max();
        if (inst.type == 3)
            continue;
        for (auto res : Resource[type])
        {
            if (res.stored == -1)
            {

                float dist = abs(res.x - inst.x) + abs(res.y - inst.y);
                if (dist < shortestDist)
                {
                    shortestDist = dist;
                    bestSlotID = slotID;
                }
            }
            slotID++;
            // unsigned SlotID = distance(Resource[type].begin(), can);
        }
        inst.rsrc = bestSlotID;
        inst.x = Resource[type][bestSlotID].x;
        inst.y = Resource[type][bestSlotID].y;
        Resource[type][bestSlotID].stored = inst.Iid;
    }
}

void initPlace()
{
    // cout << "Enter initPlace" << endl;

    for (auto &inst : instances)
    {
        vector<vector<Slot>::iterator> candidate;
        int type = inst.type;
        if (type == 3)
            continue;
        auto x_low = lower_bound(Resource[type].begin(), Resource[type].end(), Slot(0, inst.x - 40, 0), compareSlotByX);
        auto x_high = lower_bound(Resource[type].begin(), Resource[type].end(), Slot(0, inst.x + 40, 0), compareSlotByX);

        while (x_low++ != x_high)
            candidate.push_back(x_low);

        if (candidate.size() == 0)
            cout << "No candidate!! " << inst.type << ", " << inst.x << ", " << inst.y << endl;

        int bestSlotID = -1;
        float shortestDist = numeric_limits<float>::max();
        for (auto can : candidate)
        {
            float dist = abs((*can).x - inst.x) + abs((*can).y - inst.y);
            unsigned SlotID = distance(Resource[type].begin(), can);
            if (dist < shortestDist and Resource[type][SlotID].stored == -1)
            {
                shortestDist = dist;
                bestSlotID = SlotID;
            }
        }
        inst.rsrc = bestSlotID;
        inst.x = Resource[type][bestSlotID].x;
        inst.y = Resource[type][bestSlotID].y;
        Resource[type][bestSlotID].stored = inst.Iid;
    }
    // cout << "exit initPlace" << endl;

    // originWL = HPWL();
}


bool accept(double cost, int T)
{
    double random = static_cast<double>(rand()) / RAND_MAX;
    bool ac = exp(-cost * 100 / T) > random;
    // cout << (ac ? "accept" : "notAccept") << ": " << exp(-cost*100 / T )  << endl;
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

double localHPWL(Instance &inst)
{
    double local = 0;
    for (int netID : inst.net)
    {
        double
            xmax{numeric_limits<double>::lowest()},
            xmin{numeric_limits<double>::max()},
            ymax{numeric_limits<double>::lowest()},
            ymin{numeric_limits<double>::max()};
        for (auto instId : nets[netID].insts)
        {
            double x{instances[instId].x}, y{instances[instId].y};
            xmax = max(xmax, x);
            xmin = min(xmin, x);
            ymax = max(ymax, y);
            ymin = min(ymin, y);
        }
        local += ((xmax - xmin) + (ymax - ymin));
    }
    return local;
}
double Swap(int type, int r1, int r2)
{
    Slot &slot1 = Resource[type][r1];
    Slot &slot2 = Resource[type][r2];
    Instance &inst1 = instances[slot1.stored];

    if (slot1.stored == -1)
        cout << "Error! slot1 should never be empty" << endl;

    // double localHpwl = 0;

    if (slot2.stored == -1)
    {
        double beforeSwap = localHPWL(inst1);

        slot2.stored = inst1.Iid;
        slot1.stored = -1;

        inst1.rsrc = r2;
        inst1.x = slot2.x;
        inst1.y = slot2.y;
        return localHPWL(inst1) - beforeSwap;
    }
    else
    {
        Instance &inst2 = instances[slot2.stored];
        if (inst1.type != inst2.type)
            cout << "Error! try swapping instances in different type!" << endl;

        double beforeSwap = localHPWL(inst1) + localHPWL(inst2);


        slot1.stored = inst2.Iid;
        slot2.stored = inst1.Iid;

        inst1.x = slot2.x;
        inst1.y = slot2.y;
        inst1.rsrc = r2;

        inst2.x = slot1.x;
        inst2.y = slot1.y;
        inst2.rsrc = r1;
        return localHPWL(inst1) + localHPWL(inst2) - beforeSwap;
    }
    // cout << "exit swap" << endl;
    // if(slot2.stored == -1)
    //     return {inst1.Iid, -1};
    // else
    //     return {inst1.Iid, instances[slot2.stored].Iid};
}

double perturb()
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
    return Swap(randInst.type, randInst.rsrc, ResID);
}

void SA()
{
    // cout << "Enter SA()" << endl;
    double reject, reduceRatio = 0.9999;
    int nAns, uphill, T = 10000000, N = 100; // N: number of answer in T
    bestHPWL = originWL;
    do
    {
        reject = nAns = uphill = 0.;

        if (chrono::system_clock::now() >= end_time)
            return;

        while (uphill < N and nAns < 2 * N)
        {
            // int act = rand() % 4;
            // perturb();
            ++nAns;
            // perturbWL = HPWL();

            // double deltaCost = perturbWL - originWL;
            double deltaCost = perturb();

            if (deltaCost <= 0)
            // if (deltaCost <= 0  or accept(deltaCost, T))
            {
                if (deltaCost > 0)
                    ++uphill;

                originWL = originWL + deltaCost;
                if (originWL < bestHPWL)
                {
                    bestInstances = instances;
                    bestHPWL = originWL;
                }
            }
            else
            {
                ++reject;
                Swap(oldtype, oldr2, oldr1);
            }
        }

        T *= reduceRatio;
        if (T % 100 == 0)
            cout << "T: " << T << ", bestHPWL: " << bestHPWL << endl;
    } while (/* reject / nAns <= 0.95 and */ T > 10);
    // cout << "end SA" << endl;
}

int main(int argc, char *argv[])
{
    chrono::duration<int, std::ratio<60>> minutes(9);
    chrono::duration<int> seconds(50);
    start_time = chrono::system_clock::now();
    end_time = start_time + minutes + seconds;

    srand(1);
    Resource.resize(3); // 3: Resource[CLB], Resource[RAM], Resource[DSP]
    Arg arg(argc, argv);
    archParser(arg.archPath);
    instParser(arg.instPath);
    netParser(arg.netPath);

    // checkParsers(instances, nets, Resource);
    double rawHPWL = -1.0, initHPWL = -1.0;
    rawHPWL = HPWL();
    initPlace();
    // initPlace2(); // return the same initial placement as initPlace
    // randomInitPlace();
    bestHPWL = initHPWL = originWL = HPWL();
    bestInstances = instances;

    SA();

    updateResourceToBest(bestInstances, Resource);
    instances = bestInstances;
    if (checkValid(bestInstances, Resource))
        cout << "Successfully !" << endl;
    else
        cout << "Failed to make placement legal... " << endl;

    auto realDuration = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start_time);
    cout << "duration: " << realDuration.count() << " seconds." << endl;

    cout << "Given input's HPWL: " << setprecision(10) << rawHPWL << endl;
    cout << "After initPlace HPWL: " << setprecision(10) << initHPWL << endl;
    cout << "Best HPWL after legalization: " << setprecision(10) << bestHPWL << ", which has " << setprecision(5)
         << ((10000 * ((rawHPWL - bestHPWL) / rawHPWL)) / 100) << "%"
         << " imporved than Given input's HPWL" << endl;

    output(arg.outPath, bestInstances, Resource);
}
