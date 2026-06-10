#pragma once
#include <shlwapi.h>

void Internal_DebugLog(const char* pFormat, ...);

#ifdef __cplusplus
void Internal_DebugLog(const char8_t* pFormat, ...);
#endif
