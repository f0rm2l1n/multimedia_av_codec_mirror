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
#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include <node_api.h>
#include <any>
#include "tool_utility.h"
#include "napitest.h"

#define NUMBER_JS_2_C(napi_v, type, dest)        \
    if (typeid(type) == typeid(int32_t)) {       \
        dest = pxt->SwapJs2CInt32(napi_v);       \
    }                                            \
    else if (typeid(type) == typeid(uint32_t)) { \
        dest = pxt->SwapJs2CUint32(napi_v);      \
    }                                            \
    else if (typeid(type) == typeid(int64_t)) {  \
        dest = pxt->SwapJs2CInt64(napi_v);       \
    }                                            \
    else if (typeid(type) == typeid(double_t)) { \
        dest = pxt->SwapJs2CDouble(napi_v);      \
    }

#define NUMBER_JS_2_C_ENUM(napi_v, type, dest, enum_type)      \
    if (typeid(type) == typeid(int32_t))  {    \
        dest = static_cast<enum_type>(pxt->SwapJs2CInt32(napi_v));     \
    }                                           \
    else if (typeid(type) == typeid(uint32_t)) { \
        dest = static_cast<enum_type>(pxt->SwapJs2CUint32(napi_v));    \
    }                                          \
    else if (typeid(type) == typeid(int64_t)) { \
        dest = static_cast<enum_type>(pxt->SwapJs2CInt64(napi_v));     \
    }                                           \
    else if (typeid(type) == typeid(double_t)) { \
        dest = static_cast<enum_type>(pxt->SwapJs2CDouble(napi_v));    \
    }
    
#define BOOLEAN_JS_2_C(napi_v, type, dest) {    \
    dest = pxt->SwapC2JsBool(napi_v);          \
}

#define C_DELETE(p)  \
    if ((p)) {         \
        delete (p);    \
    }

__attribute__((unused)) static napi_value number_c_to_js(XNapiTool *pxt, const std::type_info &n, DataPtr num)
{
    if (n == typeid(int32_t))
        return pxt->SwapC2JsInt32(*(int32_t *)num);
    else if (n == typeid(uint32_t))
        return pxt->SwapC2JsUint32(*(uint32_t *)num);
    else if (n == typeid(int64_t))
        return pxt->SwapC2JsInt64(*(int64_t *)num);
    else if (n == typeid(double_t))
        return pxt->SwapC2JsDouble(*(double_t *)num);
    return nullptr;
}
#define NUMBER_C_2_JS(pxt, n) \
    number_c_to_js(pxt, typeid((n)), reinterpret_cast<DataPtr>(&(n)))

namespace napitest {
class AVFileDescriptor_middle {
public:
    static napi_value constructor(napi_env env, napi_callback_info info)
    {
        XNapiTool *pxt = new XNapiTool(env, info);
        AVFileDescriptor *p = new AVFileDescriptor();
        napi_value thisvar = pxt->WrapInstance(reinterpret_cast<DataPtr>(p), release);
        return thisvar;
    }
    static void release(DataPtr p)
    {
        void *dataPtr = p;
        AVFileDescriptor *p2 = static_cast<AVFileDescriptor *>(dataPtr);
        delete p2;
    }
    
