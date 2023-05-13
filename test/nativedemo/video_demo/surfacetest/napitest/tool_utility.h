/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CC_TOOL_H
#define CC_TOOL_H

#include <node_api.h>
#include <string>
#include <memory>
#include <map>
#include <any>
#include <vector>
#include <cmath>

using DataPtr = struct DumyData*;

struct AsyncFunc {
    napi_env env_;
    napi_ref funcRef_;
    napi_ref thisVarRef_;
};

class XNapiTool {
public:
    static const int ZERO = 0;
    static const int ONE = 1;
    static const int TWO = 2;
    static const int THREE = 3;
    static const int FOUE = 4;
    static const int FIVE = 5;
    static const int SIX = 6;
    static const int SEVEN = 7;
    static const int EIGHT = 8;
    static const int NINE = 9;
    void RegistAsyncFunc(std::string name, napi_value func);
    void UnregistAsyncFunc(std::string name);
    static void CallAsyncFunc(AsyncFunc *pAsyncFuncs, napi_value ret);

    using CallbackFunction = void (*)(XNapiTool *pxt, DataPtr data);
    using RELEASE_INSTANCE = void (*)(DataPtr p);
    static napi_value UndefinedValue(napi_env env);
    const uint32_t DEFAULT_ARG_COUNT = 8;
    napi_value UndefinedValue();

    napi_value CreateSubObject(napi_value parent, const char *name);
    void DefineFunction(const char *funcName, napi_callback callback, napi_value dest = nullptr);
    void DefineClass(const char *className, napi_callback constructorFunc,
        std::map<const char *, std::map<const char *, napi_callback>> &valueList, std::map<const char *,
        napi_callback> &funcList, napi_value dest = nullptr);

    void SetEnumProperty(napi_value dstObj, const char *propName, const std::any objValue);
    void CreateEnumObject(const char *enumName, std::map<const char *, std::any> enumMap);

    XNapiTool(napi_env env, napi_callback_info info);
    XNapiTool(napi_env env, napi_value exports);
    ~XNapiTool();

    bool SwapJs2CBool(napi_value value);
    int32_t SwapJs2CInt32(napi_value value);
    uint32_t SwapJs2CUint32(napi_value value);
    int64_t SwapJs2CInt64(napi_value value);
    double_t SwapJs2CDouble(napi_value value);
    size_t SwapJs2CUtf8(napi_value value, std::string &str);

    napi_value SwapC2JsBool(bool value);
    napi_value SwapC2JsInt32(int32_t value);
    napi_value SwapC2JsUint32(uint32_t value);
    napi_value SwapC2JsInt64(int64_t value);
    napi_value SwapC2JsDouble(double_t value);
    napi_value SwapC2JsUtf8(const char *value);

    napi_value GetArgv(uint32_t p);
    uint32_t GetArgc();

    napi_value GetValueProperty(napi_value value, const char *propertyName);
    napi_value SetValueProperty(napi_value &value, const char *propertyName, napi_value property);

    napi_value CreateArray(napi_value &value);
    uint32_t GetArrayLength(napi_value value);
    napi_value GetArrayElement(napi_value value, uint32_t p);
    napi_value SetArrayElement(napi_value &value, uint32_t p, napi_value ele);
    
    uint32_t GetMapLength(napi_value value);
    napi_value GetMapElementName(napi_value value, uint32_t p);
    napi_value GetMapElementValue(napi_value value, const char *p);
    napi_value SetMapElement(napi_value &value, const char *ele_key, napi_value ele_value);

    std::string GetUnionType(napi_value object);

    std::string GetAnyType(napi_value object);
    std::string GetAnyArrayType(napi_value object);
    void SetAnyValue(std::string &any_type, napi_value argv, std::any &any);
    void GetAnyValue(std::string any_type, napi_value &result, std::any any);
    void GetObjectValVecUint32(std::any &anyVal, napi_value &tnv2);
    void GetObjectValVecInt32(std::any &anyVal, napi_value &tnv2);
    void GetObjectValVecInt64(std::any &anyVal, napi_value &tnv2);
    void GetObjectValVecDouble(std::any &anyVal, napi_value &tnv2);
    void GetObjectValVecBool(std::any &anyVal, napi_value &tnv2);
    void GetObjectValVecConstchar(std::any &anyVal, napi_value &tnv2);
    void GetObjectValMapString(std::any &anyVal, napi_value &tnv2);
    void GetObjectValMapUint32(std::any &anyVal, napi_value &tnv2);
    void GetObjectValMapInt32(std::any &anyVal, napi_value &tnv2);
    void GetObjectValMapInt64(std::any &anyVal, napi_value &tnv2);
    void GetObjectValMapDouble(std::any &anyVal, napi_value &tnv2);
    void GetObjectValMapBool(std::any &anyVal, napi_value &tnv2);
    void GetObjectValMapAny(std::any &anyVal, napi_value &tnv2);
    void GetObjectValue(napi_value &result, std::map<std::string, std::any> valueIn);

    napi_value SyncCallBack(napi_value func, size_t argc, napi_value *args);

    napi_value StartAsync(CallbackFunction pe, DataPtr data, CallbackFunction pc, napi_value func = nullptr);
    void FinishAsync(size_t argc, napi_value *args);

    bool IsFailed()
    {
        return bFailed_;
    }
    napi_value GetError()
    {
        return error_;
    }
    napi_env GetEnv()
    {
        return env_;
    }

    napi_value tmp_value;

    // create code related class
public:
    static void WrapFinalize(napi_env env, XNapiTool *data, DataPtr hint);
    void ReleaseInstance();
    napi_value WrapInstance(DataPtr instance, RELEASE_INSTANCE ri);
    DataPtr UnWarpInstance();
    
    void SetAsyncInstance(DataPtr p);
    void* GetAsyncInstance();

private:
    napi_env env_;
    napi_value exports_;

    static std::map<std::string, AsyncFunc> asyncFuncs_;

    // analyze params
    napi_value argv_[8];
    size_t argc_size;
    napi_value thisVar_;
    void *data_;

    // error message
    napi_value error_;
    bool bFailed_;
    bool CheckFailed(bool b, const char *errStr);
    bool CheckValueType(napi_value value, napi_valuetype type);

    // asynchronous call related code
    static void AsyncExecute(napi_env env, XNapiTool *p);
    void AsyncExecuteFunction();
    static void AsyncComplete(napi_env env, napi_status status, XNapiTool *p);
    void AsyncCompleteFunction();
    napi_ref callbackFunc_;
    napi_ref asyncThisVar_;
    napi_async_work work_;
    bool asyncNeedRelease_;
    CallbackFunction executeFunction_;
    CallbackFunction completeFunction_;
    DataPtr valueData_;
    napi_deferred deferred_;
    enum class AsyncMode {
        NONE,
        CALLBACK,
        PROMISE,
    };
    AsyncMode asyncMode_;

private:
    napi_ref wrapper_;
    DataPtr pInstance_;
    RELEASE_INSTANCE releaseInstance_;
    DataPtr asyncInstance_;
};

#endif
