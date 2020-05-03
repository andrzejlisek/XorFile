#include "appcore.h"


AppCore::AppCore()
{
    ActionWorking = -1;
    ActionProgressCurrent = 0;
    ActionProgressCount = 0;

    IntegrityWorking = -1;
    IntegrityProgressCurrent = 0;
    IntegrityProgressCount = 0;

    // The size of packet part, which is read or written by once operation
    DefaultPacketChunkSize = 1024 * 1024 * 64;
    IntegrityPacketChunkSize = 1024 * 1024 * 64;

    IntegrityTempFileName = Eden::ToStr(QDir::temp().absoluteFilePath("xor.tmp"));

    uint I, II, III, IIII;
    uchar TempMangle;

    uint TempData1;
    uint TempData2;

    DataN = 0;

    // Filling in the mangle array
    // Mangle(x1,x2,x3,x4) = (x1+x4,x1,x2,x3)
    // 0000 -> 0000
    // 0001 -> 1000 -> 1100 -> 1110
    // 1110 -> 1111 -> 0111 -> 1011
    // 1011 -> 0101 -> 1010 -> 1101
    // 1101 -> 0110 -> 0011 -> 1001
    // 1001 -> 0100 -> 0010 -> 0001
    for (I = 0; I < 256; I++)
    {
        TempMangle = 0;
        TempMangle += ((I & (b10000000)) ^ ((I & (b00010000)) << 3));
        TempMangle += ((I & (b10000000)) >> 1);
        TempMangle += ((I & (b01000000)) >> 1);
        TempMangle += ((I & (b00100000)) >> 1);
        TempMangle += ((I & (b00001000)) ^ ((I & (b00000001)) << 3));
        TempMangle += ((I & (b00001000)) >> 1);
        TempMangle += ((I & (b00000100)) >> 1);
        TempMangle += ((I & (b00000010)) >> 1);
        Mangle[1][I] = TempMangle;
        Unmangle[1][TempMangle] = I;
    }

    // Extending the mangle array for all iterations
    for (I = 0; I < 256; I++)
    {
        Mangle[0][I] = I;
        Unmangle[0][I] = I;
        for (II = 2; II < 16; II++)
        {
            Mangle[II][I] = Mangle[1][Mangle[II - 1][I]];
            Unmangle[II][I] = Unmangle[1][Unmangle[II - 1][I]];
        }
    }


    // Creating the XMangle and XUnmangle functions by every X
    // XUnmangle(X) = X ^ Mangle(X)
    // X = XMangle(X ^ Mangle(X))
    // XMangle(X) equals to the some multiply iterated called Mangle(X), needed to found the iteration count
    for (I = 0; I < 16; I++)
    {
        XIteration[I] = 0;
        for (II = 0; II < 16; II++)
        {
            int Err0 = 0;
            for (III = 1; III < 16; III++)
            {
                TempData1 = III;
                for (IIII = 1; IIII <= I; IIII++)
                {
                    TempData1 = Mangle[1][TempData1];
                }
                TempData1 = TempData1 ^ III;


                TempData2 = III;
                for (IIII = 1; IIII <= II; IIII++)
                {
                    TempData2 = Unmangle[1][TempData2];
                }

                IIII = II;
                if (TempData2 != TempData1)
                {
                    Err0 = 1;
                }
            }
            if (Err0 == 0)
            {
                XIteration[I] = IIII;
            }
        }
    }


    // Filling in the multiply XMangle array
    for (I = 0; I < 16; I++)
    {
        for (II = 0; II < 256; II++)
        {
            TempMangle = II;
            for (III = 1; III <= XIteration[I]; III++)
            {
                TempMangle = Mangle[1][TempMangle];
            }
            XMangle[I][II] = TempMangle;
            XUnmangle[I][TempMangle] = II;
        }
    }


    // Clearing action list
    ActionList.clear();

    // Clearing the packet bunch
    for (I = 0; I < 15; I++)
    {
        FileD[I] = "";
        StatusD[I] = 0;
        DirD[I] = "";
        SizeD[I] = -3;
    }
    FileP = "";
    FileQ = "";
    StatusP = 0;
    StatusQ = 0;
    DirP = "";
    DirQ = "";
    SizeP = -3;
    SizeQ = -3;
}

void AppCore::GenerateScenarioRecoverD(int DataNum, int Recalc, int Reduce)
{
    int I;
    int MangleNum = (Recalc > Reduce) ? (Recalc - Reduce) : (Reduce - Recalc);
    int AdditionalUnmangle = 0;
    if ((Reduce > MangleNum) && (Reduce >= Recalc))
    {
        AdditionalUnmangle = (Reduce - MangleNum);
    }
    if ((Recalc > MangleNum) && (Recalc >= Reduce))
    {
        AdditionalUnmangle = (Recalc - MangleNum);
    }



    ActionList.push_back(XorAction(0, "Recover " + PacketToText(Recalc) + " using P, Q and all remaining Dxx excepting " + PacketToText(Reduce), 0, 0, 0, Recalc));
    ActionList.push_back(XorAction(1, "", -1, Reduce, 0, Recalc));
    ActionList.push_back(XorAction(2, "", -2, 0, 0, Recalc));

    for (I = 0; I < DataNum; I++)
    {
        if ((I != Recalc) && (I != Reduce))
        {
            ActionList.push_back(XorAction(3, "", I, Reduce, I, Recalc));
        }
    }


    ActionList.push_back(XorAction(6, "", 0, MangleNum, AdditionalUnmangle, Recalc));
}

