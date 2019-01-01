#include "manager.h"

#include "node.h"
#include "regmap.h"

using namespace std;

/* ------------------ Manager ------------------ */
Manager::Manager(){
    stackSize = 0;
}
Manager::Manager(const Manager& other){
    idTable = other.idTable；
    regTable = other.regTable;
}
int Manager::getStackSize(){
    return stackSize;
}

// return NULL if not found
IdNode* Manager::findId(IdNode* id){
    auto it = idTable.find(id);
    if(it==idTable.end()) // not found
        return NULL;
    else
        return *it;
}
IdNode* Manager::addId(IdNode* id){
    // auto insert
    idTable[id];
}
// 将params添加到idTable，并load到对应reg
void Manager::setParams(vector<IdNode*> params){
    for(int i = 0;i<params.size();i++){
        addId(params[i]);
        int locate = i + paraRegStart;
        loadIdToReg(params[i], locate);
    }
}

// 清除reg中别的id,更新别的id的AddrSet
// 将id load到目标reg，更新id的AddrSet
// 返回处理代码
string Manager::loadIdToReg(IdNode* id, int reg){
    cleanReg(reg);
    addIdToReg(id, reg);
    return genLoadCode(id, reg);
}
// 清除reg中的id并清除idTable中活跃于reg的id的地址描述符
string Manager::cleanReg(int reg){
    set<IdNode*> idInReg = regTable[reg];
    string cleanCode = string();
    for(auto& id:idInReg){
        bool doubleCheck = false;
        AddrDescriptor mark;
        for(auto& addr:idTable[id]){
            if(addr.getType()==RegType&&addr.getValue()==reg){
                doubleCheck = true;
                mark = addr;
                break;
            }
        }
        if(doubleCheck==false) assert(false);
        idTable[id].erase(mark);
    }
    regTable[reg].clear();
}
// 将id添加到reg,增加reg到id的addrSet,增加id到reg的idset
void Manager::addIdToReg(IdNode* id, int reg){
    idTable[id].insert(reg);
    regTable[reg].insert(id);
}
// 返回将id load到reg时对应的代码
string Manager::genLoadCode(IdNode* id, int reg){
    string code, load, idName, regName;
    idName = id->getTiggerName();
    regName = getRegName(reg);
    if(id->isArray())
        load = "loadaddr";
    else
        load = "load";

    if(id->isGlobal()){ // 全局变量
        code = load + " " + idName + " " + regName + "\n";
    }
    else if(id->isLocal()){ // 局部变量 
        // 查看是否在栈中
        // 若不在说明是函数刚开始的参数，或者第一次被定值的局部变量(?)
        // 不需要显式产生代码
        if(!hasAllocatedStack(id)){ // 没有栈描述符
            code = "";
        }
        else{ //在栈中
            idName = to_string(stackInfo[id].getValue());
            code = load + " " + idName + " " + regName + "\n";
        }
    }
    else { // 整数变量
        assert(id->isInteger());
        code = regName + " = " + to_string(id->getValue()) + "\n";
    }
    return code;
}

string Manager:getRegName(int reg){
    return regMap[reg];
}

bool Manager::hasAllocatedStack(IdNode* id){
    if(id->isGlobal()||stackInfo.find(id)==stackInfo.end())
        return false;
    else
        return true;
}

string Manager::storeIdToMem(IdNode* id){
    int fromReg;
    fromReg = findInWhichReg(id);
    assert(fromReg!=-1); //一定在某个寄存器中
    addMemToId(id); // 修改id地址描述符使包含内存位置
    return genStoreCode(id, fromReg);
}

// 返回当前存储id的寄存器编号
// 如果有多个返回第一个
int Manager::findInWhichReg(IdNode* id){
    // 找到id所在的reg
    int fromReg = -1;
    for(auto& addr:idTable[id]){
        if(addr.getType()==RegDescriptor){
            fromReg = addr.getValue();
            break;
        }
    }
    return fromReg;
}

