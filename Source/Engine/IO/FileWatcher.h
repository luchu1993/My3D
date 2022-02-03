//
// Created by luchu on 2022/2/3.
//

#pragma once

#include "Core/Mutex.h"
#include "Core/Object.h"
#include "Core/Thread.h"
#include "Core/Timer.h"


namespace My3D
{
    class FileSystem;

    /// Watches a directory and its subdirectories for files being modified.
    class MY3D_API FileWatcher : public Object, public Thread
    {
        MY3D_OBJECT(FileWatcher, Object)

    public:
        /// Construct
        explicit FileWatcher(Context* context);
        /// Destruct
        ~FileWatcher() override;

        /// Directory watching loop
        void ThreadFunction() override;

        /// Start watching a directory. Return true if successful
        bool StartWatching(const String& pathName, bool watchSubDirs);
        /// Stop watching the directory.
        void StopWatching();
        /// Set the delay in seconds before file changes are notified. This (hopefully) avoids notifying when a file save is still in progress. Default 1 second.
        void SetDelay(float interval);
        /// Add a file change into the changes queue.
        void AddChange(const String& fileName);
        /// Return a file change (true if was found, false if not).
        bool GetNextChange(String& dest);
        /// Return the path being watched, or empty if not watching.
        const String& GetPath() const { return path_; }
        /// Return the delay in seconds for notifying file changes.
        float GetDelay() const { return delay_; }

    private:
        /// Filesystem
        SharedPtr<FileSystem> fileSystem_;
        /// The path being watched
        String path_;
        /// Pending changes.  These will be returned and removed from the list when their timer has exceeded the delay.
        HashMap<String, Timer> changes_;
        /// Mutex for the change buffer
        Mutex changesMutex_;
        /// Delay int seconds for notifying changes
        float delay_;
        /// Watch subdirectories flag
        bool watchSubDirs_;

#ifdef _WIN32
        /// Directory handle for the path being watched.
        void* dirHandle_;
#elif __linux__
        /// HashMap for the directory and sub-directories (needed for inotify's int handles).
        HashMap<int, String> dirHandle_;
        /// Linux inotify needs a handle.
        int watchHandle_;
#endif
    };
}
