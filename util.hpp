#pragma once
#include "classes.hpp"

void checkParsers(vector<Instance> &instances, vector<Net> &nets, vector<vector<Slot>> &Resource)
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
    size_t nIO_inst = 0;
    for (auto inst : instances)
        if (inst.type != 3)
        {
            ++nIO_inst;
            cout << inst.name << " " << inst.type << " " << inst.x << " " << inst.y << endl;
        }
    cout << "\ntotal # of non-IO instances: " << nIO_inst << endl;

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

void output(string outputPath, vector<Instance> &instances, vector<vector<Slot>> &Resource)
{
    ofstream outputfile;
    outputfile.open(outputPath);
    for (auto inst : instances)
    {
        if (inst.type != 3)
        {
            outputfile << inst.name << " RESOURCE" << Resource[inst.type][inst.rsrc].Rid + 1 << endl;
        }
    }
    outputfile.flush();
    outputfile.close();
}

bool checkValid(vector<Instance> &instances, vector<vector<Slot>> &Resource)
{
    cout << "Checking if the answer is valid...";
    // cout << "instances.size(): "<< instances.size() << endl;
    for (auto inst : instances)
    {
        if (inst.type == 0)
        {
            if (inst.rsrc < (int)Resource[CLB].size() and inst.rsrc >= 0)
            {
                if (Resource[CLB][inst.rsrc].stored != inst.Iid)
                {
                    cout << "Invalid! Slot stored " << Resource[CLB][inst.rsrc].stored << " and instance Iid " << inst.Iid << " miss match" << endl;
                    return false;
                }
            }
            else
            {
                cout << "Invalid! Slot id in instance " << inst.name << ": " << inst.rsrc << endl;
                return false;
            }
        }
        else if (inst.type == 1)
        {
            if (inst.rsrc < (int)Resource[RAM].size() and inst.rsrc >= 0)
            {
                if (Resource[RAM][inst.rsrc].stored != inst.Iid)
                {
                    cout << "Invalid! Slot stored idx " << Resource[RAM][inst.rsrc].stored << " and instance Iid " << inst.Iid << " miss match" << endl;
                    return false;
                }
            }
            else
            {
                cout << "Invalid! Slot id in instance " << inst.name << " is wrong: " << inst.rsrc << endl;
                return false;
            }
        }
        else if (inst.type == 2)
        {
            if (inst.rsrc < (int)Resource[DSP].size() and inst.rsrc >= 0)
            {
                if (Resource[DSP][inst.rsrc].stored != inst.Iid)
                {
                    cout << "Invalid! Slot stored idx " << Resource[DSP][inst.rsrc].stored << " and instance Iid " << inst.Iid << " miss match" << endl;
                    return false;
                }
            }
            else
            {
                cout << "Invalid! Slot id in instance " << inst.name << " is wrong: " << inst.rsrc << endl;
                return false;
            }
        }
        else if(inst.type == 3) ; // skip checking IO
        else
        {
            cout << "Invalid! Instance " << inst.name << " has invalid type number: " << inst.type << endl;
            return false;
        }
    }
    return true;
}

void updateResourceToBest(vector<Instance> &bestInstances, vector<vector<Slot>> &Resource)
{
    for (auto &inst : bestInstances)
    {
        if (inst.type == 0)
            Resource[CLB][inst.rsrc].stored = inst.Iid;
        else if (inst.type == 1)
            Resource[RAM][inst.rsrc].stored = inst.Iid;
        else if(inst.type==2)
            Resource[DSP][inst.rsrc].stored = inst.Iid;
    }
}

bool compareSlotByX(const Slot &slot1, const Slot &slot2) { return slot1.x < slot2.x; }
