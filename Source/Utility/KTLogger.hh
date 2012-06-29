/*
 * KTLogger.h
 *
 *  Created on: 18.11.2011 (KLogger from KATRIN's Kasper package)
 *      Author: Marco Haag <marco.haag@kit.edu> and Sebastian Voecking
 *
 *  Ported to Katydid on: 01.05.2012
 *      By: Noah Oblath
 */

#ifndef KTLOGGER_H_
#define KTLOGGER_H_

// UTILITY MACROS

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __FILE_LINE__      __FILE__ "(" TOSTRING(__LINE__) ")"
#define __FILENAME_LINE__  (strrchr(__FILE__, '/') ? strrchr(__FILE_LINE__, '/') + 1 : __FILE_LINE__)

#if defined(_MSC_VER)
#if _MSC_VER >= 1300
#define __FUNC__ __FUNCSIG__
#endif
#else
#if defined(__GNUC__)
#define __FUNC__ __PRETTY_FUNCTION__
#endif
#endif
#if !defined(__FUNC__)
#define __FUNC__ ""
#endif

#define va_num_args(...) va_num_args_impl(__VA_ARGS__, 5,4,3,2,1)
#define va_num_args_impl(_1,_2,_3,_4,_5,N,...) N

#define macro_dispatcher(func, ...) macro_dispatcher_(func, va_num_args(__VA_ARGS__))
#define macro_dispatcher_(func, nargs) macro_dispatcher__(func, nargs)
#define macro_dispatcher__(func, nargs) func ## nargs

// INCLUDES

#include <cstring>
#include <iostream>
#include <sstream>

// CLASS DEFINITIONS

namespace Katydid {

    class KTLogger
    {
        public:
            struct Location {
                Location(const char * const fileName = "", const char * const functionName = "", int lineNumber = -1) :
                    fLineNumber(lineNumber),
                    fFileName(fileName),
                    fFunctionName(functionName)
                {}
                int fLineNumber;
                const char * fFileName;
                const char * fFunctionName;
            };

        public:
            KTLogger(const char* name);
            KTLogger(const std::string& name);

            bool IsLevelEnabled(const std::string& level) const;
            bool IsDebugEnabled() const;
            bool IsInfoEnabled() const;
            bool IsWarnEnabled() const;
            bool IsErrorEnabled() const;
            bool IsFatalEnabled() const;

            void logLevel(const std::string& level, const std::string& message, const Location& loc = Location());
            void logDebug(const std::string& message, const Location& loc = Location());
            void logInfo(const std::string& message, const Location& loc = Location());
            void logWarn(const std::string& message, const Location& loc = Location());
            void logError(const std::string& message, const Location& loc = Location());
            void logFatal(const std::string& message, const Location& loc = Location());

        private:
            struct Private;
            Private* fPrivate;
    };
}

// PRIVATE MACROS

#define __DEFAULT_LOGGER        Katydid::KTLogger(__FILENAME_LINE__)

#define __KTLOG_LOCATION         Katydid::KTLogger::Location(__FILE__, __FUNC__, __LINE__)

#define __KTLOG_DEBUG_2(I,M) \
        if (I.IsDebugEnabled()) { \
            std::ostringstream stream; \
            stream << M; \
            I.logDebug(stream.str(), __KTLOG_LOCATION); \
        }
#define __KTLOG_DEBUG_1(M)       __KTLOG_DEBUG_2(__DEFAULT_LOGGER,M)

#define __KTLOG_INFO_2(I,M) \
        if (I.IsInfoEnabled()) { \
            std::ostringstream stream; \
            stream << M; \
            I.logInfo(stream.str(), __KTLOG_LOCATION); \
        }
#define __KTLOG_INFO_1(M)        __KTLOG_INFO_2(__DEFAULT_LOGGER,M)

#define __KTLOG_WARN_2(I,M) \
        if (I.IsWarnEnabled()) { \
            std::ostringstream stream; \
            stream << M; \
            I.logWarn(stream.str(), __KTLOG_LOCATION); \
        }
#define __KTLOG_WARN_1(M)        __KTLOG_WARN_2(__DEFAULT_LOGGER,M)

#define __KTLOG_ERROR_2(I,M) \
        if (I.IsErrorEnabled()) { \
            std::ostringstream stream; \
            stream << M; \
            I.logError(stream.str(), __KTLOG_LOCATION); \
        }
#define __KTLOG_ERROR_1(M)       __KTLOG_ERROR_2(__DEFAULT_LOGGER,M)

#define __KTLOG_FATAL_2(I,M) \
        if (I.IsFatalEnabled()) { \
            std::ostringstream stream; \
            stream << M; \
            I.logFatal(stream.str(), __KTLOG_LOCATION); \
        }
#define __KTLOG_FATAL_1(M)       __KTLOG_FATAL_2(__DEFAULT_LOGGER,M)

#define __KTLOG_ASSERT_3(I,C,M) \
        if (!(C) && I.IsErrorEnabled()) { \
            std::ostringstream stream; \
            stream << M; \
            I.logError(stream.str(), __KTLOG_LOCATION); \
        }

#define __KTLOG_ASSERT_2(C,M)    __KTLOG_ASSERT_3(__DEFAULT_LOGGER,C,M)

#define __KTLOG_LOG_3(I,L,M) \
        if (I.IsLevelEnabled(L)) { \
            std::ostringstream stream; \
            stream << M; \
            I.logLevel(L,stream.str(), __KTLOG_LOCATION); \
        }
#define __KTLOG_LOG_2(L,M)       __KTLOG_LOG_3(__DEFAULT_LOGGER,L,M)
#define __KTLOG_LOG_1(M)         __KTLOG_DEBUG_1(M)


// PUBLIC MACROS

#define KTLOGGER(I,K)      static Katydid::KTLogger I(K);

#define KTLOG(...)         macro_dispatcher(__KTLOG_LOG_, __VA_ARGS__)(__VA_ARGS__)
#define KTDEBUG(...)       macro_dispatcher(__KTLOG_DEBUG_, __VA_ARGS__)(__VA_ARGS__)
#define KTINFO(...)        macro_dispatcher(__KTLOG_INFO_, __VA_ARGS__)(__VA_ARGS__)
#define KTWARN(...)        macro_dispatcher(__KTLOG_WARN_, __VA_ARGS__)(__VA_ARGS__)
#define KTERROR(...)       macro_dispatcher(__KTLOG_ERROR_, __VA_ARGS__)(__VA_ARGS__)
#define KTFATAL(...)       macro_dispatcher(__KTLOG_FATAL_, __VA_ARGS__)(__VA_ARGS__)
#define KTASSERT(...)      macro_dispatcher(__KTLOG_ASSERT_, __VA_ARGS__)(__VA_ARGS__)

#endif /* KTLOGGER_H_ */