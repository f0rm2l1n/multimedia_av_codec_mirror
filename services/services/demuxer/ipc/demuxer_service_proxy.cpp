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

int32_t DemuxerServiceProxy::Init(uint64_t sourceAddr)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteUint64(sourceAddr);
    int32_t error = Remote()->SendRequest(INIT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call Init, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t DemuxerServiceProxy::SelectSourceTrackByID(uint32_t trackIndex)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t error = Remote()->SendRequest(SELECT_SOURCE_TRACK_BY_ID, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error,
                           "Failed to call SelectSourceTrackByID, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t DemuxerServiceProxy::UnselectSourceTrackByID(uint32_t trackIndex)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t error = Remote()->SendRequest(UNSELECT_SOURCE_TRACK_BY_ID, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error,
                           "Failed to call UnselectSourceTrackByID, error: %{public}d", error);
    return reply.ReadInt32();
}
int32_t DemuxerServiceProxy::CopyNextSample(uint32_t &trackIndex, uint8_t *buffer,
                                            AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool token = data.WriteInterfaceToken(DemuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int32_t error = Remote()->SendRequest(COPY_NEXT_SAMPLE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call CopyNextSample, error: %{public}d", error);
    
    trackIndex = reply.ReadUint32();
    bufferInfo.presentationTimeUs = reply.ReadInt64();
    bufferInfo.size = reply.ReadInt32();
    bufferInfo.offset = reply.ReadInt32();
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

    int32_t error = Remote()->SendRequest(SEEK_TO_TIME, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call SeekToTime, error: %{public}d", error);
    return reply.ReadInt32();
}
}  // namespace Media
}  // namespace OHOS