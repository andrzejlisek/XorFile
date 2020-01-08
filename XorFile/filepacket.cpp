#include "filepacket.h"

FilePacket::FilePacket()
{
    Clear();
}

///
/// \brief FilePacket::Clear - Clear the file index
///
void FilePacket::Clear()
{
    CurrentFileName = "";
    IdxFileName.clear();
    IdxFileBegin.clear();
    IdxFileEnd.clear();
    IdxFileSize.clear();
    IdxFileDigest.clear();
    IdxFileInfo.clear();
    IdxFileInclude.clear();
    PacketSize = 0;
}

///
/// \brief FilePacket::LoadIndex - Load the file index
/// \param FileName
/// \return
///
bool FilePacket::LoadIndex(string FileName)
{
    Clear();

    fstream IdxF(FileName.c_str(), ios::in);
    if (IdxF.is_open())
    {
        PacketSize = 0;
        llong ByteCounter = 0;
        string Buf;
        getline(IdxF, Buf);
        while (Buf[0] != '*')
        {
            IdxFileName.push_back(Buf);
            getline(IdxF, Buf);
            llong FS = Eden::ToLLong(Buf);
            PacketSize += FS;
            IdxFileSize.push_back(FS);
            IdxFileBegin.push_back(ByteCounter);
            ByteCounter = ByteCounter + FS;
            IdxFileEnd.push_back(ByteCounter - 1);
            getline(IdxF, Buf);
            IdxFileDigest.push_back(Buf);
            getline(IdxF, Buf);
            IdxFileInfo.push_back("");
            IdxFileInclude.push_back(true);
        }
        WorkDir = Buf = Buf.substr(1);
        IdxF.close();
        CurrentFileName = FileName;
        return true;
    }
    else
    {
        return false;
    }
}

///
/// \brief FilePacket::SaveIndex - Save the file index
/// \param FileName
/// \return
///
bool FilePacket::SaveIndex(string FileName)
{
    fstream IdxF(FileName.c_str(), ios::out);
    if (IdxF.is_open())
    {
        int N = IdxFileName.size();
        for (int I = 0; I < N; I++)
        {
            IdxF << IdxFileName[I] << endl;
            IdxF << IdxFileSize[I] << endl;
            IdxF << IdxFileDigest[I] << endl;
        }
        IdxF << "*" << WorkDir << endl;
        IdxF.close();
        CurrentFileName = FileName;
        return true;
    }
    else
    {
        return false;
    }
}

///
/// \brief FilePacket::CreateBlankFiles - Create blank files based upon the index
///
void FilePacket::CreateBlankFiles()
{
    for (unsigned int i = 0; i < IdxFileName.size(); i++)
    {
        if (IdxFileInclude[i])
        {
            string FileName = WorkDir + IdxFileName[i];
            if (ValidFileName(FileName))
            {
                if (!Eden::FileExists(FileName))
                {
                    fstream F(FileName.c_str(), ios::out|ios::binary);
                    F.close();
                }
            }

        }
    }
}

///
/// \brief FilePacket::ValidFileName - Check, if the file name is correct
/// \param FN
/// \return
///
bool FilePacket::ValidFileName(string FN)
{
    if ((FN != "") && (FN[FN.size() - 1] != '?'))
    {
        return true;
    }
    else
    {
        return false;
    }
}

///
/// \brief FilePacket::LoadFragmentFromOneFile - Load data from single file
/// \param FileName
/// \param FileBegin
/// \param Size
///
void FilePacket::LoadFragmentFromOneFile(string FileName, llong FileBegin, llong Size)
{
    FileBufferTemp.resize(Size);
    for (llong i = 0; i < Size; i++)
    {
        FileBufferTemp[i] = 0;
    }
    if (ValidFileName(FileName))
    {
        fstream F(FileName.c_str(), ios::in|ios::binary);
        F.seekg(FileBegin);
        F.read((char*)FileBufferTemp.data(), FileBufferTemp.size());
        F.close();
    }
}

///
/// \brief FilePacket::SaveFragmentTo__OneFile - Save data to single file
/// \param FileName
/// \param FileBegin
///
void FilePacket::SaveFragmentTo__OneFile(string FileName, llong FileBegin)
{
    if (ValidFileName(FileName))
    {
        llong T = FileBufferTemp.size();
        fstream F(FileName.c_str(), ios::in|ios::out|ios::binary);
        F.seekp(FileBegin);

        F.write((char*)FileBufferTemp.data(), T);
        F.close();
    }
}

///
/// \brief FilePacket::CopyToTemp - Copy data buffer fragment to temporary buffer
/// \param Offset
/// \param Length
///
void FilePacket::CopyToTemp(llong Offset, llong Length)
{
    FileBufferTemp.resize(Length);
    memcpy(FileBufferTemp.data(), FileBuffer.data() + Offset, Length);
}

