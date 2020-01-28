#include "xoraction.h"

XorAction::XorAction(int _ActionType, string _LabelText, int _ParamD, int _ParamA, int _ParamB, int _OutData)
{
    MSG = "";
    ActionStatus = 0;
    ActionType = _ActionType;
    LabelText = _LabelText;
    ParamD = _ParamD;
    ParamA = _ParamA;
    ParamB = _ParamB;
    OutData = _OutData;


    if (OutData >= -2)
    {
        OutVarName = PacketToText(OutData);
    }
    else
    {
        OutData = -99;
        OutVarName = "?";
    }
    if ((ActionType == 4) || (ActionType == 5))
    {
        ParamD = -3;
    }
}

///
/// \brief XorAction::GetText - Get the action text description
/// \return
///
string XorAction::GetText()
{
    string Temp;
    if (ParamD >= -2)
    {
        InVarName = PacketToText(ParamD);
    }
    else
    {
        InVarName = "0";
    }
    if (ActionType == 0)
    {
        Temp = "\t";
    }
    else
    {
        switch (ActionStatus)
        {
            case -1:
                Temp = MSG + "\t";
                break;
            case 0:
                Temp = "Waiting\t";
                break;
            case 1:
                Temp = "Done\t";
                break;
            case 2:
                Temp = "Postponed\t";
                break;
            case 3:
                Temp = "Working\t";
                break;
            default:
                Temp = "Unknown\t";
                break;
        }
    }
    switch (ActionType)
    {
        case 0:
            Temp += LabelText;
            break;
        case 1:
            Temp += "Create " + OutVarName + " = Mangle(" + to_string(ParamA) + ", " + InVarName + ")";
            break;
        case 2:
            Temp += "Set " + OutVarName + " = " + OutVarName + " ^ Mangle(" + to_string(ParamA) + ", " + InVarName + ")";
            break;
        case 3:
            Temp += "Set " + OutVarName + " = " + OutVarName + " ^ Mangle(" + to_string(ParamA) + ", " + InVarName + ") ^ Mangle(" + to_string(ParamB) + ", " + InVarName + ")";
            break;
        case 4:
            Temp += "Set " + OutVarName + " = XMangle(" + to_string(ParamA) + ", " + OutVarName + ")";
            break;
        case 5:
            Temp += "Set " + OutVarName + " = Unmangle(" + to_string(ParamA) + ", " + OutVarName + ")";
            break;
        case 6:
            Temp += "Set " + OutVarName + " = XMangle(" + to_string(ParamA) + ", Unmangle(" + to_string(ParamB) + ", " + OutVarName + "))";
            break;
        default:
            break;
    }
    return Temp;
}

///
/// \brief XorAction::TextDataSet - Set some data after loading from text
///
void XorAction::TextDataSet()
{
    MSG = "";
    if (OutData >= -2)
    {
        OutVarName = PacketToText(OutData);
    }
    else
    {
        OutData = -99;
        OutVarName = "?";
    }
    if ((ActionType == 4) || (ActionType == 5))
    {
        ParamD = -3;
    }
}
