
#pragma once

#include "base/base_typedef.h"

#include "core/core_lua_typedef.h"

namespace LUA
{
    class CArgument;
    class CResult;
} // namespace LUA

namespace LUA
{
namespace State
{
    BState CreateState(const Base::Char* _pName);
    void DeleteState(BState _State);

    void LoadScript(BState _State, const Base::Char* _pFilename, int _Flags = 0);

    bool CallFunction(BState _State, const Base::Char* _pFunction, CResult* _pResult);
    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, CResult* _pResult = nullptr);
    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, const CArgument& _rArgument2, CResult* _pResult = nullptr);
    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, const CArgument& _rArgument2, const CArgument& _rArgument3, CResult* _pResult = nullptr);
    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, const CArgument& _rArgument2, const CArgument& _rArgument3, const CArgument& _rArgument4, CResult* _pResult = nullptr);
    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument& _rArgument1, const CArgument& _rArgument2, const CArgument& _rArgument3, const CArgument& _rArgument4, const CArgument& _rArgument5, CResult* _pResult = nullptr);
    bool CallFunction(BState _State, const Base::Char* _pFunction, const CArgument** _ppArguments, unsigned int _NumberOfArguments, CResult* _pResult = nullptr);

    void RequireLibrary(BState _State, const Base::Char* _pLibraryName, FLuaCFunc _pFunction);

    void RegisterFunction(BState _State, const Base::Char* _pFunctionName, FLuaCFunc _pFunction);
    void RegisterLibrary(BState _State, const Base::Char* _pLibraryName, const Base::Char* _pFunctionName, FLuaCFunc _pFunction);
    void RegisterObject(BState _State, const Base::Char* _pObjectName, const Base::Char* _pFunctionName, FLuaCFunc _pFunction);

    void PushBoolean(BState _State, bool _Value);
    void PushSChar(BState _State, signed char _Value);
    void PushUChar(BState _State, unsigned char _Value);
    void PushSShort(BState _State, signed short _Value);
    void PushUShort(BState _State, unsigned short _Value);
    void PushSInt(BState _State, signed int _Value);
    void PushUInt(BState _State, unsigned int _Value);
    void PushSLong(BState _State, signed long _Value);
    void PushULong(BState _State, unsigned long _Value);
    void PushSLongLong(BState _State, signed long long _Value);
    void PushULongLong(BState _State, unsigned long long _Value);
    void PushFloat(BState _State, float _Value);
    void PushDouble(BState _State, double _Value);
    void PushLongDouble(BState _State, long double _Value);
    void PushString(BState _State, const Base::Char* _pString);
    void* PushUserData(BState _State, unsigned int _NumberOfBytes, const Base::Char* _pObjectName);

    bool GetBoolean(BState _State, int _IndexOfStack, bool _DefaultValue = false);
    signed char GetSChar(BState _State, int _IndexOfStack, signed char _DefaultValue = 0);
    unsigned char GetUChar(BState _State, int _IndexOfStack, unsigned char _DefaultValue = 0);
    signed short GetSShort(BState _State, int _IndexOfStack, signed short _DefaultValue = 0);
    unsigned short GetUShort(BState _State, int _IndexOfStack, unsigned short _DefaultValue = 0);
    signed int GetSInt(BState _State, int _IndexOfStack, signed int _DefaultValue = 0);
    unsigned int GetUInt(BState _State, int _IndexOfStack, unsigned int _DefaultValue = 0);
    signed long GetSLong(BState _State, int _IndexOfStack, signed long _DefaultValue = 0);
    unsigned long GetULong(BState _State, int _IndexOfStack, unsigned long _DefaultValue = 0);
    signed long long GetSLongLong(BState _State, int _IndexOfStack, signed long long _DefaultValue = 0);
    unsigned long long GetULongLong(BState _State, int _IndexOfStack, unsigned long long _DefaultValue = 0);
    float GetFloat(BState _State, int _IndexOfStack, float _DefaultValue = 0);
    double GetDouble(BState _State, int _IndexOfStack, double _DefaultValue = 0);
    long double GetLongDouble(BState _State, int _IndexOfStack, long double _DefaultValue = 0);
    const Base::Char* GetString(BState _State, int _IndexOfStack);
    void* GetUserData(BState _State, int _IndexOfStack);

    bool IsBoolean(BState _State, int _IndexOfStack);
    bool IsNumber(BState _State, int _IndexOfStack);
    bool IsString(BState _State, int _IndexOfStack);
} // namespace State
} // namespace LUA