///
/// \brief FilePacket::CopyFromTemp - Copy temporary buffer to data buffer at the specified offset
/// \param Offset
///
void FilePacket::CopyFromTemp(llong Offset)
{
    memcpy(FileBuffer.data() + Offset, FileBufferTemp.data(), FileBufferTemp.size());
}

///
/// \brief FilePacket::LoadSaveFragment - Load or save specified data fragment
/// \param FileBegin
/// \param FileLength
/// \param Save
///
void FilePacket::LoadSaveFragment(llong FileBegin, llong FileLength, bool Save)
{
    if (!Save)
    {
        PrepareFragmentBuffer(FileLength);
    }

    llong BytesRead = 0;
    llong BytesToRead;
    llong BuffCounter = 0;
    llong CurrentFile = 0;
    llong FileCount = IdxFileName.size();

    while (!((IdxFileBegin[CurrentFile] <= FileBegin) && (IdxFileEnd[CurrentFile] >= FileBegin)))
    {
        CurrentFile++;
    }

    while ((BytesRead < FileLength) && (CurrentFile < FileCount))
    {
        if ((FileBegin + FileLength) > (IdxFileEnd[CurrentFile]))
        {
            BytesToRead = IdxFileEnd[CurrentFile] - (FileBegin + BytesRead) + 1;
        }
        else
        {
            BytesToRead = FileLength - BytesRead;
        }

        if (Save)
        {
            CopyToTemp(BuffCounter, BytesToRead);
            SaveFragmentTo__OneFile((IdxFileInclude[CurrentFile]) ? (WorkDir + IdxFileName[CurrentFile]) : "", (FileBegin + BytesRead) - IdxFileBegin[CurrentFile]);
        }
        else
        {
            LoadFragmentFromOneFile((IdxFileInclude[CurrentFile]) ? (WorkDir + IdxFileName[CurrentFile]) : "", (FileBegin + BytesRead) - IdxFileBegin[CurrentFile], BytesToRead);
            CopyFromTemp(BuffCounter);
        }

        BuffCounter = BuffCounter + BytesToRead;
        BytesRead = BytesRead + BytesToRead;
        CurrentFile = CurrentFile + 1;
    }
}

///
/// \brief FilePacket::PrepareFragmentBuffer - Prepare data buffer
/// \param FileLength
///
void FilePacket::PrepareFragmentBuffer(llong FileLength)
{
    FileBuffer.resize(FileLength);
    for (llong i = 0; i < FileLength; i++)
    {
        FileBuffer[i] = 0;
    }
}

///
/// \brief FilePacket::GetSize - Get the whole packet length
/// \param ChunkSize
/// \return
///
llong FilePacket::GetSize(llong ChunkSize)
{
    if (ChunkSize == 0)
    {
        return PacketSize;
    }
    llong X = PacketSize / ChunkSize;
    if ((PacketSize % ChunkSize))
    {
        X++;
    }
    return X;
}

string FilePacket::GetText(int N)
{
    string OutText = IdxFileName[N] + "\t" + Eden::ToStr(IdxFileSize[N]) + "\t" + Eden::ToStr(IdxFileBegin[N]) + "\t" + Eden::ToStr(IdxFileEnd[N]) + "\t";
    if (IdxFileDigest[N] != "X")
    {
        OutText += IdxFileDigest[N];
    }
    OutText += "\t";
    if (IdxFileInfo[N] != "")
    {
        OutText += IdxFileInfo[N];
    }
    return OutText;
}

int FilePacket::FileCount()
{
    return IdxFileName.size();
}

///
/// \brief FilePacket::CalcOffset - Calculate the bein offset and the ent offset for each file in packet based on file order and file sizes
///
void FilePacket::CalcOffset()
{
    llong T = 0;
    for (uint I = 0; I < IdxFileName.size(); I++)
    {
        IdxFileBegin[I] = T;
        T = T + IdxFileSize[I];
        IdxFileEnd[I] = T - 1;
    }
}

///
/// \brief FilePacket::CalcRange - Checking if some files are included in the specified data fragment
/// \param Range1 - Beginning byte
/// \param Range2 - Ending byte
///
void FilePacket::CalcRange(llong Range1, llong Range2)
{
    CalcOffset();
    for (uint I = 0; I < IdxFileName.size(); I++)
    {
        IdxFileInclude[I] = false;
        if ((IdxFileBegin[I] <= Range2) && (IdxFileEnd[I] >= Range1))
        {
            IdxFileInclude[I] = true;
        }
        else
        {
            if ((IdxFileBegin[I] >= Range2) && (IdxFileEnd[I] <= Range1))
            {
                IdxFileInclude[I] = true;
            }
        }
    }
}
