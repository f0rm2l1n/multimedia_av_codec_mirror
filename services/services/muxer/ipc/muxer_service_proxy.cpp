/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "muxer_service_proxy.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avsharedmemory_ipc.h"
#include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MuxerServiceProxy"};
}

namespace OHOS {
namespace Media {
MuxerServiceProxy::MuxerServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardMuxerService>(impl)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

MuxerServiceProxy::~MuxerServiceProxy()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t MuxerServiceProxy::InitParameter(int32_t fd, OutputFormat format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    
    bool token = data.WriteInterfaceToken(MuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Write descriptor failed!");

    CHECK_AND_RETURN_RET_LOG(data.WriteFileDescriptor(fd), AVCS_ERR_UNKNOWN, "WriteFileDescriptor failed!");
    CHECK_AND_RETURN_RET_LOG(data.WriteInt32(format), AVCS_ERR_UNKNOWN, "WriteInt32 failed!");

    int error = Remote()->SendRequest(INIT_PARAMETER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Call InitParameter failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t MuxerServiceProxy::SetLocation(float latitude, float longitude)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(MuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Write descriptor failed!!");

    CHECK_AND_RETURN_RET_LOG(data.WriteFloat(latitude), AVCS_ERR_UNKNOWN, "WriteFloat failed!");
    CHECK_AND_RETURN_RET_LOG(data.WriteFloat(longitude), AVCS_ERR_UNKNOWN, "WriteFloat failed!");

    int32_t ret = Remote()->SendRequest(SET_LOCATION, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "SetLocation failed, error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t MuxerServiceProxy::SetRotation(int32_t rotation)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(MuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Write descriptor failed!!");

    CHECK_AND_RETURN_RET_LOG(data.WriteInt32(rotation), AVCS_ERR_UNKNOWN, "WriteInt32 failed!");

    int32_t ret = Remote()->SendRequest(SET_ROTATION, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "SetRotation failed, error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t MuxerServiceProxy::AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(MuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Write descriptor failed!!");

    AVCodecParcel::Marshalling(data, trackDesc);

    int32_t ret = Remote()->SendRequest(ADD_TRACK, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "AddTrack failed, error: %{public}d", ret);
    trackIndex = reply.ReadInt32();
    return reply.ReadInt32();
}

int32_t MuxerServiceProxy::Start()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(MuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Write descriptor failed!!");

    int32_t ret = Remote()->SendRequest(START, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Start failed, error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t MuxerServiceProxy::WriteSampleBuffer(std::shared_ptr<AVSharedMemory> sampleBuffer, const TrackSampleInfo &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(MuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Write descriptor failed!!");

    WriteAVSharedMemoryToParcel(sampleBuffer, data);
    CHECK_AND_RETURN_RET_LOG(data.WriteUint32(info.trackIndex), AVCS_ERR_UNKNOWN, "Write track index failed!");
    CHECK_AND_RETURN_RET_LOG(data.WriteInt64(info.timeUs), AVCS_ERR_UNKNOWN, "Write timeUs failed!");
    CHECK_AND_RETURN_RET_LOG(data.WriteUint32(info.size), AVCS_ERR_UNKNOWN, "Write size failed!");
    CHECK_AND_RETURN_RET_LOG(data.WriteUint32(info.flags), AVCS_ERR_UNKNOWN, "Write flags failed!");

    int32_t ret = Remote()->SendRequest(WRITE_SAMPLE_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "WriteSampleBuffer failed, error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t MuxerServiceProxy::Stop()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(MuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Write descriptor failed!!");

    int32_t ret = Remote()->SendRequest(STOP, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Stop failed, error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t MuxerServiceProxy::DestroyStub()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(MuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Write descriptor failed!!");

    int ret = Remote()->SendRequest(DESTROY, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Call DestroyStub failed, error: %{public}d", ret);
    return reply.ReadInt32();
}

void MuxerServiceProxy::Release()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(MuxerServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_LOG(token, "Write descriptor failed!");

    int ret = Remote()->SendRequest(RELEASE, data, reply, option);
    CHECK_AND_RETURN_LOG(ret == AVCS_ERR_OK, " Call Release failed, error: %{public}d", ret);
}
}  // namespace Media
}  // namespace OHOS