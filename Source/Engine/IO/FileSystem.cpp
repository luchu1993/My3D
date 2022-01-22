//
// Created by luchu on 2022/1/13.
//

#include "Core/Context.h"
#include "Core/Thread.h"
#include "IO/FileSystem.h"
#include "IO/File.h"
#include "IO/Log.h"
#include "IO/IOEvents.h"
#include "Core/CoreEvents.h"

#include <sys/stat.h>
#include <cstdio>


namespace My3D
{

FileSystem::FileSystem(Context *context)
    : Object(context)
{
    SubscribeToEvent(E_BEGINFRAME, MY3D_HANDLER(FileSystem, HandleBeginFrame));
    // Subscribe to console command
    SetExecuteConsoleCommands(true);
}

FileSystem::~FileSystem()
{

}

void FileSystem::SetExecuteConsoleCommands(bool enable)
{
    if (enable == executeConsoleCommands_)
        return;

    executeConsoleCommands_ = enable;
}

void FileSystem::HandleBeginFrame(StringHash eventType, VariantMap &eventData)
{

}

bool FileSystem::CheckAccess(const String &pathName) const
{
    String fixedPath = AddTrailingSlash(pathName);

    // If no allowed directories defined, succeed always
    if (allowedPaths_.Empty())
        return true;

    // If there is any attempt to go to a parent directory, disallow
    if (fixedPath.Contains(".."))
        return false;

    // Check if the path is a partial match of any of the allowed directories
    for (HashSet<String>::ConstIterator i = allowedPaths_.Begin(); i != allowedPaths_.End(); ++i)
    {
        if (fixedPath.Find(*i) == 0)
            return true;
    }

    // Not found, so disallow
    return false;
}

String GetInternalPath(const String& pathName)
{
    return pathName.Replaced('\\', '/');
}

String GetNativePath(const String& pathName)
{
#ifdef _WIN32
    return pathName.Replaced('/', '\\');
#else
    return pathName;
#endif
}

void SplitPath(const String& fullPath, String& pathName, String& fileName, String& extension, bool lowercaseExtension)
{
    String fullPathCopy = GetInternalPath(fullPath);

    unsigned extPos = fullPathCopy.FindLast('.');
    unsigned pathPos = fullPathCopy.FindLast('/');

    if (extPos != String::NPOS && (pathPos == String::NPOS || extPos > pathPos))
    {
        extension = fullPathCopy.Substring(extPos);
        if (lowercaseExtension)
            extension = extension.ToLower();
        fullPathCopy = fullPathCopy.Substring(0, extPos);
    }
    else
        extension.Clear();

    pathPos = fullPathCopy.FindLast('/');
    if (pathPos != String::NPOS)
    {
        fileName = fullPathCopy.Substring(pathPos + 1);
        pathName = fullPathCopy.Substring(0, pathPos + 1);
    }
    else
    {
        fileName = fullPathCopy;
        pathName.Clear();
    }
}

String GetPath(const String& fullPath)
{
    String path, file, extension;
    SplitPath(fullPath, path, file, extension);
    return path;
}

String GetFileName(const String& fullPath)
{
    String path, file, extension;
    SplitPath(fullPath, path, file, extension);
    return file;
}

String GetExtension(const String& fullPath, bool lowercaseExtension)
{
    String path, file, extension;
    SplitPath(fullPath, path, file, extension, lowercaseExtension);
    return extension;
}

String AddTrailingSlash(const String& pathName)
{
    String ret = pathName.Trimmed();
    ret.Replace('\\', '/');
    if (!ret.Empty() && ret.Back() != '/')
        ret += '/';
    return ret;
}

WString GetWideNativePath(const String& pathName)
{
#ifdef _WIN32
    return WString(pathName.Replaced('/', '\\'));
#else
    return WString(pathName);
#endif
}

bool IsAbsolutePath(const String& pathName)
{
    if (pathName.Empty())
        return false;

    String path = GetInternalPath(pathName);

    if (path[0] == '/')
        return true;

#ifdef _WIN32
    if (path.Length() > 1 && IsAlpha(path[0]) && path[1] == ':')
        return true;
#endif

    return false;
}

}