int AppCore::GenerateScenario()
{
    int Missing = 0;
    int Missing1 = -1;
    int Missing2 = -1;
    int AddRemove = 0;
    int I;

    // Detecting if the P and Q are defined
    bool ExistsP = true;
    bool ExistsQ = true;
    if (GetPacketFile(-1).compare("") == 0)
    {
        ExistsP = false;
    }
    if (GetPacketFile(-2).compare("") == 0)
    {
        ExistsQ = false;
    }


    // Adding or removing P or Q packet is not possible
    if ((StatusP == 2) || (StatusQ == 2))
    {
        return 5;
    }

    // Data recovery without both P and Q is not possible
    if ((!ExistsP) && (!ExistsQ))
    {
        return 3;
    }

    if (ExistsP && (StatusP == 1)) { Missing++; }
    if (ExistsQ && (StatusQ == 1)) { Missing++; }
    for (I = 0; I < DataN; I++)
    {
        if (StatusD[I] == 1)
        {
            Missing++;
            if (Missing1 < 0)
            {
                Missing1 = I;
            }
            else
            {
                Missing2 = I;
            }
        }
    }

    // Data recovery or add/remove packet in one scenario is not possible
    for (I = 0; I < DataN; I++)
    {
        if ((StatusD[I] == 2))
        {
            AddRemove++;
            if (Missing > 0)
            {
                return 6;
            }
        }
    }


    // Clearing action list
    ActionList.clear();


    if ((Missing == 0) && (AddRemove == 0))
    {
        return 2;
    }

    if (Missing > 2)
    {
        return 1;
    }

    // Recovering two data packets needs both P and Q packets
    if (Missing == 2)
    {
        if ((!ExistsP) || (!ExistsQ))
        {
            return 4;
        }
    }

    // Recovering two data packets
    if ((Missing1 >= 0) && (Missing2 >= 0))
    {
        GenerateScenarioRecoverD(DataN, Missing1, Missing2);
        GenerateScenarioRecoverD(DataN, Missing2, Missing1);
    }


    // Recovering one data packet
    if ((Missing1 >= 0) && (Missing2 < 0))
    {
        if ((StatusP == 0) && ExistsP)
        {
            if (ExistsP)
            {
                ActionList.push_back(XorAction(0, "Recover " + PacketToText(Missing1) + " using P and all remaining Dxx", 0, 0, 0, Missing1));
                ActionList.push_back(XorAction(1, "", -1, 0, 0, Missing1));
                for (I = 0; I < DataN; I++)
                {
                    if (I != Missing1)
                    {
                        ActionList.push_back(XorAction(2, "", I, 0, 0, Missing1));
                    }
                }
            }
        }
        else
        {
            if (ExistsQ)
            {
                ActionList.push_back(XorAction(0, "Recover " + PacketToText(Missing1) + " using Q and all remaining Dxx", 0, 0, 0, Missing1));
                ActionList.push_back(XorAction(1, "", -2, 0, 0, Missing1));
                for (I = 0; I < DataN; I++)
                {
                    if (I != Missing1)
                    {
                        ActionList.push_back(XorAction(2, "", I, I, 0, Missing1));
                    }
                }
                if (Missing1 > 0)
                {
                    ActionList.push_back(XorAction(5, "", 0, Missing1, 0, Missing1));
                }
            }
        }
    }


    // Regenerating the P packet
    if ((StatusP == 1) && ExistsP)
    {
        ActionList.push_back(XorAction(0, "Generate P", 0, 0, 0, -1));
        ActionList.push_back(XorAction(1, "", -3, 0, 0, -1));
        for (I = 0; I < DataN; I++)
        {
            ActionList.push_back(XorAction(2, "", I, 0, 0, -1));
        }
    }


    // Regenerating the Q packet
    if ((StatusQ == 1) && ExistsQ)
    {
        ActionList.push_back(XorAction(0, "Generate Q", 0, 0, 0, -2));
        ActionList.push_back(XorAction(1, "", -3, 0, 0, -2));
        for (I = 0; I < DataN; I++)
        {
            ActionList.push_back(XorAction(2, "", I, I, 0, -2));
        }
    }

    // Adding or removing data packets
    if (AddRemove > 0)
    {
        for (I = 0; I < DataN; I++)
        {
            if ((StatusD[I] == 2))
            {
                ActionList.push_back(XorAction(0, "Add or remove " + PacketToText(I), 0, 0, 0, I));
                if (ExistsP)
                {
                    ActionList.push_back(XorAction(2, "" + to_string(I), I, 0, 0, -1));
                }
                if (ExistsQ)
                {
                    ActionList.push_back(XorAction(2, "" + to_string(I), I, I, 0, -2));
                }
            }
        }
    }

    return 0;
}

///
/// \brief AppCore::GetPacketNum - Aquiring the packet number based on data packet count
/// \param N
/// \return
///
int AppCore::GetPacketNum(int N)
{
    if (N < DataN)
    {
        return N;
    }
    if (N == DataN)
    {
        return -1;
    }
    if (N == (DataN + 1))
    {
        return -2;
    }
    return -3;
}

///
/// \brief AppCore::GetPacketText - Aquiring the text description for the packet
/// \param N
/// \return
///
string AppCore::GetPacketText(int N, bool OnlySize)
{
    N = GetPacketNum(N);
    stringstream SS;
    llong PacketSize = -3;
    SS << PacketToText(N) << "\t";
    if (N >= 0)
    {
        if ((FileD[N].compare("") != 0))
        {
            switch (StatusD[N])
            {
                case 0: SS << "Exists"; break;
                case 1: SS << "Missing"; break;
                case 2: SS << "Add/Remove"; break;
            }
            PacketSize = SizeD[N];
        }
        else
        {
            SS << "Not defined";
            PacketSize = -2;
        }
        SS << "\t" << FileD[N] << "\t" << DirD[N] << "\t";
    }
    if (N == (-1))
    {
        if ((FileP.compare("") != 0))
        {
            switch (StatusP)
            {
                case 0: SS << "Exists"; break;
                case 1: SS << "Missing"; break;
                case 2: SS << "Add/Remove"; break;
            }
            PacketSize = SizeP;
        }
        else
        {
            SS << "Not exists";
            PacketSize = -2;
        }
        SS << "\t" << FileP << "\t" << DirP << "\t";
    }
    if (N == (-2))
    {
        if ((FileQ.compare("") != 0))
        {
            switch (StatusQ)
            {
                case 0: SS << "Exists"; break;
                case 1: SS << "Missing"; break;
                case 2: SS << "Add/Remove"; break;
            }
            PacketSize = SizeQ;
        }
        else
        {
            SS << "Not exists";
            PacketSize = -2;
        }
        SS << "\t" << FileQ << "\t" << DirQ << "\t";
    }
    if (OnlySize)
    {
        SS.str("");
    }
    if (PacketSize >= 0)
    {
        SS << PacketSize;
    }
    else
    {
        if (PacketSize == -1)
        {
            SS << "Not available";
        }
        if (PacketSize == -3)
        {
            SS << "Unknown";
        }
    }
    return SS.str();
}

///
/// \brief AppCore::SetPacketStatus - Setting the packet status
/// \param N
/// \param PacketStatus
///
void AppCore::SetPacketStatus(int N, int PacketStatus)
{
    N = GetPacketNum(N);
    if (N >= 0)
    {
        StatusD[N] = PacketStatus;
    }
    if (N == (-1))
    {
        StatusP = PacketStatus;
    }
    if (N == (-2))
    {
        StatusQ = PacketStatus;
    }
}

///
/// \brief AppCore::GetPacketStatus - Getting the packet status
/// \param N
/// \return
///
int AppCore::GetPacketStatus(int N)
{
    N = GetPacketNum(N);
    if (N >= 0)
    {
        return StatusD[N];
    }
    if (N == (-1))
    {
        return StatusP;
    }
    if (N == (-2))
    {
        return StatusQ;
    }
    return 0;
}

