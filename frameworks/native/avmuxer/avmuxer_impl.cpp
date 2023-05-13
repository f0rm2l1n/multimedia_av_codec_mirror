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

#include "avmuxer_impl.h"
#include <unistd.h>
#include <fcntl.h>
#include "securec.h"
#include "i_avcodec_service.h"
#include "avcodec_log.h"
#include "avsharedmemorybase.h"
#include "avcodec_dfx.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVMuxerImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVMuxer> AVMuxerFactory::CreateAVMuxer(int32_t fd, OutputFormat format)
{
    AVCodecTrace trace("AVMuxerFactory::CreateAVMuxer");
    CHECK_AND_RETURN_RET_LOG((fcntl(fd, F_GETFL, 0) & O_RDWR) == O_RDWR, nullptr, "No permission to read and write fd");
    CHECK_AND_RETURN_RET_LOG(lseek(fd, 0, SEEK_CUR) != -1, nullptr, "The fd is not seekable");

    std::shared_ptr<AVMuxerImpl> impl = std::make_shared<AVMuxerImpl>(fd, format);

    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init avmuxer implementation failed");
    return impl;
}

AVMuxerImpl::AVMuxerImpl(int32_t fd, OutputFormat format) : fd_(fd), format_(format)
{
    (void)fd_;
    (void)format_;
    AVCODEC_LOGD("AVMuxerImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVMuxerImpl::~AVMuxerImpl()
{
    if (muxerService_ != nullptr) {
        (void)muxerService_->Release();
        (void)AVCodecServiceFactory::GetInstance().DestroyMuxerService(muxerService_);
        muxerService_ = nullptr;
    }
    AVCODEC_LOGD("AVMuxerImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVMuxerImpl::Init()
{
    AVCodecTrace trace("AVMuxer::Init");
    AVCODEC_LOGI("Init");
    muxerService_ = AVCodecServiceFactory::GetInstance().CreateMuxerService();
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_NO_MEMORY, "Create AVMuxer Service failed");
    return muxerService_->InitParameter(fd_, format_);
}

int32_t AVMuxerImpl::SetLocation(float latitude, float longitude)
{
    AVCodecTrace trace("AVMuxer::SetLocation");
    AVCODEC_LOGI("SetLocation");
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Service does not exist");
    return muxerService_->SetLocation(latitude, longitude);
}

int32_t AVMuxerImpl::SetRotation(int32_t rotation)
{
    AVCodecTrace trace("AVMuxer::SetRotation");
    AVCODEC_LOGI("SetRotation");
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Service does not exist");
    return muxerService_->SetRotation(rotation);
}

int32_t AVMuxerImpl::AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc)
{
    AVCodecTrace trace("AVMuxer::AddTrack");
    AVCODEC_LOGI("AddTrack");
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Service does not exist");
    return muxerService_->AddTrack(trackIndex, trackDesc);
}

int32_t AVMuxerImpl::Start()
{
    AVCodecTrace trace("AVMuxer::Start");
    AVCODEC_LOGI("Start");
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Service does not exist");
    return muxerService_->Start();
}

int32_t AVMuxerImpl::WriteSampleBuffer(uint8_t *sampleBuffer, const TrackSampleInfo &info)
{
    AVCodecTrace trace("AVMuxer::WriteSampleBuffer");
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Service does not exist");
    CHECK_AND_RETURN_RET_LOG(sampleBuffer != nullptr && info.timeUs >= 0, AVCS_ERR_INVALID_VAL, "Invalid memory");

    std::shared_ptr<AVSharedMemoryBase> sharedSampleBuffer =
        std::make_shared<AVSharedMemoryBase>(info.size, AVSharedMemory::FLAGS_READ_ONLY, "sampleBuffer");
    int32_t ret = sharedSampleBuffer->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_NO_MEMORY, "create AVSharedMemoryBase failed");
    errno_t rc = memcpy_s(sharedSampleBuffer->GetBase(), sharedSampleBuffer->GetSize(), sampleBuffer, info.size);
    CHECK_AND_RETURN_RET_LOG(rc == EOK, AVCS_ERR_UNKNOWN, "memcpy_s failed");

    return muxerService_->WriteSampleBuffer(sharedSampleBuffer, info);
}

int32_t AVMuxerImpl::Stop()
{
    AVCodecTrace trace("AVMuxer::Stop");
    AVCODEC_LOGI("Stop");
    CHECK_AND_RETURN_RET_LOG(muxerService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Service does not exist");
    return muxerService_->Stop();
}
} // namespace Media
} // namespace OHOS