#ifndef UNICODE /* VC6编译时需要将默认的/MLd改成/MT */
#define UNICODE
#endif
#define _WIN32_WINNT 0x0501
#include <afx.h>
#include <stdlib.h>
#include <stdio.h>

void ShowError(LPTSTR lpszFunction)
{
    LPVOID lpMsgBuf, lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));

    sprintf((char*)lpDisplayBuf, "%s failed with error %d: %s", lpszFunction, dw, lpMsgBuf);
	printf("%s ", lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

bool TranslateFile(LPCWSTR exeFile, LPCWSTR outFile)
{
    void *base;
    int i = 0;
    CFile *file;

    HANDLE hFile, hMapping;
    hFile = CreateFile(exeFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        ShowError(L"CreateFile!");

        return false;
    }

    hMapping = CreateFileMapping(hFile, 0, PAGE_READONLY | SEC_COMMIT, 0, 0, 0);

    if (NULL == hMapping)
    {
        ShowError(L"CreateFileMapping");
        CloseHandle(hFile);

        return false;
    }

    base = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);

    if (NULL == base)
    {
        ShowError(L"MapViewOfFile");
        CloseHandle(hMapping);
        CloseHandle(hFile);

        return false;
    }

	printf("[Trans] base=%08X\n", base);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)base;
    PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)base + dosHeader->e_lfanew);

    try
    {
        file = new CFile(outFile, CFile::typeBinary | CFile::modeWrite | CFile::modeCreate);
    }
    catch (CFileException  ex)
    {
        ex.ReportError();

        return false;
    }

    try
    {
        int vSize = 0;

        PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeader);

        vSize = sectionHeader[3].VirtualAddress + sectionHeader[3].Misc.VirtualSize;
        printf("[Trans] vSize=%08X, VirtualAddress=%08X, VirtualSize=%08X\n", vSize, 
        sectionHeader[3].VirtualAddress, sectionHeader[3].Misc.VirtualSize);

        file->SetLength(vSize);
        file->SeekToBegin();

        char buffer[256];
        memset(buffer, 0, 256);
        for (i = 0; i < vSize / 256; i++)
        {
            file->Write(buffer, 256);
        }

        file->Write(buffer, vSize % 256);
        file->SeekToBegin();

        for (i = 0; i < 4; i++) /* text, data, bss, idata */
        {
            int RawOffset = sectionHeader[i].PointerToRawData;
            int virtualAddress = sectionHeader[i].VirtualAddress;
            int sizeOfRawData = sectionHeader[i].SizeOfRawData;

            printf("[Trans] %-6s PEOff %08x, PEEnd %08x, BinOff %08x, BinEnd %08x, size %08x"
				, sectionHeader[i].Name, RawOffset, RawOffset + sizeOfRawData
				, virtualAddress, virtualAddress + sizeOfRawData, sizeOfRawData);

            if (sizeOfRawData == 0)
            {
				printf(" <no data>\n");
                continue;
            }
			printf("\n");

            file->Seek(virtualAddress, CFile::begin);
            file->Write((char*)base + RawOffset, sizeOfRawData);
        }
    }
    catch (CFileException ex)
    {
        ex.ReportError();
        file->Close();

        return false;
    }
    file->Close();
    UnmapViewOfFile(base);
    return true;
}
int _tmain(int argc, LPCWSTR argv[])
{
    if (3 != argc)
    {
        puts("usage: Trans exeFile binFileName");
    }
    TranslateFile(argv[1], argv[2]);
    return 0;
}
