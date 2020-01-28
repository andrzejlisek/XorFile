#ifndef FILEPACKETEDIT_H
#define FILEPACKETEDIT_H

#include "filepacket.h"
#include <thread>
#include "filedir.h"
#include "hashcalcmd5.h"

class FilePacketEdit : public FilePacket
{
public:
    FilePacketEdit();
    ~FilePacketEdit();
    int ItemSetSize(int N, llong NewSize);
    llong ItemGetSize(int N);
    int ItemSetDigest(int N, string Digest);
    string ItemGetDigest(int N);
    void ClearInfo(int N);


    bool DigestWorking;
    int DigestWorkingAll;
    int BatchCurrent;
    int DigestResult;
    int GetDigestProgress();


    int ItemDigestCompute(int N, bool Batch);
    void ItemDigestComputeWork(int N, string FileName);
    int ItemDigestCheck(int N, bool Batch, bool CheckDigest);
    void ItemDigestCheckWork(int N, string FileName);

    void ItemDirAbsRel(int N);
    void ItemDirRelAbs(int N);

    void ItemAdd(string FileName, llong FileSize);
    void ItemRemove(int N);
    void ItemMoveUp(uint N);
    void ItemMoveDown(uint N);

private:
    EdenClass::HashCalcMD5 * DigestObj;
    uchar * DigestCalc;

};

#endif // FILEPACKETEDIT_H
