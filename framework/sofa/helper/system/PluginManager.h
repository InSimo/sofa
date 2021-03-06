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
#ifndef SOFA_HELPER_SYSTEM_PLUGINMANAGER_H
#define SOFA_HELPER_SYSTEM_PLUGINMANAGER_H

#include <sofa/helper/system/DynamicLibrary.h>
#include <memory>
#include <map>
#include <sofa/SofaFramework.h>

namespace sofa
{
namespace helper
{
namespace system
{
class PluginManager;

class SOFA_HELPER_API Plugin
{
    friend class PluginManager;
public:
    typedef struct InitExternalModule
    {
        static const char* symbol;
        typedef void (*FuncPtr) ();
        FuncPtr func;
        void operator() ()
        {
            if (func) return func();
        }
        InitExternalModule():func(0) {}
    } InitExternalModule;

    typedef struct GetModuleName
    {
        static const char* symbol;
        typedef const char* (*FuncPtr) ();
        FuncPtr func;
        const char* operator() () const
        {
            if (func) return func();
            else return NULL;
        }
        GetModuleName():func(0) {}
    } GetModuleName;

    typedef struct GetModuleDescription
    {
        static const char* symbol;
        typedef const char* (*FuncPtr) ();
        FuncPtr func;
        const char* operator() () const
        {
            if (func) return func();
            else return NULL;
        }
        GetModuleDescription():func(0) {}
    } GetModuleDescription;

    typedef struct GetModuleLicense
    {
        static const char* symbol;
        typedef const char* (*FuncPtr) ();
        FuncPtr func;
        const char* operator() () const
        {
            if (func) return func();
            else return NULL;
        }

        GetModuleLicense():func(0) {}
    } GetModuleLicense;

    typedef struct GetModuleComponentList
    {
        static  const char* symbol;
        typedef const char* (*FuncPtr) ();
        FuncPtr func;
        const char* operator() () const
        {
            if (func) return func();
            else return NULL;
        }
        GetModuleComponentList():func(0) {}
    } GetModuleComponentList;

    typedef struct GetModuleVersion
    {
        static const char* symbol;
        typedef const char* (*FuncPtr) ();
        FuncPtr func;
        const char* operator() () const
        {
            if (func) return func();
            else return NULL;
        }
        GetModuleVersion():func(0) {}
    } GetModuleVersion;

    InitExternalModule     initExternalModule;
    GetModuleName          getModuleName;
    GetModuleDescription   getModuleDescription;
    GetModuleLicense       getModuleLicense;
    GetModuleComponentList getModuleComponentList;
    GetModuleVersion       getModuleVersion;
private:
    DynamicLibrary::Handle dynamicLibrary;

};

class SOFA_HELPER_API PluginManager
{
public:
    typedef std::map<std::string, Plugin > PluginMap;
    typedef PluginMap::iterator PluginIterator;

    static PluginManager& getInstance();

    /// Get the default suffix applied to plugin names to find the actual lib to load
    /// (depends on platform, version, debug/release build)
    static std::string getDefaultSuffix();

    /// Search for a plugin given a path or a name (if no path or extension specified, in which case
    /// the provided suffix will be added)
    /// On return the full path of the plugin is written into the path parameter
    ///
    bool findPlugin(std::string& path, const std::string& suffix = getDefaultSuffix(), std::ostream* errlog=&std::cerr);

    /// Check if a plugin is loaded given a path or a name (if no path or extension specified, in which case
    /// the default suffix will be added)
    /// On return the full path of the plugin is written into the path parameter
    bool hasPlugin(std::string& path, bool finalPath = false);

    /// Load a plugin given a path or a name (if finalPath is false and no path or extension is
    /// specified, in which case the default suffix will be added)
    /// On return the full path of the plugin is written into the path parameter
    ///
    /// CHANGE: it is no longer an error to call loadPlugin() on an already-loaded plugin
    /// this was creating annoying error messages and complex logic to avoid it
    ///
    bool loadPlugin(std::string& path, std::ostream* errlog=&std::cerr, bool finalPath = false);

    /// Unload a plugin given a path or a name (if no path or extension specified, in which case
    /// the default suffix will be added)
    /// On return the full path of the plugin is written into the path parameter
    bool unloadPlugin(std::string& path, std::ostream* errlog=&std::cerr, bool finalPath = false);

    void initRecentlyOpened();
    void init();
	void init(const std::string& pluginName);

    inline friend std::ostream& operator<< ( std::ostream& os, const PluginManager& pluginManager )
    {
        return pluginManager.writeToStream( os );
    }
    inline friend std::istream& operator>> ( std::istream& in, PluginManager& pluginManager )
    {
        return pluginManager.readFromStream( in );
    }
    PluginMap& getPluginMap()  { return m_pluginMap; }


    void readFromIniFile();
    void writeToIniFile();

private:
    PluginManager() {}
    ~PluginManager();
    PluginManager(const PluginManager& );
    DynamicLibrary* loadLibrary(const std::string& path,  std::ostream* errlog=&std::cerr);
    std::ostream& writeToStream( std::ostream& ) const;
    std::istream& readFromStream( std::istream& );
private:
    PluginMap m_pluginMap;
};


}

}

}

#endif //SOFA_HELPER_SYSTEM_PLUGINMANAGER_H

