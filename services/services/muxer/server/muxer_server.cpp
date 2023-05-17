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

#include "muxer_server.h"
#include <unistd.h>
#include <fcntl.h>
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "ipc_skeleton.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MuxerServer"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<IMuxerService> MuxerServer::Create()
{
    std::shared_ptr<MuxerServer> muxerServer = std::make_shared<MuxerServer>();
    return muxerServer;
}

MuxerServer::MuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
    appUid_ = IPCSkeleton::GetCallingUid();
    appPid_ = IPCSkeleton::GetCallingPid();
}

MuxerServer::~MuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    muxerEngine_ = nullptr;
}

int32_t MuxerServer::InitParameter(int32_t fd, OutputFormat format)
{
    muxerEngine_ = IMuxerEngineFactory::CreateMuxerEngine(appUid_, appPid_, fd, format);
    CHECK_AND_RETURN_RET_LOG(muxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Create muxer engine failed");
    return AVCS_ERR_OK;
}

int32_t MuxerServer::SetRotation(int32_t rotation)
{
    CHECK_AND_RETURN_RET_LOG(muxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "muxer engine does not exist");
    int32_t ret = muxerEngine_->SetRotation(rotation);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SetRotation");
    return AVCS_ERR_OK;
}

int32_t MuxerServer::AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc)
{
    CHECK_AND_RETURN_RET_LOG(muxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "muxer engine does not exist");
    int32_t ret = muxerEngine_->AddTrack(trackIndex, trackDesc);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call AddTrack");
    return AVCS_ERR_OK;
}

int32_t MuxerServer::Start()
{
    CHECK_AND_RETURN_RET_LOG(muxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "muxer engine does not exist");
    int32_t ret = muxerEngine_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call Start");
    return AVCS_ERR_OK;
}

int32_t MuxerServer::WriteSample(std::shared_ptr<AVSharedMemory> sample, const TrackSampleInfo &info)
{
    CHECK_AND_RETURN_RET_LOG(sample != nullptr, AVCS_ERR_INVALID_VAL, "sampleData is nullptr");
    CHECK_AND_RETURN_RET_LOG(muxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "muxer engine does not exist");
    int32_t ret = muxerEngine_->WriteSample(sample, info);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call WriteSample");
    return AVCS_ERR_OK;
}

int32_t MuxerServer::Stop()
{
    CHECK_AND_RETURN_RET_LOG(muxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "muxer engine does not exist");
    int32_t ret = muxerEngine_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call Stop");
    return AVCS_ERR_OK;
}

void MuxerServer::Release()
{
    CHECK_AND_RETURN_LOG(muxerEngine_ != nullptr, "muxer engine does not exist");
    (void)muxerEngine_->Stop();
    muxerEngine_ = nullptr;
}

int32_t MuxerServer::DumpInfo(int32_t fd)
{
    CHECK_AND_RETURN_RET_LOG(muxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "muxer engine does not exist");
    return muxerEngine_->DumpInfo(fd);
}
}  // namespace Media
}  // namespace OHOS