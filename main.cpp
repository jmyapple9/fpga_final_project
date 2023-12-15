#include "classes.hpp"

ifstream in_file;

vector<Resource> Rclb, Rram, Rdsp;
vector<Instance> instances;
vector<Net> nets;
unordered_map<string, int> inst_name_id;

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
    int Rid = 0;
    while (getline(in_file, line))
    {
        istringstream iss(line);
        iss >> name >> typeStr >> x >> y;

        if (typeStr == "CLB")
            Rclb.emplace_back(Resource(Rid++, x, y));
        else if (typeStr == "RAM")
            Rram.emplace_back(Resource(Rid++, x, y));
        else if (typeStr == "DSP")
            Rdsp.emplace_back(Resource(Rid++, x, y));
        else
            cout << "Error! There's no type \"" << typeStr << "\"\n";
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
            net.insts.emplace_back(inst_name_id[instName]);
        }
        nets.emplace_back(net);
    }

    in_file.close();
}

void check()
{
    cout << "================== checking resources ==================\n";
    for (auto re : Rclb)
        cout << re.Rid << " " << re.x << " " << re.y << endl;
    for (auto re : Rram)
        cout << re.Rid << " " << re.x << " " << re.y << endl;
    for (auto re : Rdsp)
        cout << re.Rid << " " << re.x << " " << re.y << endl;
    cout << "\ntotal # of resources: " << Rclb.size() + Rram.size() + Rdsp.size() << endl;


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

void output(string outputPath)
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

int main(int argc, char *argv[])
{
    Arg arg(argc, argv);
    archParser(arg.archPath);
    instParser(arg.instPath);
    netParser(arg.netPath);

    // check();

    output(arg.outPath);
}
