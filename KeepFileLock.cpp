// KeepFileLock.cpp
// Copyright (C) 2020 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This software is public domain software.
#include <windows.h>
#include <stdio.h>

void
show_version(void)
{
    puts("KeepFileLock Version 1.0 by katahiromz");
}

void
usage(void)
{
    puts("Usage: KeepFileLock [options] [files-to-lock]");
    puts("Options:");
    puts("--help        Show help message");
    puts("--version     Show version info");
    puts("--wait NNN    Wait file locking for NNN seconds (default: infinite)");
}

int
JustDoIt(int i, int argc, wchar_t **wargv, DWORD dwMSEC, int ret)
{
    if (ret != EXIT_SUCCESS)
        return ret;

    if (i >= argc)
    {
        Sleep(dwMSEC);
        return ret;
    }

    const wchar_t *arg = wargv[i];
    if (arg[0] == '-' && arg[1] == '-')
    {
        if (lstrcmpiW(arg, L"--wait") == 0 && i + 1 < argc)
        {
            ret = JustDoIt(i + 2, argc, wargv, dwMSEC, ret);
        }
        else
        {
            ret = JustDoIt(i + 1, argc, wargv, dwMSEC, ret);
        }
        return ret;
    }

    LARGE_INTEGER FileSize;
    ZeroMemory(&FileSize, sizeof(FileSize));

    HANDLE hFile = CreateFileW(arg, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "ERROR: Cannot open file: %ls\n", arg);
        ret = EXIT_FAILURE;
    }
    else
    {
        if (!GetFileSizeEx(hFile, &FileSize))
        {
            fprintf(stderr, "ERROR: Cannot get file size: %ls\n", arg);
            ret = EXIT_FAILURE;
        }
        else if (!LockFile(hFile, 0, 0, FileSize.LowPart, FileSize.HighPart))
        {
            fprintf(stderr, "ERROR: Cannot lock file: %ls\n", arg);
            ret = EXIT_FAILURE;
        }
    }

    ret = JustDoIt(i + 1, argc, wargv, dwMSEC, ret);

    UnlockFile(hFile, 0, 0, FileSize.LowPart, FileSize.HighPart);
    CloseHandle(hFile);
    return ret;
}

int __cdecl
wmain(int argc, wchar_t **wargv)
{
    DWORD dwMSEC = INFINITE;

    if (argc <= 1)
    {
        usage();
        return 0;
    }

    for (int i = 1; i < argc; ++i)
    {
        const wchar_t *arg = wargv[i];
        if (lstrcmpiW(arg, L"--help") == 0)
        {
            usage();
            return 0;
        }
        if (lstrcmpiW(arg, L"--version") == 0)
        {
            show_version();
            return 0;
        }
        if (lstrcmpiW(arg, L"--wait") == 0)
        {
            if (i + 1 < argc)
            {
                ++i;
                arg = wargv[i];
                if (lstrcmpiW(arg, L"INFINITE") == 0)
                {
                    dwMSEC = INFINITE;
                }
                else
                {
                    dwMSEC = _wtoi(arg) * 1000;
                }
            }
            else
            {
                fprintf(stderr, "ERROR: Option --wait requires operand.\n");
                return EXIT_FAILURE;
            }
            continue;
        }
    }

    return JustDoIt(1, argc, wargv, dwMSEC, EXIT_SUCCESS);
}
