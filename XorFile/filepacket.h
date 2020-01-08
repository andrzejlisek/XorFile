#ifndef FILEPACKET_H
#define FILEPACKET_H

#include "Eden.h"
#include <vector>
#include <string>
#include <climits>
#include <fstream>
#include <iostream>
class FilePacket
{
public:
    FilePacket();

    vector<uchar> FileBuffer;
    vector<uchar> FileBufferTemp;

    ///
    /// \brief Working directory
    ///
    string WorkDir;

    ///
    /// \brief Current file name
    ///
    string CurrentFileName;

    void Clear();
    bool LoadIndex(string FileName);
    bool SaveIndex(string FileName);
    void CreateBlankFiles();
    void LoadSaveFragment(llong FileBegin, llong FileLength, bool Save);
    void PrepareFragmentBuffer(llong FileLength);
    llong GetSize(llong ChunkSize);

    string GetText(int N);
    int FileCount();
    void CalcOffset();
    void CalcRange(llong Range1, llong Range2);

protected:
    bool ValidFileName(string FN);

    void LoadFragmentFromOneFile(string FileName, llong FileBegin, llong Size);
    void SaveFragmentTo__OneFile(string FileName, llong FileBegin);

    llong PacketSize;


    ///
    /// \brief File index - single file name
    ///
    vector<string> IdxFileName;

    ///
    /// \brief File index - single file begin offset
    ///
    vector<llong> IdxFileBegin;

    ///
    /// \brief File index - single file end offset
    ///
    vector<llong> IdxFileEnd;

    ///
    /// \brief File index - single file size
    ///
    vector<llong> IdxFileSize;

    ///
    /// \brief File index - single file digest
    ///
    vector<string> IdxFileDigest;

    ///
    /// \brief File index - displaying status information
    ///
    vector<string> IdxFileInfo;

    ///
    /// \brief File index - include this file in read/write operations
    ///
    vector<bool> IdxFileInclude;


    void CopyToTemp(llong Offset, llong Length);
    void CopyFromTemp(llong Offset);
};

#endif // FILEPACKET_H
