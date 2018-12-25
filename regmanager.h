#ifndef REGMANAGER_H
#define REGMANAGER_H

#include <set>

#include "node.h"

using namespace std;

class RegManager{
    private:
        int memSize;
    public:
        RegManager();
        int getMemSize();
        void addIdToReg(IdNode* id, int reg);
        void initPara(vector<IdNode*> params);
};
#endif