#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "../Utils/BufferFmt.hpp"
#include "GameProfile.hpp"

namespace Dumper
{
    enum DumpStatus : uint8_t
    {
        UE_DS_NONE = 0,
        UE_DS_SUCCESS,
        UE_DS_ERROR_EXE_NAME_NULL,
        UE_DS_ERROR_EXE_NOT_FOUND,
        UE_DS_ERROR_INIT_GNAMES,
        UE_DS_ERROR_INIT_NAMEPOOL,
        UE_DS_ERROR_INIT_GUOBJECTARRAY,
        UE_DS_ERROR_INIT_OBJOBJECTS,
        UE_DS_ERROR_INIT_OFFSETS,
        UE_DS_ERROR_EMPTY_PACKAGES
    };
    
    DumpStatus InitUEVars(IGameProfile *gameProfile);
    
    DumpStatus Dump(std::unordered_map<std::string, BufferFmt> *outBuffersMap);

    std::string DumpStatusToStr(DumpStatus ds);
}
