//
// Created by luchu on 2022/1/1.
//

#include "IO/Log.h"
#include "IO/IOEvents.h"
#include "IO/File.h"
#include "Core/Timer.h"
#include "Core/ProcessUtils.h"
#include "Core/CoreEvents.h"
#include "Core/Thread.h"


namespace My3D
{

const char* logLevelPrefixes[] =
{
    "TRACE",
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    nullptr
};

static Log* logInstance = nullptr;
static bool threadErrorDisplayed = false;

Log::Log(Context *context)
    : Base(context)
#ifdef _DEBUG
    , level_(LOG_DEBUG)
#else
    , level_(LOG_INFO)
#endif
    , timeStamp_(true)
    , inWrite_(false)
    , quiet_(false)
{
    logInstance = this;
    SubscribeToEvent(E_ENDFRAME, MY3D_HANDLER(Log, HandleEndFrame));
}

Log::~Log()
{
    logInstance = nullptr;
}

void Log::Open(const String &fileName)
{
    if (fileName.Empty())
        return;

    if (logFile_ && logFile_->IsOpen())
    {
        if (logFile_->GetName() == fileName)
            return;
        else
            Close();
    }

    logFile_ = new File(context_);
    if (logFile_->Open(fileName, FILE_WRITE))
        Write(LOG_INFO, "Opened log file " + fileName);
    else
    {
        logFile_.Reset();
        Write(LOG_ERROR, "Failed to create log file " + fileName);
    }
}

void Log::Close()
{
    if (logFile_ && logFile_->IsOpen())
    {
        logFile_->Close();
        logFile_.Reset();
    }
}

void Log::SetLevel(int level)
{
    if (level < LOG_TRACE || level >= LOG_NONE)
    {
        MY3D_LOGERRORF("Attempted to set erroneous log level %d", level);
        return;
    }

    level_ = level;
}

void Log::SetTimeStamp(bool enable)
{
    timeStamp_ = enable;
}

void Log::SetQuiet(bool quiet)
{
    quiet_ = quiet;
}

void Log::Write(int level, const String &message)
{
    // Special case for LOG_RAW level
    if (level == LOG_RAW)
    {
        WriteRaw(message, false);
        return;
    }

    // No-op if illegal level
    if (level < LOG_TRACE || level >= LOG_NONE)
        return;

    // If not in the main thread, store message for later processing
    if (!Thread::IsMainThread())
    {
        if (logInstance)
        {
            MutexLock lock(logInstance->logMutex_);
            logInstance->threadMessages_.Push(StoredLogMessage(message, level, false));
        }

        return;
    }
    // Do not log if message level excluded or if currently sending a log event
    if (!logInstance || logInstance->level_ > level || logInstance->inWrite_)
        return;

    String formattedMessage = logLevelPrefixes[level];
    formattedMessage += ": " + message;
    logInstance->lastMessage_ = message;

    if (logInstance->timeStamp_)
        formattedMessage = "[" + Time::GetTimeStamp() + "] " + formattedMessage;

    if (logInstance->quiet_)
    {
        if (level == LOG_ERROR)
            PrintUnicodeLine(formattedMessage, true);
    }
    else
        PrintUnicodeLine(formattedMessage, level == LOG_ERROR);

    if (logInstance->logFile_)
    {
        logInstance->logFile_->WriteLine(formattedMessage);
        logInstance->logFile_->Flush();
    }

    SendLogEvent(message, level);
}

void Log::WriteRaw(const String &message, bool error)
{
    // If not in the main thread, store message for later processing
    if (!Thread::IsMainThread())
    {
        if (logInstance)
        {
            MutexLock lock(logInstance->logMutex_);
            logInstance->threadMessages_.Push(StoredLogMessage(message, LOG_RAW, error));
        }

        return;
    }
    // Prevent recursion during log event
    if (!logInstance || logInstance->inWrite_)
        return;

    logInstance->lastMessage_ = message;

    if (logInstance->quiet_)
    {
        if (error)
            PrintUnicode(message, true);
    }
    else
        PrintUnicode(message, error);

    if (logInstance->logFile_)
    {
        logInstance->logFile_->Write(message.CString(), message.Length());
        logInstance->logFile_->Flush();
    }

    SendLogEvent(message, error ? LOG_ERROR : LOG_INFO);
}

void Log::WriteFormat(int level, const char *format, ...)
{
    if (!logInstance || logInstance->level_ > level)
        return;

    String message;
    va_list args;
    va_start(args, format);
    message.AppendWithFormatArgs(format, args);
    va_end(args);

    Write(level, message);
}

void Log::HandleEndFrame(StringHash eventType, VariantMap &eventData)
{
    // If the MainThreadID is not valid, processing this loop can potentially be endless
    if (!Thread::IsMainThread())
    {
        if (!threadErrorDisplayed)
        {
            fprintf(stderr, "Thread::mainThreadID is not setup correctly! Threaded log handling disabled\n");
            threadErrorDisplayed = true;
        }
        return;
    }

    MutexLock lock(logMutex_);

    // Process messages accumulated from other threads (if any)
    while (!threadMessages_.Empty())
    {
        const StoredLogMessage& stored = threadMessages_.Front();

        if (stored.level_ != LOG_RAW)
            Write(stored.level_, stored.message_);
        else
            WriteRaw(stored.message_, stored.error_);

        threadMessages_.PopFront();
    }
}

void Log::SendLogEvent(const String& message, int level)
{
    logInstance->inWrite_ = true;

    using namespace LogMessage;

    VariantMap& eventData = logInstance->GetEventDataMap();
    eventData[P_MESSAGE] = message;
    eventData[P_LEVEL] = level;
    logInstance->SendEvent(E_LOGMESSAGE, eventData);

    logInstance->inWrite_ = false;
}

}