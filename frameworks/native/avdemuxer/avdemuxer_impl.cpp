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

static int LOOP_LOG_MAX_COUNT = 1000;

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVDemuxerImpl"};
}

namespace OHOS {
namespace Media{


std::shared_ptr<AVDemuxer> AVDemuxerFactory::CreateWithSource(AVSource &source)
{
    AVCodecTrace trace("AVDemuxerFactory::CreateWithSource");

    AVCODEC_LOGI("create demuxerImpl from source %{public}s", source.sourceUri.c_str());

    std::shared_ptr<AVDemuxerImpl> demuxerImpl = std::make_shared<AVDemuxerImpl>();
    CHECK_AND_RETURN_RET_LOG(demuxerImpl != nullptr, nullptr, "New AVDemuxerImpl failed when create demuxer");
    
    int32_t ret = demuxerImpl->Init(source);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init AVDemuxerImpl failed when create demuxer");

    return demuxerImpl;
}

int32_t AVDemuxerImpl::Init(AVSource &source)
{
    AVCodecTrace trace("AVDemuxer::Init");

    demuxerClient_ = AVCodecServiceFactory::GetInstance().CreateDemuxerService();
    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, 
        AVCS_ERR_CREATE_DEMUXER_SUB_SERVICE_FAILED, "Create demuxer service failed when init demuxerImpl");

    uintptr_t sourceAddr = source.GetSourceAddr();

    sourceUri_ = source.sourceUri;
    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "demuxer service died when load add sourceTrack!");
    
    return demuxerClient_->Init(sourceAddr);
}

AVDemuxerImpl::AVDemuxerImpl()
{
    AVCODEC_LOGI("init demuxerImpl");
    AVCODEC_LOGD("AVDemuxerImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVDemuxerImpl::~AVDemuxerImpl()
{
    AVCODEC_LOGI("uninit demuxerImpl for source %{public}s", sourceUri_.c_str());
    if (demuxerClient_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroyDemuxerService(demuxerClient_);
        demuxerClient_ = nullptr;
    }
    AVCODEC_LOGD("AVDemuxerImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVDemuxerImpl::SelectSourceTrackByID(uint32_t trackIndex)
{
    AVCodecTrace trace("AVDemuxer::SelectSourceTrackByID");

    AVCODEC_LOGI("select source track: trackIndex=%{public}d", trackIndex);

    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "demuxer service died when load add sourceTrack!");
    return demuxerClient_->SelectSourceTrackByID(trackIndex);
}

int32_t AVDemuxerImpl::UnselectSourceTrackByID(uint32_t trackIndex)
{
    AVCodecTrace trace("AVDemuxer::UnselectSourceTrackByID");

    AVCODEC_LOGI("unselect source track: trackIndex=%{public}d", trackIndex);

    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "demuxer service died when remove sourceTrack!");
    return demuxerClient_->UnselectSourceTrackByID(trackIndex);
}

int32_t AVDemuxerImpl::CopyNextSample(uint32_t &trackIndex, uint8_t *buffer, AVCodecBufferInfo &bufferInfo,AVCodecBufferFlag &flag)
{
    AVCodecTrace trace("AVDemuxer::CopyNextSample");

    AVCODEC_LOGI("CopyNextSample");

    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "demuxer service died when copy sample!");
    
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, AVCS_ERR_INVALID_VAL, "Copy sample failed because input buffer is nullptr!");

    if ( trackLogCount < LOOP_LOG_MAX_COUNT ) {
        if ( trackLogCount==0 ) {
            AVCodecTrace::TraceBegin(std::string(__FUNCTION__), FAKE_POINTER(this));
        }
        trackLogCount++;
    }

    std::shared_ptr<AVSharedMemoryBase> memory = 
        std::make_shared<AVSharedMemoryBase>(bufferInfo.size, AVSharedMemory::FLAGS_READ_WRITE, "sampleBuffer");
    int32_t ret = memory->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_NO_MEMORY, "Copy sample failed by demuxerService!");

    ret = demuxerClient_->CopyNextSample(trackIndex, memory->GetBase(), bufferInfo,flag);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_INVALID_OPERATION, "Copy sample failed by demuxerService!");
    
    errno_t rc = memcpy_s(buffer, memory->GetSize(), memory->GetBase(), memory->GetSize());
    CHECK_AND_RETURN_RET_LOG(rc == EOK, AVCS_ERR_UNKNOWN, "memcpy_s failed");

    if ( trackLogCount == LOOP_LOG_MAX_COUNT ) {
        AVCodecTrace::TraceEnd(std::string(__FUNCTION__), FAKE_POINTER(this));
    }
    return AVCS_ERR_OK;
}

int32_t AVDemuxerImpl::SeekToTime(int64_t mSeconds, AVSeekMode mode)
{
    AVCodecTrace trace("AVDemuxer::SeekToTime");

    AVCODEC_LOGI("seek to time: mSeconds=%{public}lld; mode=%{public}d", mSeconds, mode);

    CHECK_AND_RETURN_RET_LOG(demuxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "demuxer service died when seek!");

    CHECK_AND_RETURN_RET_LOG(mSeconds >= 0, AVCS_ERR_INVALID_VAL, "Seek failed because input mSeconds is negative!");
    
    return demuxerClient_->SeekToTime(mSeconds, mode);
}

} // namespace Media
} // namespace OHOS