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

#include "codeclist_client.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecListClient"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<CodecListClient> CodecListClient::Create(const sptr<IStandardCodecListService> &ipcProxy)
{
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, nullptr, "ipcProxy is nullptr..");

    std::shared_ptr<CodecListClient> codecList = std::make_shared<CodecListClient>(ipcProxy);

    return codecList;
}

CodecListClient::CodecListClient(const sptr<IStandardCodecListService> &ipcProxy)
    : codecListProxy_(ipcProxy)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecListClient::~CodecListClient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (codecListProxy_ != nullptr) {
        (void)codecListProxy_->DestroyStub();
    }
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void CodecListClient::AVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    codecListProxy_ = nullptr;
}

std::string CodecListClient::FindDecoder(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecListProxy_ != nullptr, "", "codeclist service does not exist.");
    return codecListProxy_->FindDecoder(format);
}

std::string CodecListClient::FindEncoder(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecListProxy_ != nullptr, "", "codeclist service does not exist.");
    return codecListProxy_->FindEncoder(format);
}

CapabilityData CodecListClient::CreateCapability(std::string codecName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CapabilityData capData;
    CHECK_AND_RETURN_RET_LOG(codecListProxy_ != nullptr, capData,
        "codeclist service does not exist.");
    return codecListProxy_->CreateCapability(codecName);
}
} // namespace Media
} // namespace OHOS
