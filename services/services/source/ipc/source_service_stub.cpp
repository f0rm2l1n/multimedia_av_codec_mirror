/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "source_service_stub.h"
#include "unistd.h"
#include "avcodec_server_manager.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_dfx.h"
#include "avsharedmemory_ipc.h"
#include "avcodec_parcel.h"
#include "avcodec_xcollie.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceAVServiceStub"};
}

namespace OHOS {
namespace Media {
sptr<SourceServiceStub> SourceServiceStub::Create()
{
    sptr<SourceServiceStub> sourceStub = new(std::nothrow) SourceServiceStub();
    CHECK_AND_RETURN_RET_LOG(sourceStub != nullptr, nullptr, "Failed to create source service stub");

    int32_t ret = sourceStub->InitStub();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init SourceServiceStub");
    return sourceStub;
}

SourceServiceStub::SourceServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

SourceServiceStub::~SourceServiceStub()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t SourceServiceStub::InitStub()
{
    sourceServer_ = SourceServer::Create();
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Failed to create source server");

    sourceFuncs_[INIT] = &SourceServiceStub::Init;
    sourceFuncs_[GET_TRACK_COUNT] = &SourceServiceStub::GetTrackCount;
    sourceFuncs_[SET_TRACK_FORMAT] = &SourceServiceStub::SetTrackFormat;
    sourceFuncs_[GET_TRACK_FORMAT] = &SourceServiceStub::GetTrackFormat;
    sourceFuncs_[GET_SOURCE_FORMAT] = &SourceServiceStub::GetSourceFormat;
    sourceFuncs_[GET_SOURCE_ADDR] = &SourceServiceStub::GetSourceAddr;
    sourceFuncs_[DESTROY_STUB] = &SourceServiceStub::DestroyStub;
    return AVCS_ERR_OK;
}

int SourceServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    AVCODEC_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (SourceServiceStub::GetDescriptor() != remoteDescriptor) {
        AVCODEC_LOGE("Invalid descriptor");
        return AVCS_ERR_INVALID_OPERATION;
    }

    auto itFunc = sourceFuncs_.find(code);
    if (itFunc != sourceFuncs_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            int32_t ret = -1;
            COLLIE_LISTEN(ret = (this->*memberFunc)(data, reply),
                "SourceServiceStub::OnRemoteRequest");
            CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Failed to call memberFunc");
            return AVCS_ERR_OK;
        }
    }
    AVCODEC_LOGW("Failed to find corresponding function");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t SourceServiceStub::DestroyStub()
{
    sourceServer_ = nullptr;
    AVCodecServerManager::GetInstance().DestroyStubObject(AVCodecServerManager::SOURCE, AsObject());
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::Init(const std::string &uri)
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, AVCS_ERR_NO_MEMORY, "source server is nullptr");
    return sourceServer_->Init(uri);
}

int32_t SourceServiceStub::GetTrackCount(uint32_t &trackCount)
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, AVCS_ERR_NO_MEMORY, "source server is nullptr");
    return sourceServer_->GetTrackCount(trackCount);
}

int32_t SourceServiceStub::SetTrackFormat(const Format &format, uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, AVCS_ERR_NO_MEMORY, "source server is nullptr");
    return sourceServer_->SetTrackFormat(format, trackIndex);
}

int32_t SourceServiceStub::GetTrackFormat(Format &format, uint32_t trackIndex)
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, AVCS_ERR_NO_MEMORY, "source server is nullptr");
    return sourceServer_->GetTrackFormat(format, trackIndex);
}

int32_t SourceServiceStub::GetSourceFormat(Format &format)
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, AVCS_ERR_NO_MEMORY, "source server is nullptr");
    return sourceServer_->GetSourceFormat(format);
}

uint64_t SourceServiceStub::GetSourceAddr()
{
    CHECK_AND_RETURN_RET_LOG(sourceServer_ != nullptr, AVCS_ERR_NO_MEMORY, "source server is nullptr");
    return sourceServer_->GetSourceAddr();
}

int32_t SourceServiceStub::DumpInfo(int32_t fd)
{
    std::string dumpInfo;
    GetDumpInfo(dumpInfo);

    CHECK_AND_RETURN_RET_LOG(fd != -1, AVCS_ERR_INVALID_VAL, "Attempt to write to a invalid fd: %{public}d", fd);
    write(fd, dumpInfo.c_str(), dumpInfo.size());

    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::DestroyStub(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(DestroyStub()), AVCS_ERR_UNKNOWN, "Reply DestroyStub failed!");
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::Init(MessageParcel &data, MessageParcel &reply)
{
    std::string uri = data.ReadString();
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(Init(uri)), AVCS_ERR_UNKNOWN, "Reply Init failed");
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::GetTrackCount(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    uint32_t trackCount;
    int32_t ret = GetTrackCount(trackCount);
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(trackCount), AVCS_ERR_UNKNOWN, "Reply GetTrackCount failed");
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(ret), AVCS_ERR_UNKNOWN, "Reply GetTrackCount failed");
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::SetTrackFormat(MessageParcel &data, MessageParcel &reply)
{
    Format param;
    (void)AVCodecParcel::Unmarshalling(data, param);
    uint32_t trackIndex = data.ReadUint32();
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(SetTrackFormat(param, trackIndex)), AVCS_ERR_UNKNOWN,
                            "Reply SetTrackFormat failed");
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::GetTrackFormat(MessageParcel &data, MessageParcel &reply)
{
    uint32_t trackIndex = data.ReadUint32();
    Format format;
    int32_t ret = GetTrackFormat(format, trackIndex);
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(ret), AVCS_ERR_UNKNOWN, "Reply GetTrackFormat failed");
    AVCodecParcel::Marshalling(reply, format);
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::GetSourceFormat(MessageParcel &data, MessageParcel &reply)
{
    (void)data;

    Format format;
    int32_t ret = GetSourceFormat(format);
    AVCodecParcel::Marshalling(reply, format);
    CHECK_AND_RETURN_RET_LOG(reply.WriteInt32(ret), AVCS_ERR_UNKNOWN, "Reply GetSourceAddr failed");
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::GetSourceAddr(MessageParcel &data, MessageParcel &reply)
{
    (void)data;

    CHECK_AND_RETURN_RET_LOG(reply.WriteUint64(GetSourceAddr()), AVCS_ERR_UNKNOWN, "Reply GetSourceAddr failed!");
    return AVCS_ERR_OK;
}

int32_t SourceServiceStub::GetDumpInfo(std::string& dumpInfo)
{
    (void)dumpInfo;
    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS