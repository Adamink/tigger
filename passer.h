#ifndef PASSER_H
#define PASSER_H

#include "node.h"

class Passer{
    public:
    ExprType exprType;
    IdNode* var, *rightValue1, *rightValue2;
    string label, op;
    set<IdNode*> paras; 
    /*string varRegName, rightValue1RegName, rightValue2RegName, tmpRegName;
    string code;*/
    Passer();
    Passer(ExprType exprType_, IdNode* var_, IdNode* rightValue1_, IdNode* rightValue2_, string op);
    Passer(ExprType exprType_, IdNode* var_, set<IdNode*> paras_);
    Passer(ExprTYpe exprType_, IdNode* rightValue1_, IdNode* rightValue2_, label_);
    void swap();
    private:
};
#endif