///
/// \brief AppCore::SetPacketFile - Setting the packet index file
/// \param N
/// \param FileName
///
void AppCore::SetPacketFile(int N, string FileName)
{
    N = GetPacketNum(N);
    if (N >= 0)
    {
        FileD[N] = FileName;
    }
    if (N == (-1))
    {
        FileP = FileName;
    }
    if (N == (-2))
    {
        FileQ = FileName;
    }
}

///
/// \brief AppCore::GetPacketFile - Getting the packet index file
/// \param N
/// \return
///
string AppCore::GetPacketFile(int N)
{
    N = GetPacketNum(N);
    if (N >= 0)
    {
        return FileD[N];
    }
    if (N == (-1))
    {
        return FileP;
    }
    if (N == (-2))
    {
        return FileQ;
    }
    return "";
}

///
/// \brief AppCore::SetPacketDir - Setting the packet base directory
/// \param N
/// \param DirName
///
void AppCore::SetPacketDir(int N, string DirName)
{
    N = GetPacketNum(N);
    if (N >= 0)
    {
        DirD[N] = DirName;
    }
    if (N == (-1))
    {
        DirP = DirName;
    }
    if (N == (-2))
    {
        DirQ = DirName;
    }
}

///
/// \brief AppCore::GetPacketDir - Getting the packet base directory
/// \param N
/// \return
///
string AppCore::GetPacketDir(int N)
{
    N = GetPacketNum(N);
    if (N >= 0)
    {
        return DirD[N];
    }
    if (N == (-1))
    {
        return DirP;
    }
    if (N == (-2))
    {
        return DirQ;
    }
    return "";
}

///
/// \brief AppCore::ReadPacketDir - Getting the base directory from path to the packet index file
/// \param N
/// \return
///
string AppCore::ReadPacketDir(int N)
{
    FilePacket FP;
    FP.LoadIndex(GetPacketFile(N));
    SetPacketDir(N, FP.WorkDir);
    return FP.WorkDir;
}

///
/// \brief AppCore::SetPacketQuan - Setting the data packet quantity
/// \param N
///
void AppCore::SetPacketQuan(int N)
{
    DataN = N;
    int I;
    for (I = N; I < 15; I++)
    {
        FileD[I] = "";
        StatusD[I] = 0;
        DirD[I] = "";
        SizeD[I] = 0;
    }
}

///
/// \brief AppCore::GetPacketQuan - Getting the data packet quantity
/// \return
///
int AppCore::GetPacketQuan()
{
    return DataN;
}

///
/// \brief AppCore::ProjectNew - Creating new bunch file
///
void AppCore::ProjectNew()
{
    uint BufI = 0;
    int I;

    // Quantity of data packets
    DataN = 1;
    BufI++;

    // Data packets
    for (I = 0; I < DataN; I++)
    {
        StatusD[I] = 0;
        BufI++;
        FileD[I] = "";
        BufI++;
        DirD[I] = "";
        BufI++;
        SizeD[I] = -3;
    }

    // P and Q packets
    StatusP = 0;
    BufI++;
    FileP = "";
    BufI++;
    DirP = "";
    BufI++;
    SizeP = -3;
    StatusQ = 0;
    BufI++;
    FileQ = "";
    BufI++;
    DirQ = "";
    BufI++;
    SizeQ = -3;

    // Clearing action list
    ActionList.clear();

    // Data range
    WorkRange1 = 0;
    WorkRange2 = 0;

    // Clearing the integrity test
    IntegrityRangeBegin.clear();
    IntegrityRangeEnd.clear();
    IntegrityRangeStatusP.clear();
    IntegrityRangeStatusQ.clear();
    IntegrityActionStatus.clear();
    IntegrityActionType.clear();
    IntegrityTempFileName = "";
}


///
/// \brief AppCore::ProjectLoad - Loading bunch file
/// \param FileName
///
void AppCore::ProjectLoad(string FileName, string FilePath)
{
    EdenClass::ConfigFile CF;
    CF.FileLoad(FileName);
    int I;

    // Quantity of data packets
    DataN = CF.ParamGetI("Data");

    // Data packets
    for (I = 0; I < DataN; I++)
    {
        StatusD[I] = CF.ParamGetI("Data" + to_string(I) + "Status");
        FileD[I] = EdenClass::FileDir::DirRelativeToAbsolute(FilePath, CF.ParamGetS("Data" + to_string(I) + "File"));
        DirD[I] = CF.ParamGetS("Data" + to_string(I) + "Dir");
        SizeD[I] = -3;
    }

    // P and Q packets
    StatusP = CF.ParamGetI("ParityPStatus");
    FileP = EdenClass::FileDir::DirRelativeToAbsolute(FilePath, CF.ParamGetS("ParityPFile"));
    DirP = CF.ParamGetS("ParityPDir");
    SizeP = -3;
    StatusQ = CF.ParamGetI("ParityQStatus");
    FileQ = EdenClass::FileDir::DirRelativeToAbsolute(FilePath, CF.ParamGetS("ParityQFile"));
    DirQ = CF.ParamGetS("ParityQDir");
    SizeQ = -3;

    // Clearing action list
    ActionList.clear();

    // Quantity of actions
    int TempN = CF.ParamGetI("Action");

    // Action list
    for (I = 0; I < TempN; I++)
    {
        ActionList.push_back(XorAction(0, "", 0, 0, 0, 0));
        ActionList[I].ActionType = CF.ParamGetI("Action" + to_string(I) + "Type");
        ActionList[I].ActionStatus = CF.ParamGetI("Action" + to_string(I) + "Status");
        ActionList[I].ParamA = CF.ParamGetI("Action" + to_string(I) + "ParamA");
        ActionList[I].ParamB = CF.ParamGetI("Action" + to_string(I) + "ParamB");
        ActionList[I].ParamD = CF.ParamGetI("Action" + to_string(I) + "ParamD");
        ActionList[I].OutData = CF.ParamGetI("Action" + to_string(I) + "OutData");
        ActionList[I].LabelText = CF.ParamGetS("Action" + to_string(I) + "LabelText");
        ActionList[I].TextDataSet();
    }

    // Data range
    WorkRange1 = CF.ParamGetL("WorkRange1");
    WorkRange2 = CF.ParamGetL("WorkRange2");

    // Clearing the integrity test
    IntegrityRangeBegin.clear();
    IntegrityRangeEnd.clear();
    IntegrityRangeStatusP.clear();
    IntegrityRangeStatusQ.clear();
    IntegrityActionStatus.clear();
    IntegrityActionType.clear();

    // Temporary file name
    IntegrityTempFileName = CF.ParamGetS("IntegrityTempFileName");

    // Quantity of test data stripes
    TempN = CF.ParamGetI("IntegrityRange");

    // Test data stripes
    for (I = 0; I < TempN; I++)
    {
        IntegrityRangeBegin.push_back(CF.ParamGetL("IntegrityRange" + to_string(I) + "Begin"));
        IntegrityRangeEnd.push_back(CF.ParamGetL("IntegrityRange" + to_string(I) + "End"));
        IntegrityRangeStatusP.push_back(CF.ParamGetI("IntegrityRange" + to_string(I) + "StatusP"));
        IntegrityRangeStatusQ.push_back(CF.ParamGetI("IntegrityRange" + to_string(I) + "StatusQ"));
    }

    // Quantity of test actions
    TempN = CF.ParamGetI("IntegrityAction");

    // Test action list
    for (I = 0; I < TempN; I++)
    {
        IntegrityActionStatus.push_back(CF.ParamGetI("IntegrityAction" + to_string(I) + "Status"));
        IntegrityActionType.push_back(CF.ParamGetI("IntegrityAction" + to_string(I) + "Type"));
    }
}


