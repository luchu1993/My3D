//
// Created by luchu on 2022/2/3.
//

#include "IO/FileWatcher.h"
#include "IO/File.h"
#include "IO/Log.h"
#include "IO/FileSystem.h"


#ifdef _WIN32
#include <windows.h>
#else
#include <sys/inotify.h>
extern "C"
{
// Need read/close for inotify
#include "unistd.h"
}
#endif


namespace My3D
{
    static const unsigned BUFFERSIZE = 4096;

    FileWatcher::FileWatcher(Context *context)
        : Object(context)
        , fileSystem_(GetSubsystem<FileSystem>())
        , delay_(1.0f)
        , watchSubDirs_(false)
    {
    }

    FileWatcher::~FileWatcher()
    {
        StopWatching();
    }

    bool FileWatcher::StartWatching(const String &pathName, bool watchSubDirs)
    {
        if (!fileSystem_)
        {
            MY3D_LOGERROR("No FileSystem, can not start watching");
            return false;
        }

        // Stop any previous watching
        StopWatching();

#ifdef _WIN32
        String nativePath = GetNativePath(RemoveTrailingSlash(pathName));
        dirHandle_ = (void*) CreateFileW(
                WString(nativePath).CString(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS,
                nullptr);
        if (dirHandle_ != INVALID_HANDLE_VALUE)
        {
            path_ = AddTrailingSlash(pathName);
            watchSubDirs_ = watchSubDirs;
            Run();

            MY3D_LOGDEBUG("Started watching path " + pathName);
            return true;
        }
        else
        {
            MY3D_LOGDEBUG("Failed to start watching path " + pathName);
            return false;
        }
#else
        int flags = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO;
    int handle = inotify_add_watch(watchHandle_, pathName.CString(), (unsigned)flags);

    if (handle < 0)
    {
        MY3D_LOGERROR("Failed to start watching path " + pathName);
        return false;
    }
    else
    {
        // Store the root path here when reconstructed with inotify later
        dirHandle_[handle] = "";
        path_ = AddTrailingSlash(pathName);
        watchSubDirs_ = watchSubDirs;

        if (watchSubDirs_)
        {
            Vector<String> subDirs;
            fileSystem_->ScanDir(subDirs, pathName, "*", SCAN_DIRS, true);

            for (unsigned i = 0; i < subDirs.Size(); ++i)
            {
                String subDirFullPath = AddTrailingSlash(path_ + subDirs[i]);

                // Don't watch ./ or ../ sub-directories
                if (!subDirFullPath.EndsWith("./"))
                {
                    handle = inotify_add_watch(watchHandle_, subDirFullPath.CString(), (unsigned)flags);
                    if (handle < 0)
                        MY3D_LOGERROR("Failed to start watching subdirectory path " + subDirFullPath);
                    else
                    {
                        // Store sub-directory to reconstruct later from inotify
                        dirHandle_[handle] = AddTrailingSlash(subDirs[i]);
                    }
                }
            }
        }
        Run();

        MY3D_LOGDEBUG("Started watching path " + pathName);
        return true;
    }
#endif
    }

    void FileWatcher::StopWatching()
    {
        if (handle_)
        {
            shouldRun_ = false;

#ifdef _WIN32
            String dummyFileName = path_ + "dummy.tmp";
            File file(context_, dummyFileName, FILE_WRITE);
            file.Close();
            if (fileSystem_)
                fileSystem_->Delete(dummyFileName);
            CloseHandle((HANDLE) dirHandle_);
#endif
            Stop();

            MY3D_LOGDEBUG("Stopped watching path " + path_);
            path_.Clear();
        }
    }

    void FileWatcher::SetDelay(float interval)
    {
        delay_ = Max(interval, 0.0f);
    }

    void FileWatcher::ThreadFunction()
    {
#ifdef _WIN32
        unsigned char buffer[BUFFERSIZE];
        DWORD bytesFilled = 0;

        while (shouldRun_)
        {
            if (ReadDirectoryChangesW((HANDLE)dirHandle_,
                                      buffer,
                                      BUFFERSIZE,
                                      watchSubDirs_,
                                      FILE_NOTIFY_CHANGE_FILE_NAME |
                                      FILE_NOTIFY_CHANGE_LAST_WRITE,
                                      &bytesFilled,
                                      nullptr,
                                      nullptr))
            {
                unsigned offset = 0;

                while (offset < bytesFilled)
                {
                    FILE_NOTIFY_INFORMATION* record = (FILE_NOTIFY_INFORMATION*)&buffer[offset];

                    if (record->Action == FILE_ACTION_MODIFIED || record->Action == FILE_ACTION_RENAMED_NEW_NAME)
                    {
                        String fileName;
                        const wchar_t* src = record->FileName;
                        const wchar_t* end = src + record->FileNameLength / 2;
                        while (src < end)
                            fileName.AppendUTF8(String::DecodeUTF16(src));

                        fileName = GetInternalPath(fileName);
                        AddChange(fileName);
                    }

                    if (!record->NextEntryOffset)
                        break;
                    else
                        offset += record->NextEntryOffset;
                }
            }
        }
#else
        unsigned char buffer[BUFFERSIZE];

        while (shouldRun_)
        {
            int i = 0;
            auto length = (int)read(watchHandle_, buffer, sizeof(buffer));

            if (length < 0)
                return;

            while (i < length)
            {
                auto* event = (inotify_event*)&buffer[i];

                if (event->len > 0)
                {
                    if (event->mask & IN_MODIFY || event->mask & IN_MOVE)
                    {
                        String fileName;
                        fileName = dirHandle_[event->wd] + event->name;
                        AddChange(fileName);
                    }
                }

                i += sizeof(inotify_event) + event->len;
            }
        }
#endif
    }

    void FileWatcher::AddChange(const String& fileName)
    {
        MutexLock lock(changesMutex_);

        // Reset the timer associated with the filename. Will be notified once timer exceeds the delay
        changes_[fileName].Reset();
    }

    bool FileWatcher::GetNextChange(String& dest)
    {
        MutexLock lock(changesMutex_);

        auto delayMsec = (unsigned)(delay_ * 1000.0f);

        if (changes_.Empty())
            return false;
        else
        {
            for (HashMap<String, Timer>::Iterator i = changes_.Begin(); i != changes_.End(); ++i)
            {
                if (i->second_.GetMSec(false) >= delayMsec)
                {
                    dest = i->first_;
                    changes_.Erase(i);
                    return true;
                }
            }

            return false;
        }
    }
}