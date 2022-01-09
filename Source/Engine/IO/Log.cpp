//
// Created by luchu on 2022/1/1.
//

#include "IO/Log.h"
#include "Core/Timer.h"
#include "Core/ProcessUtils.h"


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
}

Log::~Log()
{
    logInstance = nullptr;
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

void Log::SetQuiet(bool quiet)
{
    quiet_ = quiet;
}

void Log::Write(int level, const String &message)
{
    if (level == LOG_RAW)
    {
        WriteRaw(message, false);
        return;
    }

    if (level < LOG_TRACE || level >= LOG_NONE)
        return;

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

    SendLogEvent(message, level);
}

void Log::WriteRaw(const String &message, bool error)
{
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

void Log::SendLogEvent(const String& message, int level)
{
    logInstance->inWrite_ = true;

    // Send logging event

    logInstance->inWrite_ = false;
}

}