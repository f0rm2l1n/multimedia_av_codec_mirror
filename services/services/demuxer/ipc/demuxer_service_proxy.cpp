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
#include "demuxer_service_proxy.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "avsharedmemory_ipc.h"
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "DemuxerServiceProxy"};
}

namespace OHOS {
namespace Media {
DemuxerServiceProxy::DemuxerServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardDemuxerService>(impl)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

DemuxerServiceProxy::~DemuxerServiceProxy()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t DemuxerServiceProxy::DestroyStub()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = Remote()->SendRequest(DESTROY_STUB, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call DestroyStub, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t DemuxerServiceProxy::Init(uintptr_t sourceAddr)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WritePointer(sourceAddr);
    int32_t error = Remote()->SendRequest(INIT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call Init, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t DemuxerServiceProxy::SelectTrackByID(uint32_t trackIndex)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");
    
    data.WriteUint32(trackIndex);
    int32_t error = Remote()->SendRequest(SELECT_TRACK_BY_ID, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error,
        "Failed to call SelectTrackByID, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t DemuxerServiceProxy::UnselectTrackByID(uint32_t trackIndex)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteUint32(trackIndex);
    int32_t error = Remote()->SendRequest(UNSELECT_TRACK_BY_ID, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error,
        "Failed to call UnselectTrackByID, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t DemuxerServiceProxy::ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
    AVCodecBufferInfo &info, AVCodecBufferFlag &flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");
    
    data.WriteUint32(trackIndex);
    WriteAVSharedMemoryToParcel(sample, data);
    int32_t error = Remote()->SendRequest(READ_SAMPLE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call ReadSample, error: %{public}d", error);
    
    sample = ReadAVSharedMemoryFromParcel(reply);
    info.presentationTimeUs = reply.ReadInt64();
    info.size = reply.ReadInt32();
    info.offset = reply.ReadInt32();
    flag = static_cast<enum AVCodecBufferFlag>(reply.ReadUint32());
    return reply.ReadInt32();
}
int32_t DemuxerServiceProxy::SeekToTime(int64_t mSeconds, const AVSeekMode mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteInt64(mSeconds);
    data.WriteInt32(static_cast<int32_t>(mode));
    int32_t error = Remote()->SendRequest(SEEK_TO_TIME, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call SeekToTime, error: %{public}d", error);
    return reply.ReadInt32();
}
}  // namespace Media
}  // namespace OHOS