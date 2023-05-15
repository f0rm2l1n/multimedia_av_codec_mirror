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

#include "source_service_proxy.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "avsharedmemory_ipc.h"
#include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceServiceProxy"};
}

namespace OHOS {
namespace Media {
SourceServiceProxy::SourceServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardSourceService>(impl)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

SourceServiceProxy::~SourceServiceProxy()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t SourceServiceProxy::DestroyStub()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = Remote()->SendRequest(DESTROY_STUB, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call DestroyStub, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::Init(const std::string &uri)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteString(uri);
    int error = Remote()->SendRequest(INIT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call Init, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::GetTrackCount(uint32_t &trackCount)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteUint32(trackCount);
    int error = Remote()->SendRequest(GET_TRACK_COUNT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call GetTrackCount, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::SetTrackFormat(const Format &format, uint32_t trackIndex)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    AVCodecParcel::Marshalling(data, format);
    data.WriteUint32(trackIndex);
    int error = Remote()->SendRequest(SET_TRACK_FORMAT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call SetTrackFormat, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    data.WriteUint32(trackIndex);
    int error = Remote()->SendRequest(GET_TRACK_FORMAT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call GetTrackFormat, error: %{public}d", error);

    AVCodecParcel::Unmarshalling(reply, format);
    return reply.ReadInt32();
}

int32_t SourceServiceProxy::GetSourceFormat(Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = Remote()->SendRequest(GET_SOURCE_FORMAT, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call GetTrackFormat, error: %{public}d", error);

    AVCodecParcel::Unmarshalling(reply, format);
    return reply.ReadInt32();
}

uint64_t SourceServiceProxy::GetSourceAddr()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(SourceServiceProxy::GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, AVCS_ERR_INVALID_OPERATION, "Failed to write descriptor!");

    int error = Remote()->SendRequest(GET_SOURCE_ADDR, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == AVCS_ERR_OK, error, "Failed to call GetSourceAddr, error: %{public}d", error);
    return reply.ReadUint64();
}
}  // namespace Media
}  // namespace OHOS