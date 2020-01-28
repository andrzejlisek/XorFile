#ifndef XORACTION_H
#define XORACTION_H

#include "Eden.h"
#include <string>
#include <fstream>

#define PacketToText(X) (((X) >= 0) ? (((X) < 10) ? ("D0" + to_string(X)) : ("D" + to_string(X))) : (((X) == -1) ? "P" : (((X) == -2) ? "Q" : "?")))

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

    void TextDataSet();
};

#endif // XORACTION_H
