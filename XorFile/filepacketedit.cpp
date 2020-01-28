#include "filepacketedit.h"

FilePacketEdit::FilePacketEdit()
{

}

FilePacketEdit::~FilePacketEdit()
{

}

///
/// \brief FilePacketEdit::ClearInfo - Clear the status information of all items
///
void FilePacketEdit::ClearInfo(int N)
{
    if (N >= 0)
    {
        IdxFileInfo[N] = "";
    }
    else
    {
        for (uint I = 0; I < IdxFileInfo.size(); I++)
        {
            IdxFileInfo[I] = "";
        }
    }
}

///
/// \brief FilePacketEdit::ItemGetSize - Get the item size
/// \param N
/// \return
///
llong FilePacketEdit::ItemGetSize(int N)
{
    return IdxFileSize[N];
}

///
/// \brief FilePacketEdit::ItemSetSize - Set the item size
/// \param N
/// \param NewSize
/// \return
///
int FilePacketEdit::ItemSetSize(int N, llong NewSize)
{
    if (NewSize >= 0)
    {
        IdxFileSize[N] = NewSize;
        CalcOffset();
        return 0;
    }
    else
    {
        return 1;
    }
}

///
/// \brief FilePacketEdit::ItemGetDigest - Get the item digest
/// \param N
/// \return
///
string FilePacketEdit::ItemGetDigest(int N)
{
    if (IdxFileDigest[N] == "X")
    {
        return "";
    }
    else
    {
        return (IdxFileDigest[N]);
    }
}

///
/// \brief FilePacketEdit::ItemSetDigest - Set the item digest
/// \param N
/// \param Digest
/// \return
///
int FilePacketEdit::ItemSetDigest(int N, string Digest)
{
    if ((Digest == "") || (Digest == "X"))
    {
        // Removing the digest
        IdxFileDigest[N] = "X";
        return 0;
    }
    else
    {
        // Checking if the digest consists of 32 characters
        if (Digest.size() != 32)
        {
            return 1;
        }

        // Checking if the digest consists of hexadecimal digits only
        for (int I = 0; I < 32; I++)
        {
            if (Digest[I] == 'a') { Digest[I] = 'A'; }
            if (Digest[I] == 'b') { Digest[I] = 'B'; }
            if (Digest[I] == 'c') { Digest[I] = 'C'; }
            if (Digest[I] == 'd') { Digest[I] = 'D'; }
            if (Digest[I] == 'e') { Digest[I] = 'E'; }
            if (Digest[I] == 'f') { Digest[I] = 'F'; }

            bool UnStd = true;
            if (Digest[I] == '0') { UnStd = false; }
            if (Digest[I] == '1') { UnStd = false; }
            if (Digest[I] == '2') { UnStd = false; }
            if (Digest[I] == '3') { UnStd = false; }
            if (Digest[I] == '4') { UnStd = false; }
            if (Digest[I] == '5') { UnStd = false; }
            if (Digest[I] == '6') { UnStd = false; }
            if (Digest[I] == '7') { UnStd = false; }
            if (Digest[I] == '8') { UnStd = false; }
            if (Digest[I] == '9') { UnStd = false; }
            if (Digest[I] == 'A') { UnStd = false; }
            if (Digest[I] == 'B') { UnStd = false; }
            if (Digest[I] == 'C') { UnStd = false; }
            if (Digest[I] == 'D') { UnStd = false; }
            if (Digest[I] == 'E') { UnStd = false; }
            if (Digest[I] == 'F') { UnStd = false; }

            if (UnStd)
            {
                return 1;
            }
        }

        IdxFileDigest[N] = Digest;
        return 0;
    }
}



