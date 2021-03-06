/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*            (c) 2006-2021 INRIA, USTL, UJF, CNRS, MGH, InSimo                *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                              SOFA :: Framework                              *
*                                                                             *
* Authors: The SOFA Team (see Authors.txt)                                    *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_HELPER_SYSTEM_FILEREPOSITORY_H
#define SOFA_HELPER_SYSTEM_FILEREPOSITORY_H

#include <sofa/SofaFramework.h>

#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <map>

namespace sofa
{

namespace helper
{

namespace system
{

/// Helper class to find files in a list of directories.
///
/// Each file is searched as follow:
///
/// 1: Using the specified filename in current directory, or in the specified directory.
/// If the filename does not start with "/", "./", or "../" :
/// 2: In the directory path specified using addFirstPath method.
/// 3: In the directory path specified using an environment variable (default to SOFA_DATA_PATH).
/// 4: In the directory path specified in a sofa.env file.
/// 5: In the default directories relative to the main executable (default to ../share).
/// 6: In the directory path specified using addLastPath method.
///
/// For file name starting with '/', './' or '../' only the first step is used.
///
/// A path is considered as a concatenation of directories separated by : on linux / mac and ; on windows
class SOFA_HELPER_API FileRepository
{
public:

    /// Initialize the set of paths from an environment variable.
    FileRepository(const char* envVar = "SOFA_DATA_PATH", const char* relativePath = 0);

    ~FileRepository();

    /// Return vector of strings result of spliting path with delimiter of OS.
    static std::vector<std::string> splitPath(const std::string& path);

	/// Replaces every occurrences of "//" by "/"
	static std::string cleanPath( const std::string& path );

    /// Adds a path to the front of the set of paths.
    void addFirstPath(const std::string& path);

    /// Adds a path to the back of the set of paths.
    void addLastPath(const std::string& path);

    /// Remove a path of the set of paths.
    void removePath(const std::string& path);

    /// Get the first path into the set of paths
    std::string getFirstPath();

    /// Returns a string such as refPath + string = path if path contains refPath.
    /// Otherwise returns path.
    /// Under WIN32 the method returns a lower cased unix formatted path.
    static std::string relativeToPath(std::string path, std::string refPath);

    /// Return concatenation of paths from:
    ///
    /// 1. given environment variable from execution environment (interpreted as relative to current directory)
    /// 2. entries with the same name as the environment variable from sofa.env in EnvRepository (interpreted as relative to each file) (DISABLED if envVar is SOFA_ENV_PATH)
    /// 3. default paths given in relativePaths (relative to the current executable)
    static std::vector< std::string > GetEnvPaths(const char* envVar, const char* relativePaths = 0);

    /// Return concatenation of items from:
    ///
    /// 1. given environment variable from execution environment
    /// 2. entries with the same name as the environment variable from sofa.env in EnvRepository (DISABLED if envVar is SOFA_ENV_PATH)
    /// 3. default items given in defaultItems
    static std::vector< std::string > GetEnvItems(const char* envVar, const char* defaultItems = 0);

    const std::vector< std::string > &getPaths() const {return vpath;}

    /// Find file using the stored set of paths, refering to the m_findFileFn method
    /// @param basedir override current directory (optional)
    /// @param filename requested file as input, resolved file path as output
    /// @return true if the file was found in one of the directories, false otherwise
    bool findFile(std::string& filename, const std::string& basedir = "", std::ostream* errlog = &std::cerr);

    /// Find file using the stored set of paths.
    /// @param basedir override current directory (optional)
    /// @param filename requested file as input, resolved file path as output
    /// @return true if the file was found in one of the directories, false otherwise
    bool findFileDefault(std::string& filename, const std::string& basedir = "", std::ostream* errlog = &std::cerr);

    /// Alias for findFile, but returning the resolved file as the result.
    /// Less informative for errors, but sometimes easier to use
    std::string getFile(std::string filename, const std::string& basedir="", std::ostream* errlog=&std::cerr)
    {
        findFile(filename, basedir, errlog);
        return filename;
    }

    /// Find file using the stored set of paths.
    /// @param basefile override current directory by using the parent directory of the given file
    /// @param filename requested file as input, resolved file path as output
    /// @return true if the file was found in one of the directories, false otherwise
    bool findFileFromFile(std::string& filename, const std::string& basefile, std::ostream* errlog=&std::cerr);

    /// Print the list of path to std::cout
    void print();


    /// OS-dependant character separing entries in list of paths.
    static constexpr char entrySeparator()
    {
#ifdef WIN32
        return ';';
#else
        return ':';
#endif
    }

    /// Display all current sofa search paths
    friend std::ostream& operator << (std::ostream& _flux, FileRepository _fr)
    {
        _flux<< "FileRepository vpath :"<<std::endl;
        for(std::vector<std::string>::iterator it = _fr.vpath.begin(); it!=_fr.vpath.end(); it++)
            _flux<<(*it)<<std::endl;

        return _flux;
    }

    void displayPaths() {std::cout<<(*this)<<std::endl;}


    bool getFileContentDefault(const std::string& filename, std::string& filecontent, bool isBinaryFile = false, std::ostream* errlog = &std::cerr);

    inline bool getFileContent(const std::string& filename, std::string& filecontent, bool isBinaryFile = false, std::ostream* errlog = &std::cerr)
    {
        return m_getFileContentFn(filename, filecontent, isBinaryFile, errlog);
    }

    inline void setFileContentFn(std::function < bool(const std::string& filename, std::string& filecontent, bool isBinary, std::ostream* errlog ) >  f)
    {
        m_getFileContentFn = f;
    }

    inline void setFindFileFn(std::function < bool(std::string&, const std::string&, std::ostream*) >  f)
    {
        m_findFileFn = f;
    }
protected:

    /// Vector of paths.
    std::vector<std::string> vpath;

    /// method used to get files
    std::function < bool(const std::string& filename, std::string& filecontent, bool isBinary, std::ostream* errlog) > m_getFileContentFn;

    /// method used to get files
    std::function < bool(std::string&, const std::string& , std::ostream*) > m_findFileFn;

    /// Search file in a given path.
    static bool findFileIn(std::string& filename, const std::string& path);
};

extern SOFA_HELPER_API FileRepository DataRepository; ///< Data search repository
extern SOFA_HELPER_API FileRepository PluginRepository; ///< Plugin search repository
extern SOFA_HELPER_API FileRepository EnvRepository; ///< Env files repository

} // namespace system

} // namespace helper

} // namespace sofa

#endif
