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

#include "avdemuxer_impl.h"
#include <fcntl.h>
#include <unistd.h>
#include <functional>
#include <sys/types.h>
#include "securec.h"
#include "avcodec_log.h"
#include "avsharedmemorybase.h"
#include "avcodec_dfx.h"
#include "i_avcodec_service.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVDemuxerImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVDemuxer> AVDemuxerFactory::CreateWithSource(std::shared_ptr<AVSource> source)
{
    AVCodecTrace trace("AVDemuxerFactory::CreateWithSource");

    std::shared_ptr<AVDemuxerImpl> demuxerImpl = std::make_shared<AVDemuxerImpl>();
    CHECK_AND_RETURN_RET_LOG(demuxerImpl != nullptr, nullptr, "New AVDemuxerImpl failed when create demuxer");
    
    int32_t ret = demuxerImpl->Init(source);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init AVDemuxerImpl failed when create demuxer");

    return demuxerImpl;
}

int32_t AVDemuxerImpl::Init(std::shared_ptr<AVSource> source)
{
    AVCodecTrace trace("AVDemuxer::Init");

    CHECK_AND_RETURN_RET_LOG(source != nullptr, AVCS_ERR_INVALID_VAL,
                             "Create demuxer service failed because source is nullptr");
    AVCODEC_LOGI("create demuxerImpl from source %{private}s", source->sourceUri.c_str());

    demuxerClient_ = AVCodecServiceFactory::GetInstance().CreateDemuxerService();
    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr,
        AVCS_ERR_CREATE_DEMUXER_SUB_SERVICE_FAILED, "Create demuxer service failed when init demuxerImpl");
    
    uintptr_t sourceAddr;
    int32_t ret = source->GetSourceAddr(sourceAddr);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK,
        AVCS_ERR_CREATE_DEMUXER_SUB_SERVICE_FAILED, "Get source address failed when create demuxer service");
    sourceUri_ = source->sourceUri;

    return demuxerClient_->Init(sourceAddr);
}

AVDemuxerImpl::AVDemuxerImpl()
{
    AVCODEC_LOGI("init demuxerImpl");
    AVCODEC_LOGD("AVDemuxerImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVDemuxerImpl::~AVDemuxerImpl()
{
    AVCODEC_LOGI("uninit demuxerImpl for source %{private}s", sourceUri_.c_str());
    if (demuxerClient_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroyDemuxerService(demuxerClient_);
        demuxerClient_ = nullptr;
    }
    AVCODEC_LOGD("AVDemuxerImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVDemuxerImpl::SelectTrackByID(uint32_t trackIndex)
{
    AVCodecTrace trace("AVDemuxer::SelectTrackByID");

    AVCODEC_LOGI("select track: trackIndex=%{public}u", trackIndex);

    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "demuxer service died when select track by index!");
    return demuxerClient_->SelectTrackByID(trackIndex);
}

int32_t AVDemuxerImpl::UnselectTrackByID(uint32_t trackIndex)
{
    AVCodecTrace trace("AVDemuxer::UnselectTrackByID");

    AVCODEC_LOGI("unselect track: trackIndex=%{public}u", trackIndex);

    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "demuxer service died when unselect track by index!");
    return demuxerClient_->UnselectTrackByID(trackIndex);
}

int32_t AVDemuxerImpl::ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
    AVCodecBufferInfo &info, AVCodecBufferFlag &flag)
{
    AVCodecTrace trace("AVDemuxer::ReadSample");

    AVCODEC_LOGD("ReadSample: trackIndex=%{public}u", trackIndex);

    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION,
        "demuxer service died when read sample!");

    CHECK_AND_RETURN_RET_LOG(sample != nullptr, AVCS_ERR_INVALID_VAL,
        "Copy sample failed because sample buffer is nullptr!");

    return demuxerClient_->ReadSample(trackIndex, sample, info, flag);
}

int32_t AVDemuxerImpl::SeekToTime(int64_t millisecond, AVSeekMode mode)
{
    AVCodecTrace trace("AVDemuxer::SeekToTime");

    AVCODEC_LOGI("seek to time: millisecond=%{public}" PRId64 "; mode=%{public}d", millisecond, mode);

    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "demuxer service died when seek!");

    CHECK_AND_RETURN_RET_LOG(millisecond >= 0, AVCS_ERR_INVALID_VAL,
        "Seek failed because input millisecond is negative!");
    
    return demuxerClient_->SeekToTime(millisecond, mode);
}
} // namespace Media
} // namespace OHOS