/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "source_client.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceClient"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<SourceClient> SourceClient::Create(const sptr<IStandardSourceService> &ipcProxy)
{
    std::shared_ptr<SourceClient> source = std::make_shared<SourceClient>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "Failed to create source client");
    return source;
}

SourceClient::SourceClient(const sptr<IStandardSourceService> &ipcProxy)
    : sourceProxy_(ipcProxy)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

SourceClient::~SourceClient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sourceProxy_ != nullptr) {
        (void)sourceProxy_->DestroyStub();
        sourceProxy_ = nullptr;
    }
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void SourceClient::AVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    sourceProxy_ = nullptr;
}

int32_t SourceClient::Init(const std::string &uri)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "source service does not exist.");

    AVCODEC_LOGD("source client call Init");
    return sourceProxy_->Init(uri);
}

int32_t SourceClient::GetTrackCount(uint32_t &trackCount)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "source service does not exist.");

    AVCODEC_LOGD("source client call GetTrackCount");
    return sourceProxy_->GetTrackCount(trackCount);
}

int32_t SourceClient::SetTrackFormat(const Format &format, uint32_t trackIndex)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "source service does not exist.");

    AVCODEC_LOGD("source client call SetTrackFormat");
    return sourceProxy_->SetTrackFormat(format, trackIndex);
}

int32_t SourceClient::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "source service does not exist.");

    AVCODEC_LOGD("source client call GetTrackFormat");
    return sourceProxy_->GetTrackFormat(format, trackIndex);
}

int32_t SourceClient::GetSourceFormat(Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "source service does not exist.");

    AVCODEC_LOGD("source client call GetSourceFormat");
    return sourceProxy_->GetSourceFormat(format);
}

uint64_t SourceClient::GetSourceAddr()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, AVCS_ERR_NO_MEMORY, "source service does not exist.");

    AVCODEC_LOGD("source client call GetSourceAddr");
    return sourceProxy_->GetSourceAddr();
}
}  // namespace Media
}  // namespace OHOS