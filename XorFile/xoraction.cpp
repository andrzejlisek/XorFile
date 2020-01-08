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


    if (OutData >= 0)
    {
        OutVarName = "D" + to_string(OutData);
    }
    else
    {
        switch (OutData)
        {
        case -1:
            OutVarName = "P";
            break;
        case -2:
            OutVarName = "Q";
            break;
        default:
            OutData = -99;
            OutVarName = "?";
            break;
        }
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
    InVarName = "D" + to_string(ParamD);
    if (ParamD == (-1)) { InVarName = "P"; }
    if (ParamD == (-2)) { InVarName = "Q"; }
    if (ParamD == (-3)) { InVarName = "0"; }
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
/// \brief XorAction::TextDataSave - Save action data to file
/// \param FS
///
void XorAction::TextDataSave(vector<string> &Buf)
{
    Buf.push_back(to_string(ActionType));
    Buf.push_back(to_string(ActionStatus));
    Buf.push_back(to_string(ParamA));
    Buf.push_back(to_string(ParamB));
    Buf.push_back(to_string(ParamD));
    Buf.push_back(to_string(OutData));
    Buf.push_back(LabelText);
}

///
/// \brief XorAction::TextDataLoad - Load action data to file
/// \param FS
///
void XorAction::TextDataLoad(vector<string> &Buf, uint &BufI)
{
    MSG = "";
    ActionType = Eden::ToInt(Buf[BufI]);
    BufI++;
    ActionStatus = Eden::ToInt(Buf[BufI]);
    BufI++;
    ParamA = Eden::ToInt(Buf[BufI]);
    BufI++;
    ParamB = Eden::ToInt(Buf[BufI]);
    BufI++;
    ParamD = Eden::ToInt(Buf[BufI]);
    BufI++;
    OutData = Eden::ToInt(Buf[BufI]);
    BufI++;
    LabelText = Buf[BufI];
    BufI++;

    if (OutData >= 0)
    {
        OutVarName = "D" + to_string(OutData);
    }
    else
    {
        switch (OutData)
        {
        case -1:
            OutVarName = "P";
            break;
        case -2:
            OutVarName = "Q";
            break;
        default:
            OutData = -99;
            OutVarName = "?";
            break;
        }
    }
    if ((ActionType == 4) || (ActionType == 5))
    {
        ParamD = -3;
    }
}
