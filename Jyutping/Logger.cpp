#include "Logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

namespace Global {

    static const wchar_t* LOG_FILE_PATH = L"C:\\Users\\Public\\Jyutping.log";

    void StartLog()
    {
        FILE* fp = nullptr;
        _wfopen_s(&fp, LOG_FILE_PATH, L"w");
        if (fp)
        {
            time_t now;
            time(&now);
            char timeStr[26];
            ctime_s(timeStr, sizeof(timeStr), &now);
            fprintf(fp, "=== Jyutping IME Log Started: %s ===\n", timeStr);
            fclose(fp);
        }
    }

    void Log(const wchar_t* format, ...)
    {
        FILE* fp = nullptr;
        _wfopen_s(&fp, LOG_FILE_PATH, L"a, ccs=UTF-8"); // Append mode
        if (fp)
        {
            va_list args;
            va_start(args, format);
            vfwprintf(fp, format, args);
            va_end(args);
            fwprintf(fp, L"\n");
            fclose(fp);
        }
    }
}