///
/// \brief AppCore::ProjectSave - Saving the bunch
/// \param FileName
///
void AppCore::ProjectSave(string FileName, string FilePath)
{
    EdenClass::ConfigFile CF;
    int I;

    // Quantity of data packets
    CF.ParamSet("Data", DataN);

    // Data packets
    for (I = 0; I < DataN; I++)
    {
        CF.ParamSet("Data" + to_string(I) + "Status", StatusD[I]);
        CF.ParamSet("Data" + to_string(I) + "File", EdenClass::FileDir::DirAbsoluteToRelative(FilePath, FileD[I]));
        CF.ParamSet("Data" + to_string(I) + "Dir", DirD[I]);
    }

    // P and Q packets
    CF.ParamSet("ParityPStatus", StatusP);
    CF.ParamSet("ParityPFile", EdenClass::FileDir::DirAbsoluteToRelative(FilePath, FileP));
    CF.ParamSet("ParityPDir", DirP);
    CF.ParamSet("ParityQStatus", StatusQ);
    CF.ParamSet("ParityQFile", EdenClass::FileDir::DirAbsoluteToRelative(FilePath, FileQ));
    CF.ParamSet("ParityQDir", DirQ);

    // Quantity of actions
    int TempN = ActionList.size();
    CF.ParamSet("Action", TempN);

    // Action list
    for (I = 0; I < TempN; I++)
    {
        CF.ParamSet("Action" + to_string(I) + "Type", ActionList[I].ActionType);
        CF.ParamSet("Action" + to_string(I) + "Status", ActionList[I].ActionStatus);
        CF.ParamSet("Action" + to_string(I) + "ParamA", ActionList[I].ParamA);
        CF.ParamSet("Action" + to_string(I) + "ParamB", ActionList[I].ParamB);
        CF.ParamSet("Action" + to_string(I) + "ParamD", ActionList[I].ParamD);
        CF.ParamSet("Action" + to_string(I) + "OutData", ActionList[I].OutData);
        CF.ParamSet("Action" + to_string(I) + "LabelText", ActionList[I].LabelText);
    }

    // Data range
    CF.ParamSet("WorkRange1", WorkRange1);
    CF.ParamSet("WorkRange2", WorkRange2);

    // Temporary file name
    CF.ParamSet("IntegrityTempFileName", IntegrityTempFileName);

    // Quantity of test data stripes
    TempN = IntegrityRangeBegin.size();
    CF.ParamSet("IntegrityRange", TempN);

    // Test data stripes
    for (I = 0; I < TempN; I++)
    {
        CF.ParamSet("IntegrityRange" + to_string(I) + "Begin", IntegrityRangeBegin[I]);
        CF.ParamSet("IntegrityRange" + to_string(I) + "End", IntegrityRangeEnd[I]);
        CF.ParamSet("IntegrityRange" + to_string(I) + "StatusP", IntegrityRangeStatusP[I]);
        CF.ParamSet("IntegrityRange" + to_string(I) + "StatusQ", IntegrityRangeStatusQ[I]);
    }

    // Quantity of test actions
    TempN = IntegrityActionStatus.size();

    // Test action list
    CF.ParamSet("IntegrityAction", TempN);
    for (I = 0; I < TempN; I++)
    {
        CF.ParamSet("IntegrityAction" + to_string(I) + "Status", IntegrityActionStatus[I]);
        CF.ParamSet("IntegrityAction" + to_string(I) + "Type", IntegrityActionType[I]);
    }

    CF.FileSave(FileName);
}

///
/// \brief AppCore::ActionSetStatus - Setting the action status
/// \param N
/// \param S
///
void AppCore::ActionSetStatus(uint N, int S)
{
    if (N < ActionList.size())
    {
        ActionList[N].ActionStatus = S;
    }
}

///
/// \brief AppCore::WorkRangeSet - Setting the data range
/// \param R1
/// \param R2
///
void AppCore::WorkRangeSet(llong R1, llong R2)
{
    WorkRange1 = R1;
    WorkRange2 = R2;
}

///
/// \brief AppCore::WorkRangeGet - Getting the data range
/// \param R
/// \return
///
llong AppCore::WorkRangeGet(int R)
{
    if (R == 1)
    {
        return WorkRange1;
    }
    if (R == 2)
    {
        return WorkRange2;
    }
    return -1;
}

///
/// \brief AppCore::AcrionPerformWaiting - Performing all waiting actions
///
void AppCore::ActionPerformWaiting()
{
    ActionWorking = 99999999;
    thread Thr(&AppCore::ActionPerformWaitingWork, this);
    Thr.detach();
}

///
/// \brief AppCore::ActionPerformWaitingWork - Performing all waiting actions - working method
///
void AppCore::ActionPerformWaitingWork()
{
    uint I;
    for (I = 0; I < ActionList.size(); I++)
    {
        if (!ActionPerform(I))
        {
            I = ActionList.size();
        }
    }
    ActionWorking = -1;
}

