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

#include "demuxer_engine_impl.h"
#include "securec.h"
#include "avcodec_log.h"
#include "demuxer_factory.h"
#include "avcodec_errors.h"
#include "iostream"
#include "avcodec_dfx.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "DemuxerEngineImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<IDemuxerEngine> IDemuxerEngineFactory::CreateDemuxerEngine(int32_t appUid, int32_t appPid,
                                                                            uintptr_t sourceAddr)
{
    AVCodecTrace trace("IDemuxerEngineFactory::CreateDemuxerEngine");
    std::shared_ptr<IDemuxerEngine> demuxerEngineImpl = 
        std::make_shared<DemuxerEngineImpl>(appUid, appPid, sourceAddr);
    CHECK_AND_RETURN_RET_LOG(demuxerEngineImpl != nullptr, nullptr, "create MuxerEngine implementation failed");
    return demuxerEngineImpl;
}

DemuxerEngineImpl::DemuxerEngineImpl(int32_t appUid, int32_t appPid, uintptr_t sourceAddr)
    : appUid_(appUid), appPid_(appPid), sourceAddr_(sourceAddr)
{
    AVCodecTrace trace("DemuxerEngineImpl::Create");
    AVCODEC_LOGI("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
    demuxer_ = Plugin::DemuxerFactory::Instance().CreatePlugin(sourceAddr_);
    if (demuxer_ != nullptr) {
        AVCODEC_LOGI("init demuxer engine successful");
    } else {
        AVCODEC_LOGE("init demuxer engine failed");
    }
}

DemuxerEngineImpl::~DemuxerEngineImpl()
{
    AVCODEC_LOGD("Destroy");
    appUid_ = -1;
    appPid_ = -1;
    demuxer_ = nullptr;

    AVCODEC_LOGI("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t DemuxerEngineImpl::SelectSourceTrackByID(uint32_t trackIndex)
{
    AVCodecTrace trace("DemuxerEngineImpl::SelectSourceTrackByID");
    AVCODEC_LOGI("SelectSourceTrackByID");
    return demuxer_->SelectSourceTrackByID(trackIndex);
}

int32_t DemuxerEngineImpl::UnselectSourceTrackByID(uint32_t trackIndex)
{
    AVCodecTrace trace("DemuxerEngineImpl::UnselectSourceTrackByID");
    AVCODEC_LOGI("UnselectSourceTrackByID");
    return demuxer_->UnselectSourceTrackByID(trackIndex);
}

int32_t DemuxerEngineImpl::CopyNextSample(uint32_t &trackIndex, uint8_t *buffer, AVCodecBufferInfo &bufferInfo,
                                        AVCodecBufferFlag &flag)
{
    AVCodecTrace trace("DemuxerEngineImpl::CopyNextSample");
    AVCODEC_LOGI("CopyNextSample");
    return demuxer_->CopyNextSample(trackIndex, buffer, bufferInfo, flag);
}

int32_t DemuxerEngineImpl::SeekToTime(int64_t mSeconds, AVSeekMode mode)
{
    AVCodecTrace trace("DemuxerEngineImpl::SeekToTime");
    AVCODEC_LOGI("SeekToTime");
    return demuxer_->SeekToTime(mSeconds, mode);
}
} // Media
} // OHOS
