/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*                (c) 2006-2011 INRIA, USTL, UJF, CNRS, MGH                    *
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
#include <sofa/helper/system/FileRepository.h>
#include <sofa/helper/system/SetDirectory.h>

#include <sys/types.h>
#include <sys/stat.h>
#if defined(WIN32)
#include <windows.h>
#include <direct.h>
#elif defined(_XBOX)
#include <xtl.h>
#else
#include <unistd.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <streambuf>

namespace sofa
{

namespace helper
{

namespace system
{
// replacing every occurences of "//"  by "/"
std::string cleanPath( const std::string& path )
{
    std::string p = path;
    size_t pos = p.find("//");
	size_t len = p.length();
    while( pos != std::string::npos )
    {
		if ( pos == (len-1))
			p.replace( pos, 2, "");
		else
			p.replace(pos,2,"/");
        pos = p.find("//");
    }
    return p;
}

#if defined (WIN32) || defined (_XBOX)
FileRepository EnvRepository("SOFA_ENV_PATH", "./sofa.env;../sofa.env;../../sofa.env";
FileRepository DataRepository("SOFA_DATA_PATH", "../share;../examples;../share/sofa;../share/sofa/examples");
FileRepository PluginRepository("SOFA_PLUGIN_PATH","./");
#else
FileRepository EnvRepository("SOFA_ENV_PATH", "sofa.env:../sofa.env:../../sofa.env");
FileRepository DataRepository("SOFA_DATA_PATH", "../share:../examples:../share/sofa:../share/sofa/examples");
FileRepository PluginRepository("SOFA_PLUGIN_PATH","../lib");
#endif

FileRepository::FileRepository(const char* envVar, const char* relativePath)
{
    if (envVar == "SOFA_ENV_PATH")
    {
        std::vector<std::string> file_env_paths = splitPath(relativePath);

        for (unsigned int i = 0; i < file_env_paths.size(); i++)
        {
            std::string base;
            base = SetDirectory::GetProcessFullPath(base.c_str());
            std::size_t found = base.find_last_of("/\\");
            base = base.substr(0,found);
            if (findFileIn(file_env_paths[i], base))
            {
                std::string newfname = SetDirectory::GetRelativeFromDir(file_env_paths[i].c_str(), base.c_str());
                std::ifstream infile(newfname);
                std::string line;
                while (std::getline(infile, line))
                {
                    std::size_t pos = line.find("=");
                    std::string env = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);

                    std::map<std::string,std::vector<std::string>>::iterator mapIt = envpath.find(env);
                    if ( mapIt == envpath.end() )
                    {
                        std::vector<std::string> toInsert{value};
                        envpath.insert ( std::pair<std::string,std::vector<std::string>>(env,toInsert) );
                    }
                    else
                    {
                        mapIt->second.push_back(value);
                    }
                }
            }
        }
    }
    else
    {
        if (envVar != NULL && envVar[0]!='\0')
        {
            const char* envpath = getenv(envVar);
            if (envpath != NULL && envpath[0]!='\0')
                addFirstPath(envpath);
        }
        if (relativePath != NULL && relativePath[0]!='\0')
        {
            std::vector<std::string> paths = splitPath(relativePath);
            for (const auto& p : paths)
            {
                addLastPath(SetDirectory::GetRelativeFromProcess(p.c_str()));
            }
        }
        std::vector<std::string> env = EnvRepository.getEnvPaths(envVar);
        for (const auto& e : env)
        {
            addLastPath(SetDirectory::GetRelativeFromProcess(e.c_str()));
        }

        m_getFileContentFn = [this](const std::string& filename, std::string& filecontent, bool isBinary, std::ostream* errlog)
        {
            return this->getFileContentDefault(filename, filecontent, isBinary, errlog);
        };
        m_findFileFn = [this](std::string& filename, const std::string basedir, std::ostream* errlog)
        {
            return this->findFileDefault(filename, basedir, errlog);
        };
    }
}

FileRepository::~FileRepository()
{
}

std::vector<std::string> FileRepository::splitPath(const std::string& path)
{
    std::vector<std::string> result;
    size_t p0 = 0;
    while ( p0 < path.size() )
    {
        size_t p1 = path.find(entrySeparator(),p0);
        if (p1 == std::string::npos) p1 = path.size();
        if (p1>p0+1)
        {
            result.push_back(path.substr(p0,p1-p0));
        }
        p0 = p1+1;
    }
    return result;
}

std::string FileRepository::cleanPath( const std::string& path )
{
	std::string p = path;
	size_t pos = p.find("//");
	size_t len = p.length();
	while( pos != std::string::npos )
	{
		if ( pos == (len-2))
			p.replace( pos, 2, "");
		else
			p.replace(pos,2,"/");
		pos = p.find("//");
	}
	return p;
}

void FileRepository::addFirstPath(const std::string& p)
{
    // replacing every occurences of "//" by "/"
    std::string path = cleanPath( p );

    std::vector<std::string> entries;
    size_t p0 = 0;
    while ( p0 < path.size() )
    {
        size_t p1 = path.find(entrySeparator(),p0);
        if (p1 == std::string::npos) p1 = path.size();
        if (p1>p0+1)
        {
            entries.push_back(path.substr(p0,p1-p0));
        }
        p0 = p1+1;
    }
    vpath.insert(vpath.begin(), entries.begin(), entries.end());
}

void FileRepository::addLastPath(const std::string& p)
{
    // replacing every occurences of "//" by "/"
    std::string path = cleanPath( p );

    std::vector<std::string> entries;
    size_t p0 = 0;
    while ( p0 < path.size() )
    {
        size_t p1 = path.find(entrySeparator(),p0);
        if (p1 == std::string::npos) p1 = path.size();
        if (p1>p0+1)
        {
            entries.push_back(path.substr(p0,p1-p0));
        }
        p0 = p1+1;
    }
    vpath.insert(vpath.end(), entries.begin(), entries.end());
//     std::cout << path << std::endl;
}

void FileRepository::removePath(const std::string& path)
{
    std::vector<std::string> entries;
    size_t p0 = 0;
    while ( p0 < path.size() )
    {
        size_t p1 = path.find(entrySeparator(),p0);
        if (p1 == std::string::npos) p1 = path.size();
        if (p1>p0+1)
        {
            entries.push_back(path.substr(p0,p1-p0));
        }
        p0 = p1+1;
    }

    for(std::vector<std::string>::iterator it=entries.begin();
        it!=entries.end(); ++it)
    {
        vpath.erase( find(vpath.begin(), vpath.end(), *it) );
    }

    // Display
    // std::cout<<(*this)<<std::endl;
}

std::string FileRepository::getFirstPath()
{
    if (vpath.size() > 0)
        return vpath.front();
    else return "";
}

bool FileRepository::findFileIn(std::string& filename, const std::string& path)
{
    if (filename.empty()) return false; // no filename
    struct stat s;
    std::string newfname = SetDirectory::GetRelativeFromDir(filename.c_str(), path.c_str());
    //std::cout << "Looking for " << newfname <<std::endl;
    if (!stat(newfname.c_str(),&s))
    {
        // File found
        //std::cout << "File "<<filename<<" found in "<<path.substr(p0,p1-p0)<<std::endl;
        filename = newfname;
        return true;
    }
    return false;
}

bool FileRepository::findFile(std::string& filename, const std::string& basedir, std::ostream* errlog)
{
    return m_findFileFn(filename, basedir, errlog);
}

bool FileRepository::findFileDefault(std::string& filename, const std::string& basedir, std::ostream* errlog)
{
    if (filename.empty()) return false; // no filename
    std::string currentDir = SetDirectory::GetCurrentDir();
    if (!basedir.empty())
    {
        currentDir = SetDirectory::GetRelativeFromDir(basedir.c_str(),currentDir.c_str());
    }
    if (findFileIn(filename, currentDir)) return true;

    if (SetDirectory::IsAbsolute(filename)) return false; // absolute file path
    if (filename.substr(0,2)=="./" || filename.substr(0,3)=="../")
    {
        // update filename with current dir
        filename = SetDirectory::GetRelativeFromDir(filename.c_str(), currentDir.c_str());
        return false; // local file path
    }
    for (std::vector<std::string>::const_iterator it = vpath.begin(); it != vpath.end(); ++it)
        if (findFileIn(filename, *it)) return true;
    if (errlog)
    {
        (*errlog) << "File "<<filename<<" NOT FOUND in "<<basedir;
        for (std::vector<std::string>::const_iterator it = vpath.begin(); it != vpath.end(); ++it)
            (*errlog) << ':'<<*it;
        (*errlog)<<std::endl;
    }
    return false;
}

bool FileRepository::findFileFromFile(std::string& filename, const std::string& basefile, std::ostream* errlog)
{
    return findFile(filename, SetDirectory::GetParentDir(basefile.c_str()), errlog);
}

bool FileRepository::getFileContentDefault(const std::string& filename, std::string& filecontent, bool isBinaryFile, std::ostream* errlog)
{
    filecontent.clear();
    std::ifstream file(filename.c_str(), isBinaryFile ? std::ifstream::in | std::ifstream::binary : std::ifstream::in);
    filecontent = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    bool result = file.good();
    file.close();
    return result;
}

void FileRepository::print()
{
    for (std::vector<std::string>::const_iterator it = vpath.begin(); it != vpath.end(); ++it)
        std::cout << *it << std::endl;
}
/*static*/
std::string FileRepository::relativeToPath(std::string path, std::string refPath)
{
#ifdef WIN32

    /*
    WIN32 is a pain here because of mixed case formatting with randomly
    picked slash and backslash to separate dirs.
    */
    std::replace(path.begin(),path.end(),'\\' , '/' );
    std::replace(refPath.begin(),refPath.end(),'\\' , '/' );
    std::transform(path.begin(), path.end(), path.begin(), ::tolower );
    std::transform(refPath.begin(), refPath.end(), refPath.begin(), ::tolower );

#endif
    std::string::size_type loc = path.find( refPath, 0 );
    if (loc==0) path = path.substr(refPath.size()+1);

    return path;


}

} // namespace system

} // namespace helper

} // namespace sofa

