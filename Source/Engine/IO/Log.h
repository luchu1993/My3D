//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"
#include "Core/Object.h"

namespace My3D
{

static const int LOG_RAW = -1;
static const int LOG_TRACE = 0;
static const int LOG_DEBUG = 1;
static const int LOG_INFO = 2;
static const int LOG_WARNING = 3;
static const int LOG_ERROR = 4;
static const int LOG_NONE = 5;


class MY3D_API Log : public Object
{
    MY3D_OBJECT(Log, Object)

public:
    explicit Log(Context* context);
    ~Log() override;

    void SetLevel(int level);

    void SetQuiet(bool quiet);

    static void Write(int level,  const String& message);
    static void WriteFormat(int level, const char* format, ...);
    static void WriteRaw(const String& message, bool error = false);

private:
    /// Send logging event
    static void SendLogEvent(const String& message, int level);
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