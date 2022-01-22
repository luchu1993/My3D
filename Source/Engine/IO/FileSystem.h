//
// Created by luchu on 2022/1/13.
//

#pragma once

#include "Core/Object.h"
#include "Container/HashSet.h"


namespace My3D
{

class MY3D_API FileSystem : public Object
{
    MY3D_OBJECT(FileSystem, Object);
public:
    /// Construct
    explicit FileSystem(Context* context);
    /// Destruct
    ~FileSystem() override;
    /// Set whether to execute engine console commands as OS-specific system command.
    void SetExecuteConsoleCommands(bool enable);
    /// Check if a path is allowed to be accessed. If no paths are registered, all are allowed.
    bool CheckAccess(const String& pathName) const;

private:
    /// Handle begin frame event to check for completed async executions.
    void HandleBeginFrame(StringHash eventType, VariantMap& eventData);
    /// Handle a console command event.
    void HandleConsoleCommand(StringHash eventType, VariantMap& eventData);

    /// Allowed directories.
    HashSet<String> allowedPaths_;
    /// Flag for executing engine console commands as OS-specific system command. Default to true.
    bool executeConsoleCommands_{};
};

/// Split a full path to path, filename and extension. The extension will be converted to lowercase by default.
MY3D_API void SplitPath(const String& fullPath, String& pathName, String& fileName, String& extension, bool lowercaseExtension = true);
/// Return the path from a full path.
MY3D_API String GetPath(const String& fullPath);
/// Return the filename from a full path.
MY3D_API String GetFileName(const String& fullPath);
/// Return the extension from a full path, converted to lowercase by default.
MY3D_API String GetExtension(const String& fullPath, bool lowercaseExtension = true);
/// Add a slash at the end of the path if missing and convert to internal format (use slashes).
MY3D_API String AddTrailingSlash(const String& pathName);
/// Remove the slash from the end of a path if exists and convert to internal format (use slashes).
MY3D_API String RemoveTrailingSlash(const String& pathName);
/// Return the parent path, or the path itself if not available.
MY3D_API String GetParentPath(const String& path);
/// Convert a path to internal format (use slashes).
MY3D_API String GetInternalPath(const String& pathName);
/// Convert a path to the format required by the operating system.
MY3D_API String GetNativePath(const String& pathName);
/// Convert a path to the format required by the operating system in wide characters.
MY3D_API WString GetWideNativePath(const String& pathName);
/// Return whether a path is absolute.
MY3D_API bool IsAbsolutePath(const String& pathName);
}