// 修改id地址描述符使包含内存位置
void Manager::addMemToId(IdNode* id){
    if(id->isGlobal()){
        idTable[id].insert(GlobalDescriptor());
    }
    else if(id->isLocal()){
        if(!hasAllocatedStack(id))
            allocateStack(id);
        idTable[id].insert(stackInfo[id]);
    }
}

// 给id分配栈空间
void Manager::allocateStack(IdNode* id){
    stackInfo[id] = StackDescriptor(stackSize);
    stackSize++;
}

// 返回将fromReg处的id存到内存位置时候的代码
string Manager::genStoreCode(IdNode* id, int fromReg){
    string code = string();
    if(id->isGlobal()){
        code += "loadaddr " + id->getTiggerName() + " " + getRegName(tmpReg) + "\n";
        code += getRegName(tmpReg) + "[0] = " + getRegName(fromReg) + "\n"; 
    }
    else if(id->isLocal()){
        int locate = stackInfo[id].getValue();
        code += "store " + getRegName(fromReg) + " " + to_string(locate) + "\n";
    }
    return code;
}


// 使reg只包含id
// 使id只包含reg
// 更新其它活跃于reg的id的AddrSet
void Manager::makeIdMonopolizeReg(IdNode* id, int reg){
    cleanReg(reg);
    cleanId(id);
    addIdToReg(id, reg);
}

// 清除id中的所有活跃reg并清除所有regTable中的id
void Manager::cleanId(IdNode* id){
    for(auto& addr:idTable[id]){
        if(addr.getType()==RegDescriptor){
            int reg = addr.getValue();
            inRegRange(reg);
            regTable[reg].erase(id);
        }
    }
    idTable[id].clear();
}

// 检查reg下标是否越界, 否则报错
void inRegRange(int index){
    if(index < 0 || index >= regNum) assert(false);
}

// 将id加入reg
// 使得id只包含reg
void makeIdShareReg(IdNode* id, int reg){
    cleanId(id);
    addIdToReg(id, reg);
}


string Manager::genCode(ExprNode* expr){
    string code;
    switch(expr->getExprType()){
        case Op2Type:
            // x = y + z
            IdNode* y = expr->getRightValue1();
            IdNode* z = expr->getRightValue2();
            int y_locate;
            y_locate = findInWhichReg(y);
            if(y_locate==-1){

            }

        case Op1Type:
        case NoOpType:
        case StoreArrayType:
        case VisitArrayType:
        case IfBranchType:
        case GotoType:
        case LabelType:
        case CallType:
        case ReturnType:
        case LocalDeclareType:
    }
}

int Manager::pickReg(){
    for(int reg = 0;reg < regNum; reg++){
        if(reg==tmpReg) reg++;

        if(regTable[reg].empty())
            return reg;        
    }
    for(int reg = 0;reg < regNum; reg++){
        if(reg==tmpReg) reg++;

        set<IdNode*> idSet = regTable[reg];

    }
}

/* ------------------ AddrDescriptor ------------------ */
AddrDescriptor::AddrDescriptor(AddrDescriptorType type_, int value_ = -1){
    value = value_;
    descriptorType = type_;
}
AddrDescriptorType AddrDescriptor::getType(){
    return descriptorType;
}
int AddrDescriptor::getValue(){
    return value;
}

/* ------------------ RegDescriptor ------------------ */
RegDescriptor::RegDescriptor(int value_):AddrDescriptor(RegType, value_){};
bool RegDescriptor::operator==(const RegDescriptor& other){
    return value==other.getValue();
}
/* ------------------ GlobalDescriptor ------------------ */
GlobalDescriptor::GlobalDescriptor():AddrDescriptor(GlobalType){}

/* ------------------ StackDescriptor ------------------ */
StackDescriptor::StackDescriptor(int value_):AddrDescriptor(StackType, value_){}