///
/// \brief AppCore::ActionPerform - Performing one action
/// \param N
/// \return
///
bool AppCore::ActionPerform(uint N)
{
    bool Good = true;

    if ((WorkRange2 <= WorkRange1) || (WorkRange1 < 0))
    {
        return false;
    }

    llong * ProgressC = new llong();
    llong * ProgressM = new llong();

    // The action will be performed only if the action is not label and has the "Waiting" status
    if ((ActionList[N].ActionType != 0) && (ActionList[N].ActionStatus == 0))
    {
        // Changing the action status to "Working"
        ActionList[N].ActionStatus = 3;
        ActionWorking = N;

        // Destination packet
        FilePacket PacketDst;
        FilePacket PacketSrc;
        if (PacketDst.LoadIndex(GetPacketFile(ActionList[N].OutData)))
        {
            PacketDst.WorkDir = GetPacketDir(ActionList[N].OutData);
            PacketDst.CalcRange(WorkRange1, WorkRange2);

            // Source packet - loaded if there is required
            if (ActionList[N].ParamD != (-3))
            {
                if (PacketSrc.LoadIndex(GetPacketFile(ActionList[N].ParamD)))
                {
                    PacketSrc.WorkDir = GetPacketDir(ActionList[N].ParamD);
                    PacketSrc.CalcRange(WorkRange1, WorkRange2);

                    if (PacketSrc.GetSize(0) != PacketDst.GetSize(0))
                    {
                        ActionList[N].MSG = "ERROR: Packets " + ActionList[N].InVarName + " and " + ActionList[N].OutVarName + " has different sizes.";
                        Good = false;
                    }
                }
                else
                {
                    ActionList[N].MSG = "ERROR: Packet " + ActionList[N].InVarName + " cannot be open.";
                    Good = false;
                }

            }
        }
        else
        {
            ActionList[N].MSG = "ERROR: Packet " + ActionList[N].OutVarName + " cannot be open.";
            Good = false;
        }


        if (Good)
        {
            llong PacketChunkCount = ((WorkRange2 - WorkRange1 + 1) / DefaultPacketChunkSize) + 1;
            if (((WorkRange2 - WorkRange1 + 1) % DefaultPacketChunkSize) == 0)
            {
                PacketChunkCount--;
            }
            ActionProgressCurrent = 0;
            ActionProgressCount = PacketChunkCount;

            int AParamA = ActionList[N].ParamA;
            int AParamB = ActionList[N].ParamB;
            int AParamD = ActionList[N].ParamD;

            llong PacketChunkI;
            llong PacketChunkO;
            llong BufferI;
            llong PacketChunkSize = DefaultPacketChunkSize;

            // Performing action according to action type
            switch(ActionList[N].ActionType)
            {
            case 1:
                // Creating the X = M(A, D) packet
                if (AParamD != (-3))
                {
                    PacketDst.CreateBlankFiles();
                    PacketChunkO = WorkRange1;
                    for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                    {
                        if (PacketChunkI == 1)
                        {
                            PacketChunkSize = WorkRange2 - PacketChunkO + 1;
                        }

                        PacketSrc.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                        PacketDst.PrepareFragmentBuffer(PacketChunkSize);
                        for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                        {
                            PacketDst.FileBuffer[BufferI] = Mangle[AParamA][PacketSrc.FileBuffer[BufferI]];
                        }
                        PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, true);
                        ActionProgressCurrent++;
                        PacketChunkO += PacketChunkSize;
                    }
                }
                else
                {
                    // For every A: Mangle(A, 0x00) = 0x00
                    PacketDst.CreateBlankFiles();
                    PacketDst.PrepareFragmentBuffer(PacketChunkSize);
                    PacketChunkO = WorkRange1;
                    for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                    {
                        if (PacketChunkI == 1)
                        {
                            PacketChunkSize = WorkRange2 - PacketChunkO + 1;
                        }

                        PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, true);
                        ActionProgressCurrent++;
                        PacketChunkO += PacketChunkSize;
                    }
                }
                break;
            case 2:
                // X = X ^ M(A, D)
                if (AParamD != (-3))
                {
                    PacketChunkO = WorkRange1;
                    for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                    {
                        if (PacketChunkI == 1)
                        {
                            PacketChunkSize = WorkRange2 - PacketChunkO + 1;
                        }

                        PacketSrc.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                        PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                        for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                        {
                            PacketDst.FileBuffer[BufferI] = PacketDst.FileBuffer[BufferI] ^ Mangle[AParamA][PacketSrc.FileBuffer[BufferI]];
                        }
                        PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, true);
                        ActionProgressCurrent++;
                        PacketChunkO += PacketChunkSize;
                    }
                }
                break;
            case 3:
                // X = X ^ M(A, D) ^ M(B, D)
                if (AParamD != (-3))
                {
                    PacketChunkO = WorkRange1;
                    for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                    {
                        if (PacketChunkI == 1)
                        {
                            PacketChunkSize = WorkRange2 - PacketChunkO + 1;
                        }

                        PacketSrc.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                        PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                        for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                        {
                            PacketDst.FileBuffer[BufferI] = PacketDst.FileBuffer[BufferI] ^ Mangle[AParamA][PacketSrc.FileBuffer[BufferI]] ^ Mangle[AParamB][PacketSrc.FileBuffer[BufferI]];
                        }
                        PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, true);
                        ActionProgressCurrent++;
                        PacketChunkO += PacketChunkSize;
                    }
                }
                break;
            case 4:
                // X = XM(A, X)
                PacketChunkO = WorkRange1;
                for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                {
                    if (PacketChunkI == 1)
                    {
                        PacketChunkSize = WorkRange2 - PacketChunkO + 1;
                    }

                    PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                    for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                    {
                        PacketDst.FileBuffer[BufferI] = XMangle[AParamA][PacketDst.FileBuffer[BufferI]];
                    }
                    PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, true);
                    ActionProgressCurrent++;
                    PacketChunkO += PacketChunkSize;
                }
                break;
            case 5:
                // X = UM(A, X)
                PacketChunkO = WorkRange1;
                for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                {
                    if (PacketChunkI == 1)
                    {
                        PacketChunkSize = WorkRange2 - PacketChunkO + 1;
                    }

                    PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                    for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                    {
                        PacketDst.FileBuffer[BufferI] = Unmangle[AParamA][PacketDst.FileBuffer[BufferI]];
                    }
                    PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, true);
                    ActionProgressCurrent++;
                    PacketChunkO += PacketChunkSize;
                }
                break;
            case 6:
                // X = XM(A, UM(B, X))
                PacketChunkO = WorkRange1;
                for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                {
                    if (PacketChunkI == 1)
                    {
                        PacketChunkSize = WorkRange2 - PacketChunkO + 1;
                    }

                    PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                    for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                    {
                        PacketDst.FileBuffer[BufferI] = XMangle[AParamA][Unmangle[AParamB][PacketDst.FileBuffer[BufferI]]];
                    }
                    PacketDst.LoadSaveFragment(PacketChunkO, PacketChunkSize, true);
                    ActionProgressCurrent++;
                    PacketChunkO += PacketChunkSize;
                }
                break;
            }

            // Changing the status to "Done"
            ActionList[N].ActionStatus = 1;
        }
        else
        {
            // Changing the status to dummy to display error message
            ActionList[N].ActionStatus = -1;
        }
    }
    ActionProgressCurrent = 0;
    delete ProgressC;
    delete ProgressM;
    return Good;
}

///
/// \brief AppCore::GetProgress - Aquiring the current action progress
/// \return
///
int AppCore::GetActionProgress()
{
    if (ActionProgressCount > 0)
    {
        llong X = (ActionProgressCurrent * 1000) / ActionProgressCount;
        return X;
    }
    else
    {
        return 0;
    }
}

