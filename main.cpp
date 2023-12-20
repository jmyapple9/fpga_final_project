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

    cout << "Enter HPWL" << endl;
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
    cout << "end HPWL" << endl;
    return hpwl;
}

void initPlace()
{
    cout << "Enter initPlace" << endl;
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
    bool ac = exp(-cost / baselineWL / T) > random;
    // cout << (ac ? "accept" : "notAccept") << endl;

    return ac;
}

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
void Swap(int type, int r1, int r2)
{
    // cout << "enter swap" << endl;
    Slot &slot1 = Resource[type][r1];
    Slot &slot2 = Resource[type][r2];
    Instance &inst1 = instances[slot1.stored];
    if (slot2.stored == -1)
    {
        cout << "swap 1" << endl;
        slot2.stored = inst1.Iid;
        slot1.stored = -1;

        inst1.rsrc = slot2.Rid;
        inst1.x = slot2.x;
        inst1.y = slot2.y;
    }
    else
    {
        cout << "swap 2" << endl;
        Instance &inst2 = instances[slot2.stored];
        // Instance tmp1 = instances[slot1.stored];
        if (inst1.type != inst2.type)
            cout << "Error! try swapping instances in different type!" << endl;

        slot1.stored = inst1.Iid;
        slot2.stored = inst2.Iid;

        inst1.x = slot2.x;
        inst1.y = slot2.y;
        inst1.rsrc = slot2.Rid;

        inst2.x = slot1.x;
        inst2.y = slot1.y;
        inst2.rsrc = slot1.Rid;
    }
    // cout << "exit swap" << endl;
    /* Instance &inst1 = instances[instId1];
    Instance &inst2 = instances[instId2];
    Instance tmp1 = instances[instId1];
    if (inst1.type != inst2.type)
        cout << "Error! try swapping instances in different type!" << endl;

    inst1.x = inst2.x;
    inst1.y = inst2.y;
    inst1.rsrc = inst2.rsrc;
    inst2.x = tmp1.x;
    inst2.y = tmp1.y;
    inst2.rsrc = tmp1.rsrc;

    Resource[inst1.type][inst1.rsrc].stored = instId1;
    Resource[inst2.type][inst2.rsrc].stored = instId2; */
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
    Swap(randInst.type, randInst.rsrc, ResID);

    int cnt = 0;
    for (size_t i = 0; i < Resource[0].size(); i++)
        if(Resource[0][i].stored != -1) cnt++;
    cout << cnt << " clb in total" << endl;
    /* if (slot.stored == -1)
    {
        Resource[randInst.type][randInst.rsrc].stored = -1;
        slot.stored = instID;
        randInst.rsrc = slot.Rid;
        randInst.x = slot.x;
        randInst.y = slot.y;
    }
    else
    {
        Resource[randInst.type][randInst.rsrc].stored = slot.stored;
        slot.stored = instID;
        randInst.rsrc = slot.Rid;
        randInst.x = slot.x;
        randInst.y = slot.y;
    } */

    // cout << "end perturb" << endl;
    /*
    // global swap (CLB only)
    vector<int> X_coords, Y_coords;
    // vector<float> X_coords, Y_coords;
    int Iid = rand() % instances.size();
    Instance &inst = instances[Iid];

    // extract neighbors' x, y coords; get origin hpwl in
    for (int nId : inst.net)
    {
        vector<int> &neighborInst = nets[nId].insts;
        for (int nbr : neighborInst)
        {
            if (instances[nbr].type == 0)
            {
                X_coords.emplace_back(nbr);
                Y_coords.emplace_back(nbr);
            }
        }
    }

    sort(X_coords.begin(), X_coords.end(), ascendingX());
    sort(Y_coords.begin(), Y_coords.end(), ascendingY());

    // nth_element(X_coords.begin(), X_coords.begin() + (X_coords.size() / 2), X_coords.end());
    // nth_element(Y_coords.begin(), Y_coords.begin() + (Y_coords.size() / 2), Y_coords.end());
    int range = 1;
    size_t
        xmid1 = min(X_coords.size() / 2 + range, X_coords.size() - 1),
        xmid2 = max(X_coords.size() / 2 - range, 0UL),
        ymid1 = min(Y_coords.size() / 2 + range, Y_coords.size() - 1),
        ymid2 = max(Y_coords.size() / 2 - range, 0UL);
    float
        X_mid1{instances[X_coords[xmid1]].x},
        X_mid2{instances[X_coords[xmid2]].x},
        Y_mid1{instances[X_coords[ymid1]].y},
        Y_mid2{instances[X_coords[ymid2]].y};
    // cout << "X_mid1: " << X_mid1 << ", X_mid2: " << X_mid2 << ", Y_mid1: " << Y_mid1 << ", Y_mid2: " << Y_mid2 << endl;

    int bestOne = -1;
    double current_best = DBL_MAX;
     for (auto &clbSlot : Resource[CLB])
    {
        if(X_mid1 <= clbSlot.x)
    }
    for (auto clbID : X_coords)
    {
        Instance& clb = instances[clbID];
        if ((X_mid1 <= clb.x and clb.x <= X_mid2) or
            (Y_mid1 <= clb.y and clb.y <= Y_mid2))
        {
            double oldHPWL = HPWL();
            Swap(clb.Iid, inst.Iid);
            double newHPWL = HPWL();
            if (newHPWL < oldHPWL and newHPWL < current_best){
                bestOne = clb.Iid;
                current_best = newHPWL;
            }
            Swap(clb.Iid, inst.Iid);
        }
    }
    if (bestOne == -1)
    {
        cout << "fail to do global swap..." << endl;
    }
    else
    {
        cout << "global swap" << endl;
        Swap(bestOne, inst.Iid);
    } */
    // another perturb: swap with other resource slot
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
            // if (!checkValid(instances, Resource))
            // {
            //     break;
            // }
            perturb();
            ++nAns;
            perturbWL = HPWL();

            // cout << "originWL: " << originWL <<endl;
            // cout << "perturbWL: " << perturbWL<<endl;
            double deltaCost = perturbWL - originWL;
            if (deltaCost <= 0 /* or accept(deltaCost, T, originWL / 600000) */)
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
                // revert();
                Swap(oldtype, oldr1, oldr2);
            }
        }

        T *= reduceRatio;
        // if (T % 10000 == 0)
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

    initPlace();
    
    // cout << Resource[0][i].stored << endl;
    SA();

    updateResourceToBest(bestInstances, Resource);
    if (checkValid(bestInstances, Resource))
        cout << "Successfully !" << endl;
    else
        cout << "Failed to make placement legal... " << endl;

    output(arg.outPath, bestInstances, Resource);
}
