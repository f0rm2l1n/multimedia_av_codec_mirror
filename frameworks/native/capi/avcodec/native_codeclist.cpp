/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "native_avmagic.h"
#include "avcodec_list.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "native_avcodec_list.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NativeCodecList"};
}

using namespace OHOS::Media;
const char *OH_AVCodec_FindEncoder(const OH_AVFormat *format)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    std::string strname = codeclist->FindEncoder(format->format_);
    char *ret = new char[strname.length() + 1];
    std::strcpy(ret, strname.c_str());
    AVCODEC_LOGD("get codecname: %{public}s", ret);
    return ret;
}

const char *OH_AVCodec_FindDecoder(const OH_AVFormat *format)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    std::string strname = codeclist->FindDecoder(format->format_);
    char *ret = new char[strname.length() + 1];
    std::strcpy(ret, strname.c_str());
    AVCODEC_LOGD("get codecname: %{public}s", ret);
    return ret;
}

OH_AVCapability *OH_AVCodec_CreateCapability(const char *name)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    CapabilityData capabilityData = codeclist->CreateCapability(name);
    return new (std::nothrow) OH_AVCapability(capabilityData);
}

OH_AVErrCode OH_AVCodec_DestroyCapability(OH_AVCapability *capability)
{
    if (capability == nullptr) {
        return AV_ERR_INVALID_VAL;
    }
    delete capability;
    return AV_ERR_OK;
}