    static napi_value getvalue_fd(napi_env env, napi_callback_info info)
    {
        XNapiTool *pxt = std::make_unique<XNapiTool>(env, info).release();
        void *instPtr = pxt->UnWarpInstance();
        AVFileDescriptor *p = static_cast<AVFileDescriptor *>(instPtr);
        napi_value result = nullptr;
        result = NUMBER_C_2_JS(pxt, p->fd);
        delete pxt;
        return result;
    }
    static napi_value setvalue_fd(napi_env env, napi_callback_info info)
    {
        std::shared_ptr<XNapiTool> pxt = std::make_shared<XNapiTool>(env, info);
        void *instPtr = pxt->UnWarpInstance();
        AVFileDescriptor *p = static_cast<AVFileDescriptor *>(instPtr);
        NUMBER_JS_2_C(pxt->GetArgv(XNapiTool::ZERO), int32_t, p->fd);
        return nullptr;
    }
    static napi_value getvalue_offset(napi_env env, napi_callback_info info)
    {
        XNapiTool *pxt = std::make_unique<XNapiTool>(env, info).release();
        void *instPtr = pxt->UnWarpInstance();
        AVFileDescriptor *p = static_cast<AVFileDescriptor *>(instPtr);
        napi_value result = nullptr;
        result = NUMBER_C_2_JS(pxt, p->offset);
        delete pxt;
        return result;
    }
    static napi_value setvalue_offset(napi_env env, napi_callback_info info)
    {
        std::shared_ptr<XNapiTool> pxt = std::make_shared<XNapiTool>(env, info);
        void *instPtr = pxt->UnWarpInstance();
        AVFileDescriptor *p = static_cast<AVFileDescriptor *>(instPtr);
        NUMBER_JS_2_C(pxt->GetArgv(XNapiTool::ZERO), int64_t, p->offset);
        return nullptr;
    }
    static napi_value getvalue_length(napi_env env, napi_callback_info info)
    {
        XNapiTool *pxt = std::make_unique<XNapiTool>(env, info).release();
        void *instPtr = pxt->UnWarpInstance();
        AVFileDescriptor *p = static_cast<AVFileDescriptor *>(instPtr);
        napi_value result = nullptr;
        result = NUMBER_C_2_JS(pxt, p->length);
        delete pxt;
        return result;
    }
    static napi_value setvalue_length(napi_env env, napi_callback_info info)
    {
        std::shared_ptr<XNapiTool> pxt = std::make_shared<XNapiTool>(env, info);
        void *instPtr = pxt->UnWarpInstance();
        AVFileDescriptor *p = static_cast<AVFileDescriptor *>(instPtr);
        NUMBER_JS_2_C(pxt->GetArgv(XNapiTool::ZERO), int64_t, p->length);
        return nullptr;
    }
};
struct setSurfaceID_value_struct {
    std::string in0;
    AVFileDescriptor in1;
    int32_t in2;
    int32_t in3;
    int32_t in4;
    uint32_t outErrCode = 0;
    std::string out;
};

void setSurfaceID_execute(XNapiTool *pxt, DataPtr data)
{
    void *data_ptr = data;
    setSurfaceID_value_struct *vio = static_cast<setSurfaceID_value_struct *>(data_ptr);
    
    setSurfaceID(vio->in0, vio->in1, vio->in2, vio->in3, vio->in4, vio->outErrCode, vio->out);
}

void setSurfaceID_complete(XNapiTool *pxt, DataPtr data)
{
    void *data_ptr = data;
    setSurfaceID_value_struct *vio = static_cast<setSurfaceID_value_struct *>(data_ptr);
    napi_value result = nullptr;
    result = pxt->SwapC2JsUtf8(vio->out.c_str());
    napi_value errCodeResult = nullptr;
    napi_value napiErrCode = nullptr;
    napiErrCode = NUMBER_C_2_JS(pxt, vio->outErrCode);
    pxt->SetValueProperty(errCodeResult, "code", napiErrCode);
    {
        napi_value args[XNapiTool::TWO] = {errCodeResult, result};
        pxt->FinishAsync(XNapiTool::TWO, args);
    }
    
    delete vio;
}

napi_value setSurfaceID_middle(napi_env env, napi_callback_info info)
{
    XNapiTool *pxt = std::make_unique<XNapiTool>(env, info).release();
    if (pxt->IsFailed()) {
        napi_value err = pxt->GetError();
        delete pxt;
        return err;
    }
    
    struct setSurfaceID_value_struct *vio = new setSurfaceID_value_struct();
        pxt->SwapJs2CUtf8(pxt->GetArgv(XNapiTool::ZERO), vio->in0);
    napi_value tnv1 = pxt->GetValueProperty(pxt->GetArgv(XNapiTool::ONE), "fd");
    if (tnv1 != nullptr) {NUMBER_JS_2_C(tnv1, int32_t, vio->in1.fd);}
napi_value tnv2 = pxt->GetValueProperty(pxt->GetArgv(XNapiTool::ONE), "offset");
    if (tnv2 != nullptr) {NUMBER_JS_2_C(tnv2, int64_t, vio->in1.offset);}
napi_value tnv3 = pxt->GetValueProperty(pxt->GetArgv(XNapiTool::ONE), "length");
    if (tnv3 != nullptr) {NUMBER_JS_2_C(tnv3, int64_t, vio->in1.length);}

    NUMBER_JS_2_C(pxt->GetArgv(XNapiTool::TWO), int32_t, vio->in2);
    NUMBER_JS_2_C(pxt->GetArgv(XNapiTool::THREE), int32_t, vio->in3);
    NUMBER_JS_2_C(pxt->GetArgv(XNapiTool::FOUE), int32_t, vio->in4);

    napi_value result = pxt->StartAsync(setSurfaceID_execute, reinterpret_cast<DataPtr>(vio), setSurfaceID_complete,
    pxt->GetArgc() == XNapiTool::SIX? pxt->GetArgv(XNapiTool::FIVE) : nullptr);
    if (pxt->IsFailed()) {
        result = pxt->GetError();
    }
    return result;
}
struct getSurfaceID_value_struct {
    uint32_t outErrCode = 0;
    std::string out;
};

void getSurfaceID_execute(XNapiTool *pxt, DataPtr data)
{
    void *data_ptr = data;
    getSurfaceID_value_struct *vio = static_cast<getSurfaceID_value_struct *>(data_ptr);
    
    getSurfaceID(vio->outErrCode, vio->out);
}

void getSurfaceID_complete(XNapiTool *pxt, DataPtr data)
{
    void *data_ptr = data;
    getSurfaceID_value_struct *vio = static_cast<getSurfaceID_value_struct *>(data_ptr);
    napi_value result = nullptr;
    result = pxt->SwapC2JsUtf8(vio->out.c_str());
    napi_value errCodeResult = nullptr;
    napi_value napiErrCode = nullptr;
    napiErrCode = NUMBER_C_2_JS(pxt, vio->outErrCode);
    pxt->SetValueProperty(errCodeResult, "code", napiErrCode);
    {
        napi_value args[XNapiTool::TWO] = {errCodeResult, result};
        pxt->FinishAsync(XNapiTool::TWO, args);
    }
    
    delete vio;
}

napi_value getSurfaceID_middle(napi_env env, napi_callback_info info)
{
    XNapiTool *pxt = std::make_unique<XNapiTool>(env, info).release();
    if (pxt->IsFailed()) {
        napi_value err = pxt->GetError();
        delete pxt;
        return err;
    }
    
    struct getSurfaceID_value_struct *vio = new getSurfaceID_value_struct();
    
    napi_value result = pxt->StartAsync(getSurfaceID_execute, reinterpret_cast<DataPtr>(vio), getSurfaceID_complete,
    pxt->GetArgc() == XNapiTool::ONE? pxt->GetArgv(XNapiTool::ZERO) : nullptr);
    if (pxt->IsFailed()) {
        result = pxt->GetError();
    }
    return result;
}}
static napi_value init(napi_env env, napi_value exports)
{
    std::shared_ptr<XNapiTool> pxt = std::make_shared<XNapiTool>(env, exports);
    {
    std::map<const char *, std::map<const char *, napi_callback>> valueList;
    valueList["fd"]["getvalue"] = napitest::AVFileDescriptor_middle::getvalue_fd;
    valueList["fd"]["setvalue"] = napitest::AVFileDescriptor_middle::setvalue_fd;
    valueList["offset"]["getvalue"] = napitest::AVFileDescriptor_middle::getvalue_offset;
    valueList["offset"]["setvalue"] = napitest::AVFileDescriptor_middle::setvalue_offset;
    valueList["length"]["getvalue"] = napitest::AVFileDescriptor_middle::getvalue_length;
    valueList["length"]["setvalue"] = napitest::AVFileDescriptor_middle::setvalue_length;
    std::map<const char *, napi_callback> funcList;
    pxt->DefineClass("AVFileDescriptor", napitest::AVFileDescriptor_middle::constructor,
        valueList, funcList);
}
    pxt->DefineFunction("setSurfaceID", napitest::setSurfaceID_middle);
    pxt->DefineFunction("getSurfaceID", napitest::getSurfaceID_middle);

    return exports;
}

static napi_module g_napitest_Module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = init,
    .nm_modname = "napitest",
    .nm_priv = ((void *)0),
    .reserved = {(void *)0},
};

extern "C" __attribute__((constructor)) void Register_napitest_Module(void)
{
    napi_module_register(&g_napitest_Module);
}
