#include "filedir.h"

namespace EdenClass
{
    FileDir::FileDir()
    {
    }

    ///
    /// \brief FileDir::CorrectDir - Correct path by appending separator at the end if the separator does not exist
    /// \param DirPath
    /// \param DirSepChar
    /// \return
    ///
    string FileDir::CorrectDir(string DirPath, char DirSepChar)
    {
        if (DirPath[DirPath.length() - 1] != DirSepChar)
        {
            return DirPath + string(1, DirSepChar);
        }
        else
        {
            return DirPath;
        }
    }

    ///
    /// \brief FileDir::DirGetSeparator - Detect the separator character based on counting the '/' and '\' characters
    /// \param DirPath
    /// \return
    ///
    char FileDir::DirGetSeparator(string DirPath)
    {
        int C1 = 0;
        int C2 = 0;
        for (unsigned int i = 0; i < DirPath.length(); i++)
        {
            if (DirPath[i] == '/') { C1++; }
            if (DirPath[i] == '\\') { C2++; }
        }
        if ((DirPath.length() == 2) && (DirPath[1] == ':'))
        {
            return '\\';
        }
        if (C2 > C1)
        {
            return '\\';
        }
        else
        {
            return '/';
        }
    }

    ///
    /// \brief FileDir::DirPathToArray - Convert the path to the directory array
    /// \param DirPath
    /// \return
    ///
    vector<string> FileDir::DirPathToArray(string DirPath, char DirSepChar)
    {
        vector<string> Elems;
        string Temp = "";
        unsigned int L = DirPath.length();
        for (unsigned int I = 0; I < L; I++)
        {
            if (DirPath[I] == DirSepChar)
            {
                Elems.push_back(Temp);
                Temp = "";
            }
            else
            {
                Temp = Temp + string(1, DirPath[I]);
            }
        }
        Elems.push_back(Temp);
        return Elems;
    }

    ///
    /// \brief FileDir::DirArrayToPath - Convert the directory array to the path
    /// \param DirPath
    /// \param Sep
    /// \return
    ///
    string FileDir::DirArrayToPath(vector<string> DirPath, string Sep)
    {
        string X = "";
        for (unsigned int i = 0; i < DirPath.size(); i++)
        {
            if (i > 0)
            {
                X = X + Sep;
            }
            X = X + DirPath[i];
        }
        return X;
    }

    ///
    /// \brief FileDir::DirAbsoluteToRelative - Change the absolute path to the relative path
    /// \param BaseDir
    /// \param AbsoluteDir
    /// \return
    ///
    string FileDir::DirAbsoluteToRelative(string BaseDir, string AbsoluteDir)
    {
        char DirSepChar = DirGetSeparator(BaseDir + AbsoluteDir);

        vector<string> BaseDir0 = DirPathToArray(FileDir::CorrectDir(BaseDir, DirSepChar), DirSepChar);
        int BaseDirL = BaseDir0.size();
        vector<string> AbsoluteDir0 = DirPathToArray(AbsoluteDir, DirSepChar);
        int AbsoluteDirL = AbsoluteDir0.size();

        // Detecting the common part
        int CommonL = BaseDirL;
        if (AbsoluteDirL < CommonL)
        {
            CommonL = AbsoluteDirL;
        }
        int DirSep = 0;
        for (int i = 0; i < CommonL; i++)
        {
            if (BaseDir0[i] != AbsoluteDir0[i])
            {
                DirSep = i;
                i = CommonL;
            }
        }

        vector<string> Dir0;

        // Appending the upper directory symbols
        if (DirSep > 0)
        {
            for (int i = DirSep + 1; i < BaseDirL; i++)
            {
                Dir0.push_back("..");
            }
        }

        // Appending the subdirectory names
        for (int i = DirSep; i < AbsoluteDirL; i++)
        {
            Dir0.push_back(AbsoluteDir0[i]);
        }

        return DirArrayToPath(Dir0, string(1, DirSepChar).c_str());
    }

    ///
    /// \brief FileDir::DirRelativeToAbsolute - Change the relative path to the absolute path
    /// \param BaseDir
    /// \param RelativeDir
    /// \return
    ///
    string FileDir::DirRelativeToAbsolute(string BaseDir, string RelativeDir)
    {
        char DirSepChar = DirGetSeparator(BaseDir + RelativeDir);

        vector<string> BaseDir0 = DirPathToArray(FileDir::CorrectDir(BaseDir, DirSepChar), DirSepChar);
        int BaseDirL = BaseDir0.size();
        vector<string> RelativeDir0 = DirPathToArray(RelativeDir, DirSepChar);
        int RelativeDirL = RelativeDir0.size();


        // If the first element of the base directory and relative path differs and there is a conol (the Windows path convention),
        // then the relative path is not relative against the base dir
        if (RelativeDir0[0] != BaseDir0[0])
        {
            if (RelativeDir0[0].length() >= 2)
            {
                if (RelativeDir0[0][1] == ':')
                {
                    return RelativeDir;
                }
            }
        }

        // Counting the ".." in relative path
        int UpDirCounter = 0;
        int I = 0;
        while ((RelativeDirL > I) && (RelativeDir0[I] == ".."))
        {
            UpDirCounter++;
            I++;
        }

        // Creating shortened base directory
        vector<string> BaseDir00;
        BaseDir00.resize(BaseDirL - UpDirCounter);
        for (int i = 0; i < BaseDirL - UpDirCounter - 1; i++)
        {
            BaseDir00[i] = BaseDir0[i];
        }

        // Creating shortened relative directory
        vector<string> RelativeDir00;
        RelativeDir00.resize(RelativeDirL - UpDirCounter);
        for (int i = 0; i < RelativeDirL - UpDirCounter; i++)
        {
            RelativeDir00[i] = RelativeDir0[i + UpDirCounter];
        }

        string BaseDirX = DirArrayToPath(BaseDir00, string(1, DirSepChar));
        string RelativeDirX = DirArrayToPath(RelativeDir00, string(1, DirSepChar));

        return BaseDirX + RelativeDirX;
    }
}
