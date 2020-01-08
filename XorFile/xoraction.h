#ifndef XORACTION_H
#define XORACTION_H

#include "Eden.h"
#include <string>
#include <fstream>

using namespace std;
class XorAction
{
public:
    /*
    0:Label
    1:Create X = D
    2:X = X ^ M(A, D)
    3:X = X ^ M(A, D) ^ M(B, D)
    4:X = XM(A, X)
    5:X = UM(A, X)
    6:X = XM(A, UM(B, X))
    */
    int ActionType;


    int ActionStatus;
    XorAction(int _ActionType, string _LabelText, int _ParamD, int _ParamA, int _ParamB, int _OutData);
    string GetText();
    string LabelText;
    int ParamA;
    int ParamB;
    int ParamD;
    string OutVarName;
    string InVarName;
    int OutData;
    string MSG;

    void TextDataSave(vector<string> &Buf);
    void TextDataLoad(vector<string> &Buf, uint &BufI);
};

#endif // XORACTION_H