llong AppCore::ReadSizes()
{
    llong S = 0;
    FilePacket FP;
    for (int I = 0; I < 15; I++)
    {
        if ((FileD[I].compare("") != 0))
        {
            if (FP.LoadIndex(FileD[I]))
            {
                SizeD[I] = FP.GetSize(0);
                if (SizeD[I] >= 0)
                {
                    S = SizeD[I];
                }
            }
            else
            {
                SizeD[I] = -1;
            }
        }
    }
    if ((FileP.compare("") != 0))
    {
        if (FP.LoadIndex(FileP))
        {
            SizeP = FP.GetSize(0);
            if (SizeP >= 0)
            {
                S = SizeP;
            }
        }
        else
        {
            SizeP = -1;
        }
    }
    if ((FileQ.compare("") != 0))
    {
        if (FP.LoadIndex(FileQ))
        {
            SizeQ = FP.GetSize(0);
            if (SizeQ >= 0)
            {
                S = SizeQ;
            }
        }
        else
        {
            SizeQ = -1;
        }
    }
    return S;
}


string AppCore::IntegrityRangeStatusText(int I)
{
    switch (I)
    {
        case 0: return "Not tested";
        case 1: return "Passed";
        case 2: return "Failed";
        case 3: return "Testing";
        case 4: return "Error";
    }
    return "";
}

void AppCore::IntegrityRangeAdd(llong XBegin, llong XEnd)
{
    IntegrityRangeBegin.push_back(XBegin);
    IntegrityRangeEnd.push_back(XEnd);
    IntegrityRangeStatusP.push_back(0);
    IntegrityRangeStatusQ.push_back(0);
    IntegrityRangeSort();
}

void AppCore::IntegrityRangeRemove(int I)
{
    IntegrityRangeBegin.erase(IntegrityRangeBegin.begin() + I);
    IntegrityRangeEnd.erase(IntegrityRangeEnd.begin() + I);
    IntegrityRangeStatusP.erase(IntegrityRangeStatusP.begin() + I);
    IntegrityRangeStatusQ.erase(IntegrityRangeStatusQ.begin() + I);
}

void AppCore::IntegrityRangeSort()
{
    int L = IntegrityRangeBegin.size();
    for (int i = 0; i < L; i++)
    {
        int k = i;
        for (int j = i; j < L; j++)
        {
            if (IntegrityRangeBegin[j] < IntegrityRangeBegin[k])
            {
                k = j;
            }
        }
        if (i < k)
        {
            llong Td = IntegrityRangeBegin[i];
            IntegrityRangeBegin[i] = IntegrityRangeBegin[k];
            IntegrityRangeBegin[k] = Td;
            Td = IntegrityRangeEnd[i];
            IntegrityRangeEnd[i] = IntegrityRangeEnd[k];
            IntegrityRangeEnd[k] = Td;
            int Td0 = IntegrityRangeStatusP[i];
            IntegrityRangeStatusP[i] = IntegrityRangeStatusP[k];
            IntegrityRangeStatusP[k] = Td0;
            Td0 = IntegrityRangeStatusQ[i];
            IntegrityRangeStatusQ[i] = IntegrityRangeStatusQ[k];
            IntegrityRangeStatusQ[k] = Td0;
        }
    }
}

///
/// \brief AppCore::IntegrityScenarioGenerate - Generating the integrity test scenario
/// \param Mode
///
void AppCore::IntegrityActionGenerate(int Mode)
{
    // 1 - P only
    // 2 - Q only
    // 3 - Both P and Q simultaneously (the temporary file size will be twice than the sum of stripe sizes)
    // 4 - Both P and Q sequentially (the temporary file size will be the same as the sum of stripe sizes)

    // Clearing the scenario
    IntegrityActionStatus.clear();
    IntegrityActionType.clear();

    // Adding create temporary file action
    IntegrityActionStatus.push_back(0);
    IntegrityActionType.push_back(b11000111);


    // Creating the scenario
    switch (Mode)
    {
        case 0: // P only
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000000);
            for (int I = 0; I < DataN; I++)
            {
                IntegrityActionStatus.push_back(0);
                IntegrityActionType.push_back(I | b00000000);
            }
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000001);
            break;
        case 1: // Q only
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000000);
            for (int I = 0; I < DataN; I++)
            {
                IntegrityActionStatus.push_back(0);
                IntegrityActionType.push_back(I | b10000000);
            }
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000101);
            break;
        case 2: // P and Q sequentially
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000000);
            for (int I = 0; I < DataN; I++)
            {
                IntegrityActionStatus.push_back(0);
                IntegrityActionType.push_back(I | b00000000);
            }
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000001);
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000000);
            for (int I = 0; I < DataN; I++)
            {
                IntegrityActionStatus.push_back(0);
                IntegrityActionType.push_back(I | b10000000);
            }
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000101);
            break;
        case 3: // P and Q simultaneously
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000010);
            for (int I = 0; I < DataN; I++)
            {
                IntegrityActionStatus.push_back(0);
                IntegrityActionType.push_back(I | b01000000);
            }
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000001);
            IntegrityActionStatus.push_back(0);
            IntegrityActionType.push_back(b11000100);
            break;
    }

    // Adding remove temporary file action
    IntegrityActionStatus.push_back(0);
    IntegrityActionType.push_back(b11000110);

}

///
/// \brief AppCore::IntegrityActionText - Getting the label for the scenario item
/// \param N
/// \return
///
string AppCore::IntegrityActionText(int N)
{
    // Checking if the item number is contained in the scenario
    if ((N < 0) || (N >= (int)IntegrityActionStatus.size()))
    {
        return ("X\tX");
    }

    string X = "\t";

    // Action status
    switch (IntegrityActionStatus[N])
    {
        case 0: X = "Waiting" + X; break;
        case 1: X = "Done" + X; break;
        case 2: X = "Postponed" + X; break;
        case 3: X = "Working" + X; break;
    }

    // Action type
    switch (IntegrityActionType[N] & b11100000)
    {
        case b00000000:
            if ((IntegrityActionType[N] & b00011111) == b00011111)
            {
                X += "Read P";
            }
            else
            {
                X += "Read " + PacketToText(IntegrityActionType[N] & b00011111) + " for P";
            }
            break;
        case b10000000:
            if ((IntegrityActionType[N] & b00011111) == b00011111)
            {
                X += "Read Q";
            }
            else
            {
                X += "Read " + PacketToText(IntegrityActionType[N] & b00011111) + " for Q";
            }
            break;
        case b01000000:
            if ((IntegrityActionType[N] & b00011111) == b00011111)
            {
                X += "Read ?";
            }
            else
            {
                X += "Read " + PacketToText(IntegrityActionType[N] & b00011111) + " for P and Q";
            }
            break;
        case b11000000:
            switch (IntegrityActionType[N] & b00011111)
            {
                case b00000000: X += "Create blank file"; break;
                case b00000010: X += "Create blank file"; break;
                case b00000001: X += "Check file - P"; break;
                case b00000101: X += "Check file - Q"; break;
                case b00000100: X += "Check file - Q"; break;
                case b00000111: X += "Reset test status, create temporary file"; break;
                case b00000110: X += "Remove temporary file"; break;
                case b00000011: X += "XXX"; break;
            }
            break;
    }

    return X;
}


