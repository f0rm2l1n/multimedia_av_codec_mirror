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

#include "codec_ability_singleton.h"
#include "codeclist_xml_parser.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecAbilitySingleton"};
}

namespace OHOS {
namespace Media {

CodecAbilitySingleton& CodecAbilitySingleton::GetInstance()
{
    AVCODEC_LOGE("CodecAbilitySingleton entered: start getting ins");
    static CodecAbilitySingleton instance;
    bool ret = instance.ParseCodecXml();
    if(!ret){
        AVCODEC_LOGE("Parse codec xml failed");
    }
    return instance;
}

bool CodecAbilitySingleton::ParseCodecXml()
{
    AVCODEC_LOGE("start parsing xml");
    std::lock_guard<std::mutex> lock(mutex_);
    if (isParsered_) {
        AVCODEC_LOGE("Parse codec xml done");
        return true;
    }
    CodeclistXmlParser xmlParser;
    bool ret = xmlParser.LoadConfiguration();
    if (!ret) {
        this->isParsered_ = false;
        AVCODEC_LOGE("AVCodecList LoadConfiguration failed");
        return false;
    }
    ret = xmlParser.Parse();
    if (!ret) {
        isParsered_ = false;
        AVCODEC_LOGE("AVCodecList Parse failed.");
        return false;
    }
    std::vector<CapabilityData> data = xmlParser.GetCapabilityDataArray();
    capabilityDataArray_.insert(capabilityDataArray_.end(), data.begin(), data.end());
    isParsered_ = true;
    AVCODEC_LOGE("Parse codec xml successful, num = %{public}d", capabilityDataArray_.size());
    return true;
}

CodecAbilitySingleton::CodecAbilitySingleton()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecAbilitySingleton::~CodecAbilitySingleton()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void CodecAbilitySingleton::RegisterCapabilityArray(const std::vector<CapabilityData> &capaArray)
{
    std::lock_guard<std::mutex> lock(mutex_);
    capabilityDataArray_.insert(capabilityDataArray_.end(), capaArray.begin(),
        capaArray.end());
    AVCODEC_LOGD("RegisterCapability success");
}

std::vector<CapabilityData> CodecAbilitySingleton::GetCapabilityArray()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return capabilityDataArray_;
}
// #endif
} // namespace Media
} // namespace OHOS
