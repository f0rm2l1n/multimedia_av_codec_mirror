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
#include "demuxer_server.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_dfx.h"
#include "ipc_skeleton.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "DemuxerServer"};
}

namespace OHOS {
namespace MediaAVCodec {
std::shared_ptr<IDemuxerService> DemuxerServer::Create()
{
    std::shared_ptr<DemuxerServer> server = std::make_shared<DemuxerServer>();
    CHECK_AND_RETURN_RET_LOG(server != nullptr, nullptr, "Demuxer Service does not exist");
    return server;
}

DemuxerServer::DemuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
    appUid_ = IPCSkeleton::GetCallingUid();
    appPid_ = IPCSkeleton::GetCallingPid();
}

DemuxerServer::~DemuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    demuxerEngine_ = nullptr;
}

int32_t DemuxerServer::Init(uintptr_t sourceAddr)
{
    demuxerEngine_ = IDemuxerEngineFactory::CreateDemuxerEngine(appUid_, appPid_, sourceAddr);
    CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine create failed");
    return AVCS_ERR_OK;
}

int32_t DemuxerServer::SelectTrackByID(uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");
    int32_t ret = demuxerEngine_->SelectTrackByID(trackIndex);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SelectTrackByID");
    return AVCS_ERR_OK;
}

int32_t DemuxerServer::UnselectTrackByID(uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");
    int32_t ret = demuxerEngine_->UnselectTrackByID(trackIndex);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call UnselectTrackByID");
    return AVCS_ERR_OK;
}

int32_t DemuxerServer::ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
    AVCodecBufferInfo &info, AVCodecBufferFlag &flag)
{
    CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");
    int32_t ret = demuxerEngine_->ReadSample(trackIndex, sample, info, flag);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call ReadSample");
    return AVCS_ERR_OK;
}

int32_t DemuxerServer::SeekToTime(int64_t millisecond, const AVSeekMode mode)
{
    CHECK_AND_RETURN_RET_LOG(demuxerEngine_ != nullptr, AVCS_ERR_INVALID_OPERATION, "Demuxer engine does not exist");
    int32_t ret = demuxerEngine_->SeekToTime(millisecond, mode);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call SeekToTime");
    return AVCS_ERR_OK;
}
}  // namespace MediaAVCodec
}  // namespace OHOS