///
/// \brief FilePacketEdit::ItemDigestCompute - Compute the digest
///
int FilePacketEdit::ItemDigestCompute(int N, bool Batch)
{
    if (!Batch)
    {
        DigestWorkingAll = 0;
    }
    DigestWorking = true;
    IdxFileDigest[N] = "X";
    if (IdxFileName[N] == "")
    {
        if (Batch)
        {
            DigestWorking = false;
        }
        IdxFileInfo[N] = "No file";
        return 1;
    }
    try
    {
        IdxFileInfo[N] = "Working";
        string FileName = WorkDir + IdxFileName[N];

        if (!Eden::FileExists(FileName))
        {
            if (Batch)
            {
                DigestWorking = false;
            }
            IdxFileInfo[N] = "ERROR: File not found or file open error";
            return 1;
        }
        if (IdxFileSize[N] != Eden::FileSize(FileName))
        {
            if (Batch)
            {
                DigestWorking = false;
            }
            IdxFileInfo[N] = "ERROR: File size mismatch - real file size: " + to_string(Eden::FileSize(FileName));
            return 1;
        }
        CalcOffset();
        thread Thr(&FilePacketEdit::ItemDigestComputeWork, this, N, FileName);
        Thr.detach();

        return 0;
    }
    catch (...)
    {
        if (Batch)
        {
            DigestWorking = false;
        }
        IdxFileInfo[N] = "ERROR: File not found or file open error";
        return 1;
    }
}

///
/// \brief FilePacketEdit::ItemDigestComputeWork - Compute the digest - working method
/// \param N
/// \param FileName
///
void FilePacketEdit::ItemDigestComputeWork(int N, string FileName)
{
    DigestResult = 0;
    DigestObj = new EdenClass::HashCalcMD5();
    try
    {
        DigestCalc = DigestObj->CalcObj(FileName.c_str());
        IdxFileDigest[N] = Eden::ToHex(DigestCalc, 16);
        delete[] DigestCalc;
    }
    catch (...)
    {
        IdxFileInfo[N] = "ERROR: File not found or file open error";
        DigestResult = 1;
    }
    delete DigestObj;
    DigestObj = NULL;
    DigestWorking = false;
    if (DigestResult == 0)
    {
        IdxFileInfo[N] = "OK";
    }
}

///
/// \brief FilePacketEdit::ItemDigestCheck - Check the digest
///
int FilePacketEdit::ItemDigestCheck(int N, bool Batch, bool CheckDigest)
{
    if (!Batch)
    {
        DigestWorkingAll = 0;
    }
    if (IdxFileName[N] == "")
    {
        IdxFileInfo[N] = "No file";
        return 1;
    }
    try
    {
        IdxFileInfo[N] = "Working";
        string FileName = WorkDir + IdxFileName[N];

        if (!Eden::FileExists(FileName))
        {
            IdxFileInfo[N] = "ERROR: File not found or file open error";
            return 1;
        }
        if (Eden::FileSize(FileName) != IdxFileSize[N])
        {
            IdxFileInfo[N] = "FAIL: File size mismatch - real file size: " + to_string(Eden::FileSize(FileName));
            return 2;
        }

        if (CheckDigest)
        {
            if ((IdxFileDigest[N] == "X") || (IdxFileDigest[N] == ""))
            {
                IdxFileInfo[N] = "FAIL: File size match, but digest is not defined";
                return 4;
            }
            DigestWorking = true;
            thread Thr(&FilePacketEdit::ItemDigestCheckWork, this, N, FileName);
            Thr.detach();
        }
        else
        {
            IdxFileInfo[N] = "PASS: File size match";
            return 4;
        }

        return 0;
    }
    catch (...)
    {
        DigestWorking = true;
        IdxFileInfo[N] = "ERROR: File not found or file open error";
        return 1;
    }
}

///
/// \brief FilePacketEdit::ItemDigestCheckWork - Check the digest - working method
/// \param N
/// \param FileName
///
void FilePacketEdit::ItemDigestCheckWork(int N, string FileName)
{
    DigestResult = 0;
    DigestObj = new EdenClass::HashCalcMD5();
    try
    {
        IdxFileInfo[N] = "Working";
        DigestCalc = DigestObj->CalcObj(FileName.c_str());
        string DigestS = Eden::ToHex(DigestCalc, 16);
        if (DigestS != IdxFileDigest[N])
        {
            IdxFileInfo[N] = "FAIL: File size match, but file digest mismatch with definition";
            DigestResult = 2;
        }
        else
        {
            IdxFileInfo[N] = "PASS: File size and digest match with definition";
            DigestResult = 3;
        }
    }
    catch (...)
    {
        DigestResult = 1;
        IdxFileInfo[N] = "ERROR: File not found or file open error";
    }
    delete[] DigestCalc;
    delete DigestObj;
    DigestObj = NULL;
    DigestWorking = false;
}

