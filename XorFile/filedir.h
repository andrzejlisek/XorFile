#ifndef FILEDIR_H
#define FILEDIR_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

namespace EdenClass
{
    class FileDir
    {
    public:
        FileDir();
        static string CorrectDir(string DirPath, char DirSepChar);
        static char DirGetSeparator(string DirPath);
        static vector<string> DirPathToArray(string DirPath, char DirSepChar);
        static string DirArrayToPath(vector<string> DirPath, string Sep);
        static string DirAbsoluteToRelative(string BaseDir, string AbsoluteDir);
        static string DirRelativeToAbsolute(string BaseDir, string RelativeDir);
    };
}

#endif // FILEDIR_H
