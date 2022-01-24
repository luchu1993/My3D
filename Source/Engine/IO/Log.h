//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "Container/List.h"
#include "Core/Object.h"
#include "Core/Mutex.h"
#include "Core/StringUtils.h"


namespace My3D
{

/// Fictional message level to indicate a stored raw message.
static const int LOG_RAW = -1;
/// Trace message level.
static const int LOG_TRACE = 0;
/// Debug message level. By default only shown in debug mode.
static const int LOG_DEBUG = 1;
/// Informative message level.
static const int LOG_INFO = 2;
/// Warning message level.
static const int LOG_WARNING = 3;
/// Error message level.
static const int LOG_ERROR = 4;
/// Disable all log messages.
static const int LOG_NONE = 5;

class File;

/// Stored log message from another thread.
struct StoredLogMessage
{
    /// Construct undefined.
    StoredLogMessage() = default;

    /// Construct with parameters.
    StoredLogMessage(const String& message, int level, bool error)
        : message_(message)
        , level_(level)
        , error_(error)
    {
    }

    /// Message text.
    String message_;
    /// Message level. -1 for raw messages.
    int level_{};
    /// Error flag for raw messages.
    bool error_{};
};

class MY3D_API Log : public Object
{
    MY3D_OBJECT(Log, Object)

public:
    /// Construct.
    explicit Log(Context* context);
    /// Destruct. Close the log file if open.
    ~Log() override;

    /// Open the log file.
    void Open(const String& fileName);
    /// Close the log file.
    void Close();
    /// Set logging level.
    void SetLevel(int level);
    /// Set whether to timestamp log messages.
    void SetTimeStamp(bool enable);
    /// Set quiet mode ie. only print error entries to standard error stream (which is normally redirected to console also). Output to log file is not affected by this mode.
    void SetQuiet(bool quiet);

    /// Return logging level.
    int GetLevel() const { return level_; }
    /// Return whether log messages are timestamped.
    bool GetTimeStamp() const { return timeStamp_; }
    /// Return last log message.
    String GetLastMessage() const { return lastMessage_; }
    /// Return whether log is in quiet mode (only errors printed to standard error stream).
    bool IsQuiet() const { return quiet_; }

    /// Write to the log. If logging level is higher than the level of the message, the message is ignored.
    static void Write(int level, const String& message);
    /// Write formatted message to the log. If logging level is higher than the level of the message, the message is ignored.
    static void WriteFormat(int level, const char* format, ...);
    /// Write raw output to the log.
    static void WriteRaw(const String& message, bool error = false);

private:
    /// Send logging event
    static void SendLogEvent(const String& message, int level);

    /// Handle end of frame. Process the threaded log messages.
    void HandleEndFrame(StringHash eventType, VariantMap& eventData);
    /// Mutex for threaded operation.
    Mutex logMutex_;
    /// Log messages from other threads.
    List<StoredLogMessage> threadMessages_;
    /// Log file.
    SharedPtr<File> logFile_;
    /// Last log message
    String lastMessage_;
    /// Logging level
    int level_;
    /// Timestamp log messages flag
    bool timeStamp_;
    /// In write flag to prevent recursion
    bool inWrite_;
    /// Quiet mode flag
    bool quiet_;
};

#ifdef MY3D_LOGGING
#define MY3D_LOG(level, message) My3D::Log::Write(level, message)
#define MY3D_LOGTRACE(message) My3D::Log::Write(My3D::LOG_TRACE, message)
#define MY3D_LOGDEBUG(message) My3D::Log::Write(My3D::LOG_DEBUG, message)
#define MY3D_LOGINFO(message) My3D::Log::Write(My3D::LOG_INFO, message)
#define MY3D_LOGWARNING(message) My3D::Log::Write(My3D::LOG_WARNING, message)
#define MY3D_LOGERROR(message) My3D::Log::Write(My3D::LOG_ERROR, message)
#define MY3D_LOGRAW(message) My3D::Log::Write(My3D::LOG_RAW, message)

#define MY3D_LOGF(level, format, ...) My3D::Log::WriteFormat(level, format, ##__VA_ARGS__)
#define MY3D_LOGTRACEF(format, ...) My3D::Log::WriteFormat(My3D::LOG_TRACE, format, ##__VA_ARGS__)
#define MY3D_LOGDEBUGF(format, ...) My3D::Log::WriteFormat(My3D::LOG_DEBUG, format, ##__VA_ARGS__)
#define MY3D_LOGINFOF(format, ...) My3D::Log::WriteFormat(My3D::LOG_INFO, format, ##__VA_ARGS__)
#define MY3D_LOGWARNINGF(format, ...) My3D::Log::WriteFormat(My3D::LOG_WARNING, format, ##__VA_ARGS__)
#define MY3D_LOGERRORF(format, ...) My3D::Log::WriteFormat(My3D::LOG_ERROR, format, ##__VA_ARGS__)
#define MY3D_LOGRAWF(format, ...) My3D::Log::WriteFormat(My3D::LOG_RAW, format, ##__VA_ARGS__)
#else
#define MY3D_LOG(message)
#define MY3D_LOGTRACE(message)
#define MY3D_LOGDEBUG(message)
#define MY3D_LOGINFO(message)
#define MY3D_LOGWARNING(message)
#define MY3D_LOGERROR(message)
#define MY3D_LOGRAW(message)

#define MY3D_LOGF(...)
#define MY3D_LOGTRACEF(...)
#define MY3D_LOGDEBUGF(...)
#define MY3D_LOGINFOF(...)
#define MY3D_LOGWARNINGF(...)
#define MY3D_LOGERRORF(...)
#define MY3D_LOGRAWF(...)

#endif

}