///
/// \brief FilePacketEdit::GetDigestProgress - Get the progress of digest computation
/// \return
///
int FilePacketEdit::GetDigestProgress()
{
    try
    {
        if (DigestObj != NULL)
        {
            llong X = DigestObj->ChunkAll;
            if (X > 0)
            {
                X = (DigestObj->ChunkCurrent * 1000) / DigestObj->ChunkAll;
                return X;
            }
        }
    }
    catch (...)
    {
    }
    return 0;
}

void FilePacketEdit::ItemDirAbsRel(int N)
{
    IdxFileName[N] = EdenClass::FileDir::DirAbsoluteToRelative(WorkDir, IdxFileName[N]);
}

void FilePacketEdit::ItemDirRelAbs(int N)
{
    IdxFileName[N] = EdenClass::FileDir::DirRelativeToAbsolute(WorkDir, IdxFileName[N]);
}

void FilePacketEdit::ItemAdd(string FileName, llong FileSize)
{
    if (FileSize < 0)
    {
        FileSize = Eden::FileSize(FileName);
    }
    IdxFileName.push_back(FileName);
    IdxFileSize.push_back(FileSize);
    IdxFileDigest.push_back("X");
    IdxFileInfo.push_back("");
    IdxFileBegin.push_back(0);
    IdxFileEnd.push_back(0);
    PacketSize += FileSize;
    CalcOffset();
}

void FilePacketEdit::ItemRemove(int N)
{
    PacketSize -= IdxFileSize[N];
    IdxFileName.erase(IdxFileName.begin() + N);
    IdxFileSize.erase(IdxFileSize.begin() + N);
    IdxFileDigest.erase(IdxFileDigest.begin() + N);
    IdxFileInfo.erase(IdxFileInfo.begin() + N);
    IdxFileBegin.erase(IdxFileBegin.begin() + N);
    IdxFileEnd.erase(IdxFileEnd.begin() + N);
    CalcOffset();
}

void FilePacketEdit::ItemMoveUp(uint N)
{
    if (N > 0)
    {
        string TempName = IdxFileName[N];
        llong TempSize = IdxFileSize[N];
        string TempDigest = IdxFileDigest[N];
        string TempInfo = IdxFileInfo[N];
        IdxFileName[N] = IdxFileName[N - 1];
        IdxFileSize[N] = IdxFileSize[N - 1];
        IdxFileDigest[N] = IdxFileDigest[N - 1];
        IdxFileInfo[N] = IdxFileInfo[N - 1];
        IdxFileName[N - 1] = TempName;
        IdxFileSize[N - 1] = TempSize;
        IdxFileDigest[N - 1] = TempDigest;
        IdxFileInfo[N - 1] = TempInfo;
    }
    CalcOffset();
}

void FilePacketEdit::ItemMoveDown(uint N)
{
    if (N < (IdxFileName.size() - 1))
    {
        string TempName = IdxFileName[N];
        llong TempSize = IdxFileSize[N];
        string TempDigest = IdxFileDigest[N];
        string TempInfo = IdxFileInfo[N];
        IdxFileName[N] = IdxFileName[N + 1];
        IdxFileSize[N] = IdxFileSize[N + 1];
        IdxFileDigest[N] = IdxFileDigest[N + 1];
        IdxFileInfo[N] = IdxFileInfo[N + 1];
        IdxFileName[N + 1] = TempName;
        IdxFileSize[N + 1] = TempSize;
        IdxFileDigest[N + 1] = TempDigest;
        IdxFileInfo[N + 1] = TempInfo;
    }
    CalcOffset();
}
