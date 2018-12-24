#ifndef IDMANAGER_H
#define IDMANAGER_H

#include <set>

#include "node.h"

using namespace std;

class IdManager{
    private:
        set<IdNode*> idSet; 
    public:
        IdManager();
        IdNode* addId(IdNode* );
        IdNode* findId(IdNode* );
};
#endif