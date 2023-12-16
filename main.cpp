// #include "classes.hpp"
#include "util.hpp"

ifstream in_file;
vector<vector<Slot>> Resource;
vector<Slot> clb, ram, dsp;
vector<Instance> instances, bestInstances;
vector<Net> nets;
unordered_map<string, int> inst_name_id;

double bestWL{0.0}, perturbWL{0.0}, originWL{0.0};

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
        if (type != 3)
        {
            instances.emplace_back(Instance(Iid++, type, x, y, name));
        }
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

    auto start_time = chrono::system_clock::now();
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
    cout << "hpwl after initPlace: " << hpwl << endl;
    auto realDuration = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start_time);
    cout << "duration: " << realDuration.count() << " seconds." << endl;
    return hpwl;
}

void initPlace()
{
    cout << "Enter initPlace" << endl;
    int clbIdx{0}, ramIdx{0}, dspIdx{0};
    for (auto &nIO : instances)
    {
        if (nIO.type == 0)
        {
            nIO.rsrc = clbIdx;
            Resource[CLB][clbIdx++].stored = nIO.Iid;
        }
        else if (nIO.type == 1)
        {
            nIO.rsrc = ramIdx;
            Resource[RAM][ramIdx++].stored = nIO.Iid;
        }
        else
        {
            nIO.rsrc = dspIdx;
            Resource[DSP][dspIdx++].stored = nIO.Iid;
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

/* return: delta cost (newWL - oldWL) */
double perturb()
{
    // global swap (CLB only)
    vector<float> X_coords, Y_coords;
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
                X_coords.emplace_back(instances[nbr].x);
                Y_coords.emplace_back(instances[nbr].y);
            }
        }
    }

    sort(X_coords.begin(), X_coords.end());
    sort(Y_coords.begin(), Y_coords.end());

    nth_element(X_coords.begin(), X_coords.begin() + (X_coords.size() / 2), X_coords.end());
    nth_element(Y_coords.begin(), Y_coords.begin() + (Y_coords.size() / 2), Y_coords.end());
    size_t
        xmid1 = X_coords.size() / 2,
        xmid2 = (X_coords.size() < 2) ? (xmid1) : (xmid1 + 1),
        ymid1 = Y_coords.size() / 2,
        ymid2 = (Y_coords.size() < 2) ? (ymid1) : (ymid1 + 1);
    float
        X_mid1{X_coords[xmid1]},
        X_mid2{X_coords[xmid2]},
        Y_mid1{X_coords[ymid1]},
        Y_mid2{X_coords[ymid2]};

    // getting the resource in optimal region by Binary search, check if the resource is occupied or not
    

    // another perturb: swap with other resource slot
}

void revert()
{
}

void SA()
{
    double reject, reduceRatio = 0.9999;
    int nAns, uphill, T = 1000000, N = 70; // N: number of answer in T

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
            if (deltaCost <= 0 or accept(deltaCost, T, originWL / 600000))
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
                revert();
            }
        }

        T *= reduceRatio;
        if (T % 10000 == 0)
            cout << "T: " << T << ", bestWL: " << bestWL << endl;
    } while (reject / nAns <= 0.95 and T > 10);
}

int main(int argc, char *argv[])
{
    Resource.resize(3); // 3: Resource[CLB], Resource[RAM], Resource[DSP]
    Arg arg(argc, argv);
    archParser(arg.archPath);
    instParser(arg.instPath);
    netParser(arg.netPath);

    // checkParsers(instances, nets, Resource);

    initPlace();

    // SA();

    updateResourceToBest(bestInstances, Resource);
    if (checkValid(instances, Resource))
        cout << "Successfully !" << endl;
    else
        cout << "Failed to make placement legal... " << endl;

    output(arg.outPath, instances);
}
