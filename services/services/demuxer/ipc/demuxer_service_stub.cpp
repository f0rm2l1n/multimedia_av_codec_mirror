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
#include "demuxer_service_stub.h"
#include <unistd.h>
#include "avcodec_server_manager.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avsharedmemory_ipc.h"
#include "avcodec_xcollie.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "DemuxerServiceStub"};
}

namespace OHOS {
namespace Media {
sptr<DemuxerServiceStub> DemuxerServiceStub::Create()
{
    sptr<DemuxerServiceStub> demuxerStub = new(std::nothrow) DemuxerServiceStub();
    CHECK_AND_RETURN_RET_LOG(demuxerStub != nullptr, nullptr, "Failed to create demuxer service stub");

    int32_t ret = demuxerStub->InitStub();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init DemuxerServiceStub");
    return demuxerStub;
}

DemuxerServiceStub::DemuxerServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

DemuxerServiceStub::~DemuxerServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t DemuxerServiceStub::InitStub()
{
    demuxerServer_ = DemuxerServer::Create();
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Failed to create muxer server");

    demuxerFuncs_[INIT] = &DemuxerServiceStub::Init;
    demuxerFuncs_[SELECT_SOURCE_TRACK_BY_ID] = &DemuxerServiceStub::SelectSourceTrackByID;
    demuxerFuncs_[UNSELECT_SOURCE_TRACK_BY_ID] = &DemuxerServiceStub::UnselectSourceTrackByID;
    demuxerFuncs_[COPY_NEXT_SAMPLE] = &DemuxerServiceStub::CopyNextSample;
    demuxerFuncs_[SEEK_TO_TIME] = &DemuxerServiceStub::SeekToTime;

    demuxerFuncs_[DESTROY_STUB] = &DemuxerServiceStub::DestroyStub;

    return AVCS_ERR_OK;
}

int DemuxerServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    AVCODEC_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (DemuxerServiceStub::GetDescriptor() != remoteDescriptor) {
        AVCODEC_LOGE("Invalid descriptor");
        return AVCS_ERR_INVALID_OPERATION;
    }

    auto itFunc = demuxerFuncs_.find(code);
    if (itFunc != demuxerFuncs_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            int32_t ret = -1;
            COLLIE_LISTEN(ret = (this->*memberFunc)(data, reply),
                "DemuxerServiceStub::OnRemoteRequest");
            CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call memberFunc");
            return AVCS_ERR_OK;
        }
    }
    AVCODEC_LOGW("Failed to find corresponding function");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DemuxerServiceStub::DestroyStub()
{
    demuxerServer_ = nullptr;
    AVCodecServerManager::GetInstance().DestroyStubObject(AVCodecServerManager::DEMUXER, AsObject());
    return AVCS_ERR_OK;
}

int32_t DemuxerServiceStub::Init(uint64_t sourceAddr)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->Init(sourceAddr);
}

int32_t DemuxerServiceStub::SelectSourceTrackByID(uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->SelectSourceTrackByID(trackIndex);
}

int32_t DemuxerServiceStub::UnselectSourceTrackByID(uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->UnselectSourceTrackByID(trackIndex);
}

int32_t DemuxerServiceStub::CopyNextSample(uint32_t &trackIndex, uint8_t *buffer,
                                         AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->CopyNextSample(trackIndex, buffer, bufferInfo, flag);
}

int32_t DemuxerServiceStub::SeekToTime(int64_t mSeconds, const AVSeekMode mode)
{
    CHECK_AND_RETURN_RET_LOG(demuxerServer_ != nullptr, AVCS_ERR_NO_MEMORY, "demuxer service is nullptr");
    return demuxerServer_->SeekToTime(mSeconds, mode);
}

int32_t DemuxerServiceStub::DumpInfo(int32_t fd)
{
    std::string dumpInfo;
    dumpInfo += "# DemuxerServiceStub \n";
    GetDumpInfo(dumpInfo);

    CHECK_AND_RETURN_RET_LOG(fd != -1, AVCS_ERR_INVALID_VAL, "Attempt to write to a invalid fd: %{public}d", fd);
    write(fd, dumpInfo.c_str(), dumpInfo.size());

    return AVCS_ERR_OK;
}

int32_t DemuxerServiceStub::Init(MessageParcel &data, MessageParcel &reply)
{
    uint64_t sourceAddr = data.ReadUint64();
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(Init(sourceAddr)), AVCS_ERR_UNKNOWN, "Reply Init failed!");
    return AVCS_ERR_OK;
}

int32_t DemuxerServiceStub::SelectSourceTrackByID(MessageParcel &data, MessageParcel &reply)
{
    uint32_t trackIndex = data.ReadUint32();

    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(SelectSourceTrackByID(trackIndex)), AVCS_ERR_UNKNOWN,
                           "Reply SelectSourceTrackByID failed!");
    return AVCS_ERR_OK;
}

int32_t DemuxerServiceStub::UnselectSourceTrackByID(MessageParcel &data, MessageParcel &reply)
{
    uint32_t trackIndex = data.ReadUint32();

    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(UnselectSourceTrackByID(trackIndex)), AVCS_ERR_UNKNOWN,
                           "Reply UnselectSourceTrackByID failed!");
    return AVCS_ERR_OK;
}

int32_t DemuxerServiceStub::CopyNextSample(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    uint32_t trackIndex;
    AVCodecBufferInfo info;
    AVCodecBufferFlag flag;
    uint8_t *buffer = nullptr;
    
    int32_t ret = CopyNextSample(trackIndex, buffer, info, flag);

    CHECK_AND_RETURN_RET_LOG(reply.WriteUint32(trackIndex), AVCS_ERR_UNKNOWN, "Reply CopyNextSample failed!");
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt64(info.presentationTimeUs), AVCS_ERR_UNKNOWN,
                           "Reply CopyNextSample failed!");
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(info.size), AVCS_ERR_UNKNOWN, "Reply CopyNextSample failed!");
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(info.offset), AVCS_ERR_UNKNOWN, "Reply CopyNextSample failed!");
    CHECK_AND_RETURN_RET_LOG(reply.WriteUint32(static_cast<uint32_t>(flag)), AVCS_ERR_UNKNOWN,
                           "Reply CopyNextSample failed!");
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(ret), AVCS_ERR_UNKNOWN, "Reply CopyNextSample failed!");
    return AVCS_ERR_OK;
}

int32_t DemuxerServiceStub::SeekToTime(MessageParcel &data, MessageParcel &reply)
{
    int64_t mSeconds = data.ReadInt64();
    AVSeekMode seekMode = static_cast<AVSeekMode>(data.ReadInt32());

    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(SeekToTime(mSeconds, seekMode)),
                           AVCS_ERR_UNKNOWN, "Reply SeekToTime failed!");
    return AVCS_ERR_OK;
}

int32_t DemuxerServiceStub::DestroyStub(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(DestroyStub()), AVCS_ERR_UNKNOWN, "Reply DestroyStub failed!");
    return AVCS_ERR_OK;
}

int32_t DemuxerServiceStub::GetDumpInfo(std::string& dumpInfo)
{
    dumpInfo += "## pid: " + std::to_string(getpid());
    dumpInfo += "## uid: " + std::to_string(getuid());
    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS