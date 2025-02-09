
#include<iostream>
#include<string>
#include <windows.h>
#include <sysinfoapi.h>
#include <chrono>
#include <unordered_map>
#include <map>
#include <algorithm>

using namespace std;


BOOL GetCpuCashLineSize(std::map<BYTE, WORD>& mapCpuCacheLineSize)
{
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;

    BOOL bDone = FALSE;
    do
    {
        DWORD rc = GetLogicalProcessorInformation(buffer, &returnLength);
        if (FALSE == rc)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (buffer)
                    free(buffer);
                // returnLength에 buffer를 할당한 사이즈를 전달 받습니다.
                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
                // buffer를 할당하고 다시 GetLogicalProcessorInformation 호출! 
                if (NULL == buffer)
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            bDone = TRUE;
        }
    } while (!bDone);


    if (buffer)
    {
        ptr = buffer;
        DWORD byteOffset = 0;
        PCACHE_DESCRIPTOR Cache;

        // 전달받은 버퍼를 순회하면서 Cache정보한 사용합니다
        while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
        {
            switch (ptr->Relationship)
            {
                // 타입이 캐시 일 때만 정보를 가져옵니다. 
            case RelationCache:
                Cache = &ptr->Cache;
                if (mapCpuCacheLineSize.end() == mapCpuCacheLineSize.find(Cache->Level))
                {
                    mapCpuCacheLineSize.insert(std::map<BYTE, WORD>::value_type(Cache->Level, Cache->LineSize));
                }
                break;
            }

            byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
            ptr++;
        }
        free(buffer);
        return TRUE;
    }

    return FALSE;
}

int main()
{
    std::map<BYTE, WORD> mapCpuCacheLineSize;
    if (FALSE == GetCpuCashLineSize(mapCpuCacheLineSize))
    {
        cout << "FALSE == GetCpuCashLineSize GetLastError = " << GetLastError() << endl;
    }
    else
    {
        for (auto val : mapCpuCacheLineSize)
        {
            cout << "CacheLevel = " << (WORD)val.first << " // CPUCacheLineSize = " << val.second << endl;
        }
    }
}