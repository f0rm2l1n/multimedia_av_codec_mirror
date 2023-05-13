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

#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "i_avcodec_service.h"
#include "avcodeclist_impl.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecListImpl"};
}
namespace OHOS {
namespace Media {
std::shared_ptr<AVCodecList> AVCodecListFactory::CreateAVCodecList()
{
    std::shared_ptr<AVCodecListImpl> impl = std::make_shared<AVCodecListImpl>();

    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "failed to init AVCodecListImpl");

    return impl;
}

int32_t AVCodecListImpl::Init()
{
    codecListService_ = AVCodecServiceFactory::GetInstance().CreateCodecListService();
    CHECK_AND_RETURN_RET_LOG(codecListService_ != nullptr, AVCS_ERR_UNKNOWN, "failed to create AVCodecList service");
    return AVCS_ERR_OK;
}

AVCodecListImpl::AVCodecListImpl()
{
    AVCODEC_LOGD("AVCodecListImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecListImpl::~AVCodecListImpl()
{
    if (codecListService_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroyCodecListService(codecListService_);
        codecListService_ = nullptr;
    }
    AVCODEC_LOGD("AVCodecListImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

std::string AVCodecListImpl::FindDecoder(const Format &format)
{
    return codecListService_->FindDecoder(format);
}

std::string AVCodecListImpl::FindEncoder(const Format &format)
{
    return codecListService_->FindEncoder(format);
}

CapabilityData AVCodecListImpl::CreateCapability(std::string codecName)
{
    return codecListService_->CreateCapability(codecName);
}
} // namespace Media
} // namespace OHOS