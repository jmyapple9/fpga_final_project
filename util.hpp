#pragma once
#include "classes.hpp"

void checkParsers(vector<Instance> &instances, vector<Net> &nets, vector<vector<Slot>>& Resource)
{
    cout << "================== checking resources ==================\n";
    for (auto re : Resource[CLB])
        cout << re.Rid << " " << re.x << " " << re.y << endl;
    for (auto re : Resource[RAM])
        cout << re.Rid << " " << re.x << " " << re.y << endl;
    for (auto re : Resource[DSP])
        cout << re.Rid << " " << re.x << " " << re.y << endl;
    cout << "\ntotal # of resources: " << Resource[CLB].size() + Resource[RAM].size() + Resource[DSP].size() << endl;

    cout << "================== checking instances ==================\n";
    for (auto inst : instances)
        cout << inst.name << " " << inst.type << " " << inst.x << " " << inst.y << endl;
    cout << "\ntotal # of non-IO instances: " << instances.size() << endl;

    cout << "================== checking nets ==================\n";
    for (auto net : nets)
    {
        cout << net.name;
        for (auto Iid : net.insts)
            cout << " " << instances[Iid].name;
        cout << endl;
    }
    cout << "\ntotal # of net: " << nets.size() << endl;
}

void output(string outputPath, vector<Instance> &instances)
{
    ofstream outputfile;
    outputfile.open(outputPath);
    for (auto nIO : instances)
    {
        outputfile << nIO.name << " "
                   << "RESOURCE" << nIO.rsrc + 1 << endl;
    }

    outputfile.flush();
    outputfile.close();
}


bool checkValid(vector<Instance> & instances, vector<vector<Slot>> &Resource)
{
    cout << "Checking if the answer is valid..." << endl;

    for (auto nIO : instances)
    {
        if (nIO.type == 0)
        {
            if (nIO.rsrc < (int)Resource[CLB].size() and nIO.rsrc >= 0)
            {
                if (Resource[CLB][nIO.rsrc].stored != nIO.Iid)
                {
                    cout << "Invalid! Slot stored " << Resource[CLB][nIO.rsrc].stored << " and instance Iid " << nIO.Iid << "miss match" << endl;
                    return false;
                }
            }
            else
            {
                cout << "Invalid! Slot id in instance " << nIO.name << ": " << nIO.rsrc << endl;
                return false;
            }
        }
        else if (nIO.type == 1)
        {
            if (nIO.rsrc < (int)Resource[RAM].size() and nIO.rsrc >= 0)
            {
                if (Resource[RAM][nIO.rsrc].stored != nIO.Iid)
                {
                    cout << "Invalid! Slot stored idx " << Resource[RAM][nIO.rsrc].stored << " and instance Iid " << nIO.Iid << "miss match" << endl;
                    return false;
                }
            }
            else
            {
                cout << "Invalid! Slot id in instance " << nIO.name << " is wrong: " << nIO.rsrc << endl;
                return false;
            }
        }
        else if (nIO.type == 2)
        {
            if (nIO.rsrc < (int)Resource[DSP].size() and nIO.rsrc >= 0)
            {
                if (Resource[DSP][nIO.rsrc].stored != nIO.Iid)
                {
                    cout << "Invalid! Slot stored idx " << Resource[DSP][nIO.rsrc].stored << " and instance Iid " << nIO.Iid << "miss match" << endl;
                    return false;
                }
            }
            else
            {
                cout << "Invalid! Slot id in instance " << nIO.name << " is wrong: " << nIO.rsrc << endl;
                return false;
            }
        }
        else
        {
            cout << "Invalid! Instance " << nIO.name << " has invalid type number: " << nIO.type << endl;
            return false;
        }
    }
    return true;
}


void updateResourceToBest(vector<Instance> & bestInstances, vector<vector<Slot>> &Resource)
{
    for (auto &nIO : bestInstances)
    {
        if (nIO.type == 0)
            Resource[CLB][nIO.rsrc].stored = nIO.Iid;
        else if (nIO.type == 1)
            Resource[RAM][nIO.rsrc].stored = nIO.Iid;
        else
            Resource[DSP][nIO.rsrc].stored = nIO.Iid;
    }
}