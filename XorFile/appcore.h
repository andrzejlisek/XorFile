#ifndef APPCORE_H
#define APPCORE_H

#include "binary.h"
#include "eden.h"
#include <iostream>
#include <vector>
#include "xoraction.h"
#include <string>
#include <sstream>
#include <fstream>
#include "filepacket.h"
#include "filepacketedit.h"
#include <thread>
#include "configfile.h"

#define PacketToText(X) (((X) >= 0) ? (((X) < 10) ? ("D0" + to_string(X)) : ("D" + to_string(X))) : (((X) == -1) ? "P" : (((X) == -2) ? "Q" : "?")))

using namespace std;
class AppCore
{
    friend class CalcTest;
public:
    int ActionWorking;
    int IntegrityWorking;
    AppCore();
    void GenerateScenarioRecoverD(int DataNum, int Recalc, int Reduce);
    int GenerateScenario();
    string GetPacketText(int N, bool OnlySize);
    int GetPacketNum(int N);
    void SetPacketStatus(int N, int PacketStatus);
    int GetPacketStatus(int N);
    void SetPacketFile(int N, string FileName);
    string GetPacketFile(int N);
    void SetPacketDir(int N, string DirName);
    string GetPacketDir(int N);
    string ReadPacketDir(int N);
    void SetPacketQuan(int N);
    int GetPacketQuan();
    vector<XorAction> ActionList;
    void ProjectNew();
    void ProjectLoad(string FileName);
    void ProjectSave(string FileName);
    void ActionSetStatus(uint N, int S);
    void ActionPerformWaiting();
    void ActionPerformWaitingWork();
    bool ActionPerform(uint N);

    string IntegrityPerformWaiting(bool GetSize);
    void IntegrityPerformWaitingWork();
    bool IntegrityPerform(uint N);

    llong DefaultPacketChunkSize;
    llong IntegrityPacketChunkSize;
    int GetActionProgress();
    int GetIntegrityProgress();

    llong ReadSizes();
    void WorkRangeSet(llong R1, llong R2);
    llong WorkRangeGet(int R);


    vector<llong> IntegrityRangeBegin;
    vector<llong> IntegrityRangeEnd;
    vector<int> IntegrityRangeStatusP;
    vector<int> IntegrityRangeStatusQ;
    vector<llong> IntegrityRangeWorkChunks;
    vector<llong> IntegrityRangeWorkOffset;
    llong IntegrityRangeAllSize;
    string IntegrityRangeStatusText(int I);
    void IntegrityRangeAdd(llong XBegin, llong XEnd);
    void IntegrityRangeRemove(int I);
    void IntegrityRangeSort();

    vector<int> IntegrityActionStatus;
    vector<int> IntegrityActionType;

    void IntegrityActionGenerate(int Mode);
    string IntegrityActionText(int N);
    void IntegrityActionSetStatus(int N, int S);

    string IntegrityTempFileName;

private:


    llong WorkRange1;
    llong WorkRange2;

    uchar Mangle[16][256];
    uchar Unmangle[16][256];
    uchar XMangle[16][256];
    uchar XUnmangle[16][256];
    uchar XIteration[16];

    int DataN;

    int StatusD[15];
    int StatusP;
    int StatusQ;

    string FileD[15];
    string FileP;
    string FileQ;

    string DirD[15];
    string DirP;
    string DirQ;

    llong SizeD[15];
    llong SizeP;
    llong SizeQ;

    llong ActionProgressCurrent;
    llong ActionProgressCount;
    llong IntegrityProgressCurrent;
    llong IntegrityProgressCount;
};

#endif // APPCORE_H