///
/// \brief AppCore::IntegrityActionSetStatus - Changing the integrity test action status
/// \param N
/// \param S
///
void AppCore::IntegrityActionSetStatus(int N, int S)
{
    if ((S >= 0) && (S <= 2) && (N >= 0) && (N < (int)IntegrityActionStatus.size()))
    {
        IntegrityActionStatus[N] = S;
    }
}


string AppCore::IntegrityPerformWaiting(bool GetSize)
{
    IntegrityRangeAllSize = 0;

    // Checking if the stripes are defined
    if (IntegrityRangeStatusP.size() == 0)
    {
        return "None of data stripes";
    }

    IntegrityProgressCurrent = 0;
    IntegrityProgressCount = 0;
    IntegrityRangeWorkChunks.clear();
    IntegrityRangeWorkOffset.clear();
    llong WorkOffset = 0;
    for (int I = 0; I < (int)IntegrityRangeStatusP.size(); I++)
    {
        // Checking if the stripe is correct
        if ((IntegrityRangeBegin[I] > IntegrityRangeEnd[I]) || (IntegrityRangeBegin[I] < 0))
        {
            return "At least one stripe is not defined correctly";
        }

        // Calculating the number of packets in the stripe
        llong PacketChunkCount = ((IntegrityRangeEnd[I] - IntegrityRangeBegin[I] + 1) / IntegrityPacketChunkSize) + 1;
        if (((IntegrityRangeEnd[I] - IntegrityRangeBegin[I] + 1) % IntegrityPacketChunkSize) == 0)
        {
            PacketChunkCount--;
        }

        IntegrityRangeWorkChunks.push_back(PacketChunkCount);
        IntegrityProgressCount += PacketChunkCount;
        IntegrityRangeWorkOffset.push_back(WorkOffset);
        WorkOffset = WorkOffset + (IntegrityRangeEnd[I] - IntegrityRangeBegin[I] + 1);
    }
    IntegrityRangeAllSize = WorkOffset;

    if (GetSize)
    {
        for (uint I = 0; I < IntegrityActionType.size(); I++)
        {
            if (IntegrityActionType[I] == b11000000)
            {
                return to_string(IntegrityRangeAllSize);
            }
            if (IntegrityActionType[I] == b11000010)
            {
                return to_string(IntegrityRangeAllSize * 2);
            }
        }
        return to_string(0);
    }
    else
    {
        // Performing the integrity test
        IntegrityWorking = 99999999;
        thread Thr(&AppCore::IntegrityPerformWaitingWork, this);
        Thr.detach();
    }
    return "";
}

void AppCore::IntegrityPerformWaitingWork()
{
    uint I;

    for (I = 0; I < IntegrityActionType.size(); I++)
    {
        if (IntegrityActionStatus[I] == 0)
        {
            if (!IntegrityPerform(I))
            {
                I = IntegrityActionType.size();
            }
        }
    }
    IntegrityWorking = -1;
}

