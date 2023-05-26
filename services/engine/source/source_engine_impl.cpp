/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "source_engine_impl.h"
#include <set>
#include <fcntl.h>
#include <unistd.h>
#include "securec.h"
#include "source.h"
#include "avcodec_log.h"
#include "avcodec_dfx.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceEngineImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<ISourceEngine> ISourceEngineFactory::CreateSourceEngine(int32_t appUid, int32_t appPid,
                                                                        const std::string& uri)
{
    AVCodecTrace trace("ISourceEngineFactory::CreateSourceEngine");
    std::shared_ptr<ISourceEngine> sourceEngineImpl = std::make_shared<SourceEngineImpl>(appUid, appPid, uri);
    CHECK_AND_RETURN_RET_LOG(sourceEngineImpl != nullptr, nullptr, "create SourceEngine implementation failed");
    return sourceEngineImpl;
}

int32_t SourceEngineImpl::Create()
{
    AVCodecTrace trace("SourceEngineImpl::Create");
    AVCODEC_LOGI("Create");
    std::unique_lock<std::mutex> lock(mutex_);
    return source_->Create(uri_);
}

SourceEngineImpl::SourceEngineImpl(int32_t appUid, int32_t appPid, const std::string& uri)
    : appUid_(appUid), appPid_(appPid), uri_(uri)
{
    AVCODEC_LOGI("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
    source_ = std::make_shared<Plugin::Source>();
    if (source_ == nullptr) {
        AVCODEC_LOGE("create demuxer engine failed");
    }
}

SourceEngineImpl::~SourceEngineImpl()
{
    AVCODEC_LOGD("Destroy SourceEngineImpl");
    appUid_ = -1;
    appPid_ = -1;
    source_ = nullptr;
    AVCODEC_LOGI("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t SourceEngineImpl::GetTrackCount(uint32_t &trackCount)
{
    AVCodecTrace trace("SourceEngineImpl::GetTrackCount");
    AVCODEC_LOGI("GetTrackCount");
    std::unique_lock<std::mutex> lock(mutex_);
    return source_->GetTrackCount(trackCount);
}

int32_t SourceEngineImpl::GetSourceFormat(Format &format)
{
    AVCodecTrace trace("SourceEngineImpl::GetSourceFormat");
    AVCODEC_LOGI("GetSourceFormat");
    std::unique_lock<std::mutex> lock(mutex_);
    return source_->GetSourceFormat(format);
}

int32_t SourceEngineImpl::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    AVCodecTrace trace("SourceEngineImpl::GetTrackFormat");
    AVCODEC_LOGI("GetTrackFormat");
    std::unique_lock<std::mutex> lock(mutex_);
    return source_->GetTrackFormat(format, trackIndex);
}

uintptr_t SourceEngineImpl::GetSourceAddr()
{
    AVCodecTrace trace("SourceEngineImpl::GetSourceAddr");
    AVCODEC_LOGI("GetSourceAddr");
    std::unique_lock<std::mutex> lock(mutex_);
    return source_->GetSourceAddr();
}
} // Media
} // OHOS
