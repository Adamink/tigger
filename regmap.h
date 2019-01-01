#ifndef REGMAP_H
#define REGMAP_H

#include <set>
#include <map>
#include <string>

using namespace std;
const int regNum = 27;
const int paraRegStart = 0;
const int paraRegEnd = 7;
const int zeroReg =  26;
const int tmpReg = 13;
const map<int, string> regMap{
{0, "a0"},
{1, "a1"},
{2, "a2"},
{3, "a3"},
{4, "a4"},
{5, "a5"},
{6, "a6"},
{7, "t0"},
{8, "t1"},
{9, "t2"},
{10, "t3"},
{11, "t4"},
{12, "t5"},
{13, "t6"},
{14, "s0"},
{15, "s1"},
{16, "s2"},
{17, "s3"},
{18, "s4"},
{19, "s5"},
{20, "s6"},
{21, "s7"},
{22, "s8"},
{23, "s9"},
{24, "s10"},
{25, "s11"},
{26, "x0"}
};

#endif