///
/// \brief AppCore::IntegrityPerform - Performing the one integrity test action
/// \param N
/// \return
///
bool AppCore::IntegrityPerform(uint N)
{
    IntegrityProgressCurrent = 0;

    bool Good = true;

    // Changing the status to "Working"
    IntegrityActionStatus[N] = 3;
    IntegrityWorking = N;

    // Variables associated with packet chunks
    llong PacketChunkI;
    llong PacketChunkO;
    llong PacketChunkSize;
    llong PacketChunkCount;
    llong BufferI;

    vector<uchar> FileBuffer;
    FileBuffer.resize(IntegrityPacketChunkSize);
    fill(FileBuffer.begin(), FileBuffer.end(), 0);

    // Creating temporary file is not a iteration by file stripes
    if ((IntegrityActionType[N] == b11000000) || (IntegrityActionType[N] == b11000010))
    {
        PacketChunkSize = IntegrityPacketChunkSize;
        PacketChunkCount = (IntegrityRangeAllSize / IntegrityPacketChunkSize) + 1;
        PacketChunkO = 0;

        fstream F(IntegrityTempFileName, ios::binary | ios::out | ios::in);
        if (F.is_open())
        {
            switch (IntegrityActionType[N])
            {
                case b11000000:
                    // Creating the blank file with the same size as sum of stripes
                    for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                    {
                        if (PacketChunkI == 1)
                        {
                            PacketChunkSize = IntegrityRangeAllSize - PacketChunkO;
                        }
                        F.write((char*)FileBuffer.data(), PacketChunkSize);
                        IntegrityProgressCurrent++;
                        PacketChunkO += PacketChunkSize;
                    }
                    break;

                case b11000010:
                    // Creating the blank file with the twice size as sum of stripes
                    for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                    {
                        if (PacketChunkI == 1)
                        {
                            PacketChunkSize = IntegrityRangeAllSize - PacketChunkO;
                        }
                        F.write((char*)FileBuffer.data(), PacketChunkSize);
                        F.write((char*)FileBuffer.data(), PacketChunkSize);
                        IntegrityProgressCurrent++;
                        PacketChunkO += PacketChunkSize;
                    }
                    break;
            }
            F.close();
        }
    }
    else
    {
        // The action is performed for every stripe
        for (uint RangeI = 0; RangeI < IntegrityRangeStatusP.size(); RangeI++)
        {
            // Encounting packet quantity
            PacketChunkCount = IntegrityRangeWorkChunks[RangeI];
            PacketChunkSize = IntegrityPacketChunkSize;

            // Performing the action
            switch (IntegrityActionType[N] & b11100000)
            {
                case b00000000:
                case b10000000:
                case b01000000:
                    {
                        // Reading file contents to insert this in the temporary file
                        FilePacket PacketWork;
                        int FX = (IntegrityActionType[N] & b00011111);
                        int ReadMangle = (((IntegrityActionType[N] & b11100000) == b10000000) || ((IntegrityActionType[N] & b11100000) == b01000000)) ? FX : 0;
                        bool PQ = ((IntegrityActionType[N] & b11100000) == b01000000);
                        if (PacketWork.LoadIndex(GetPacketFile(FX)))
                        {
                            PacketWork.WorkDir = GetPacketDir(FX);

                            if (PacketWork.GetSize(0) <= IntegrityRangeEnd[RangeI])
                            {
                                //ActionList[N].MSG = "ERROR: Packet " + ActionList[N].InVarName + " is too small.";
                                Good = false;
                            }
                        }
                        else
                        {
                            //ActionList[N].MSG = "ERROR: Packet " + ActionList[N].InVarName + " cannot be open.";
                            Good = false;
                        }

                        if (Good)
                        {
                            fstream F(IntegrityTempFileName, ios::binary | ios::out | ios::in);
                            if (F.is_open())
                            {
                                llong PacketWorkPos = IntegrityRangeWorkOffset[RangeI];
                                llong PacketWorkPos_ = PacketWorkPos + IntegrityRangeAllSize;

                                PacketChunkO = IntegrityRangeBegin[RangeI];
                                for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                                {
                                    if (PacketChunkI == 1)
                                    {
                                        PacketChunkSize = IntegrityRangeEnd[RangeI] - PacketChunkO + 1;
                                    }
                                    PacketWork.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                                    if (PQ)
                                    {
                                        F.seekg(PacketWorkPos);
                                        F.read((char*)FileBuffer.data(), PacketChunkSize);
                                        for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                                        {
                                            FileBuffer[BufferI] = FileBuffer[BufferI] ^ Mangle[0][PacketWork.FileBuffer[BufferI]];
                                        }
                                        F.seekp(PacketWorkPos);
                                        F.write((char*)FileBuffer.data(), PacketChunkSize);

                                        F.seekg(PacketWorkPos_);
                                        F.read((char*)FileBuffer.data(), PacketChunkSize);
                                        for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                                        {
                                            FileBuffer[BufferI] = FileBuffer[BufferI] ^ Mangle[ReadMangle][PacketWork.FileBuffer[BufferI]];
                                        }
                                        F.seekp(PacketWorkPos_);
                                        F.write((char*)FileBuffer.data(), PacketChunkSize);
                                    }
                                    else
                                    {
                                        F.seekg(PacketWorkPos);
                                        F.read((char*)FileBuffer.data(), PacketChunkSize);
                                        for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                                        {
                                            FileBuffer[BufferI] = FileBuffer[BufferI] ^ Mangle[ReadMangle][PacketWork.FileBuffer[BufferI]];
                                        }
                                        F.seekp(PacketWorkPos);
                                        F.write((char*)FileBuffer.data(), PacketChunkSize);
                                    }

                                    IntegrityProgressCurrent++;
                                    PacketChunkO += PacketChunkSize;
                                    PacketWorkPos += PacketChunkSize;
                                    PacketWorkPos_ += PacketChunkSize;
                                }
                                F.close();
                            }
                        }
                        break;
                    }
                case b11000000:
                    switch (IntegrityActionType[N] & b00011111)
                    {
                        case b00000001:
                        case b00000101:
                        case b00000100:
                            {
                                IntegrityWorking = 1000 + RangeI;
                                // Comparing the temporary file with the P or the Q packet
                                bool IsQ = (((IntegrityActionType[N] & b00011111) == b00000101) || ((IntegrityActionType[N] & b00011111) == b00000100));
                                int Passed = 1;
                                int FX;
                                if (IsQ)
                                {
                                    IntegrityRangeStatusQ[RangeI] = 3;
                                    FX = -2;
                                }
                                else
                                {
                                    IntegrityRangeStatusP[RangeI] = 3;
                                    FX = -1;
                                }


                                FilePacket PacketWork;
                                if (PacketWork.LoadIndex(GetPacketFile(FX)))
                                {
                                    PacketWork.WorkDir = GetPacketDir(FX);

                                    if (PacketWork.GetSize(0) <= IntegrityRangeEnd[RangeI])
                                    {
                                        //ActionList[N].MSG = "ERROR: Packet " + ActionList[N].InVarName + " is too small.";
                                        Good = false;
                                    }
                                }
                                else
                                {
                                    //ActionList[N].MSG = "ERROR: Packet " + ActionList[N].InVarName + " cannot be open.";
                                    Good = false;
                                }


                                if (Good)
                                {
                                    fstream F(IntegrityTempFileName, ios::binary | ios::in);
                                    if (F.is_open())
                                    {
                                        if (((IntegrityActionType[N] & b00011111) == b00000100))
                                        {
                                            F.seekg(IntegrityRangeWorkOffset[RangeI] + IntegrityRangeAllSize);
                                        }
                                        else
                                        {
                                            F.seekg(IntegrityRangeWorkOffset[RangeI]);
                                        }

                                        PacketChunkO = IntegrityRangeBegin[RangeI];
                                        for (PacketChunkI = PacketChunkCount; PacketChunkI > 0; PacketChunkI--)
                                        {
                                            if (PacketChunkI == 1)
                                            {
                                                PacketChunkSize = IntegrityRangeEnd[RangeI] - PacketChunkO + 1;
                                            }
                                            if (Passed == 1)
                                            {
                                                PacketWork.LoadSaveFragment(PacketChunkO, PacketChunkSize, false);
                                                F.read((char*)FileBuffer.data(), PacketChunkSize);
                                                for (BufferI = 0; BufferI < PacketChunkSize; BufferI++)
                                                {
                                                    if (FileBuffer[BufferI] != PacketWork.FileBuffer[BufferI])
                                                    {
                                                        Passed = 2;
                                                    }
                                                }
                                            }
                                            IntegrityProgressCurrent++;
                                            PacketChunkO += PacketChunkSize;
                                        }
                                        F.close();
                                    }
                                    else
                                    {
                                        Passed = 4;
                                    }
                                }
                                else
                                {
                                    Passed = 4;
                                }

                                if (IsQ)
                                {
                                    IntegrityRangeStatusQ[RangeI] = Passed;
                                }
                                else
                                {
                                    IntegrityRangeStatusP[RangeI] = Passed;
                                }
                                break;
                            }
                        case b00000111:
                            {
                                // Setting the status as "Not tested"
                                IntegrityRangeStatusP[RangeI] = 0;
                                IntegrityRangeStatusQ[RangeI] = 0;

                                // Creating the temporary file
                                remove(IntegrityTempFileName.c_str());
                                fstream F(IntegrityTempFileName, ios::binary | ios::out);
                                F.close();
                                break;
                            }
                        case b00000110:
                            {
                                // Removing the temporary file
                                remove(IntegrityTempFileName.c_str());
                                break;
                            }
                    }
                    break;
            }
        }
    }
    IntegrityWorking = N;


    // Changing the action status to "Done"
    IntegrityActionStatus[N] = 1;


    IntegrityProgressCurrent = 0;

    return Good;
}

int AppCore::GetIntegrityProgress()
{
    if (IntegrityProgressCount > 0)
    {
        llong X = (IntegrityProgressCurrent * 1000) / IntegrityProgressCount;
        return X;
    }
    else
    {
        return 0;
    }
}
