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
// 设置regToSearch
void Manager::setParams(vector<IdNode*> params){
    for(int i = 0;i<params.size();i++){
        addId(params[i]);
        int locate = i + paraRegStart;
        loadIdToReg(params[i], locate);
    }
    calcRegToSearch(params.size());
}
// 得到要搜索的reg集合，去掉tmpReg,zeroReg和paraReg
void Manager::calcRegToSearch(int paraNum){
    for(int reg = 0;reg < regNum; reg++){
        if(reg==tmpReg) continue;
        if(reg>=paraRegStart&&reg<paraRegStart + paraNum) continue; 
        if(reg==zeroReg) continue;
        regToSearch.insert(reg);     
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

// 保存reg中的id,要求id在aliveVarSet中
string Manager::saveIdInReg(int reg){
    string code = string();
    for(auto& id:regTable[reg]){
        if(expr->isAlive(id)){
            code += storeIdToMem(id);
        }
    }
    return code;
}

string Manager::genCode(ExprNode* expr_){
    string code;
    expr = expr_
    switch(expr->getExprType()){
        case Op2Type:
            // x = y op z
            IdNode* x = expr->getVar();
            IdNode* y = expr->getRightValue1();
            IdNode* z = expr->getRightValue2();
            bool ifInteger = swapIfInteger(y,z);
            string op = expr->getOp();
            bool couldAbbr = ifInteger && (op == "+" || op == "<");
            int y_locate = findInWhichReg(y);
            if(y_locate==-1){
                y_locate = pickReg();
                code += saveIdInReg(y_locate);
                code += loadIdToReg(y, y_locate);
            }
            int z_locate = findInWhichReg(z);
            if(z_locate==-1){
                if(couldAbbr) break;
                if(ifInteger&&z->getValue()==0){
                    z_locate = zeroReg;
                    makeIdShareReg(z, z_locate);
                    break;
                }
                banRegToSearch(y_locate);
                z_locate = pickReg();
                restoreRegToSearch(y_locate);
                code += saveIdInReg(z_locate);
                code += loadIdToReg(z, z_locate);
            }
            int x_locate = findInWhichReg(x);
            if(x_locate==-1){
                // y的寄存器可以复用
                if(!expr->isAliveNext(y)){
                    x_locate = y_locate;
                    // if x==y, also ok
                    makeIdShareReg(x, x_locate);
                    break;
                }
                // z在寄存器中且可以复用
                if(z_locate!=-1&&!expr->isAliveNext(z)){
                    x_locate = z_locate;
                    makeIdShareReg(x, x_locate);
                    break;
                }
                // 给x分配寄存器
                banRegToSearch(y_locate, z_locate);
                x_locate = pickReg();
                restoreRegToSearch(y_locate, z_locate);
                code += saveIdInReg(x_locate);
                code += loadIdToReg(x, x_locate);
            }
            if(couldAbbr)
                code += regMap[x_locate] + " = " + regMap[y_locate] + " "
                 + op + " " + to_string(z->getValue()) + "\n";
            else    
                code += regMap[x_locate] + " = " + regMap[y_locate] + " "
                 + op + " " + regMap[z_locate] + "\n";

        break;
        case Op1Type:
            // x = op y, op = ! or -
            IdNode* x = expr->getVar();
            IdNode* y = expr->getRightValue();
            string op = expr->getOp();
            int y_locate = findInWhichReg(y);
            if(y_locate==-1){
                if(y->isInteger()&&y->getValue()==0){
                    y_locate = zeroReg;
                    makeIdShareReg(y, zeroReg);
                    break;
                }
                y_locate = pickReg();
                code += saveIdInReg(y_locate);
                code += loadIdToReg(y, y_locate);
            }
            int x_locate = findInWhichReg(x);
            if(x_locate==-1){
                // y的寄存器可以复用
                if(!expr->isAliveNext(y)){
                    x_locate = y_locate;
                    makeIdShareReg(x, x_locate);
                    break;
                }
                // 给x分配寄存器
                banRegToSearch(y_locate);
                x_locate = pickReg();
                restoreRegToSearch(y_locate);
                code += saveIdInReg(x_locate);
                code += loadIdToReg(x, x_locate);
            }
            code += regMap[x_locate] + " = " + op + " " + regMap[y_locate] + "\n";
        break;
        case NoOpType:
            // x = y
            IdNode* x = expr->getVar();
            IdNode* y = expr->getRightValue();
            int y_locate = findInWhichReg(y);
            if(y_locate==-1){
                if(y->isInteger()&&y->getValue()==0){
                    y_locate = zeroReg;
                    makeIdShareReg(y, zeroReg);
                    break;
                }
                y_locate = pickReg();
                code += saveIdInReg(y_locate);
                code += loadIdToReg(y, y_locate);
            }
            makeIdShareReg(x, y_locate);
        break;
        case StoreArrayType:
            // x [y] = z, y could be var, then let t = x + y
            // t[0] = z
            IdNode* x = expr->getVar();
            IdNode* y = expr->getRightValue1();
            IdNode* z = expr->getRightValue2();
            if(y->isInteger()){
                int x_locate = findInWhichReg(x);
                if(x_locate==-1){
                    x_locate = pickReg();
                    code += saveInReg(x_locate);
                    code += loadIdToReg(x, x_locate);
                }
                int z_locate = findInWhichReg(z);
                if(z_locate==-1){
                    if(z->isInteger()&&z->getValue()==0){
                        z_locate = zeroReg;
                        makeIdShareReg(z, zeroReg);
                        break;
                    }
                    banRegToSearch(x_locate);
                    z_locate = pickReg();
                    restoreRegToSearch(x_locate);
                    code += saveInReg(z_locate);
                    code += loadIdToReg(z, z_locate);
                }
                code += regMap[x_locate] + "[" + to_string(y->getValue()) + "] = "
                 + regMap[z_locate] + "\n";
            }
            else{
                int x_locate = findInWhichReg(x);
                if(x_locate==-1){
                    x_locate = pickReg();
                    code += saveInReg(x_locate);
                    code += loadIdToReg(x, x_locate);
                }
                int y_locate = findInWhichReg(y);
                if(y_locate==-1){
                    
                }
            }
        break;
        case VisitArrayType:
        break;
        case IfBranchType:
        break;
        case GotoType:
        break;
        case LabelType:
        break;
        case CallType:
        break;
        case ReturnType:
        break;
        case LocalDeclareType:
        break;
    }
}
// 如果yz中有整数返回true
// 并将顺序调整为整数在后
bool swapIfInteger(IdNode*& y, IdNode*& z){
    if(y->isInteger){
        IdNode* tmp = y;
        y = z;
        z= tmp;
        return true;
    }
    if(z->isInteger)
        return true;
    return false;
}
int Manager::pickReg(){
    set<IdNode*> aliveVarSet = expr->getAliveVarSet();
    int minCntAliveLocate;
    int minCntAlive = 100;
    for(auto& reg:regToSearch){
        // 优先找空reg
        if(regTable[reg].empty())
            return reg;
        // 再找reg里存的都是不活跃变量的reg 
        set<IdNode*> idSet = regTable[reg];
        int cntAlive = 0;
        for(auto& id:idSet){
            if(isAlive(id, aliveVarSet))
                cntAlive++;
        }
        // 如果没有活跃变量则该寄存器可用
        if(cntAlive==0)
            return reg;
        // 否则找出开销最小的寄存器
        if(cntAlive < minCntAlive){
            minCntAlive = cntAlive;
            minCntAliveLocate = reg;
        }
    }

    // 还没有找到寄存器，找cntAlive最小的溢出
    return minCntAliveLocate;
}
void Manager::banRegToSearch(int reg1, int reg2 = -1){
    regToSearch.erase(reg1);
    regToSearch.erase(reg2);
}
void Manager::restoreRegToSearch(int reg1, int reg2 = -1){
    if(reg1!=-1)
        regToSearch.insert(reg1);
    if(reg2!=-1)
        regToSearch.insert(reg2);
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

