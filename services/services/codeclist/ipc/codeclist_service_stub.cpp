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

#include <unistd.h>
#include <string>
#include <map>
#include "avsharedmemory_ipc.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_server_manager.h"
#include "avcodec_xcollie.h"
#include "codeclist_service_stub.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecListServiceStub"};

    const std::map<uint32_t, std::string> CODECLIST_FUNC_NAME = {
        { static_cast<uint32_t>(OHOS::Media::CodecListServiceStub::AVCodecListServiceMsg::FIND_DECODER),
            "CodecListServiceStub DoFindDecoder" },
        { static_cast<uint32_t>(OHOS::Media::CodecListServiceStub::AVCodecListServiceMsg::FIND_ENCODER),
            "CodecListServiceStub DoFindEncoder" },
        { static_cast<uint32_t>(OHOS::Media::CodecListServiceStub::AVCodecListServiceMsg::GET_CAPABILITY),
            "CodecListServiceStub DoGetCapability" },
        { static_cast<uint32_t>(OHOS::Media::CodecListServiceStub::AVCodecListServiceMsg::DESTROY),
            "CodecListServiceStub DoDestroyStub" },
    };
}

namespace OHOS {
namespace Media {
sptr<CodecListServiceStub> CodecListServiceStub::Create()
{
    sptr<CodecListServiceStub> codecListStub = new (std::nothrow) CodecListServiceStub();
    CHECK_AND_RETURN_RET_LOG(codecListStub != nullptr, nullptr, "Create codeclist service stub failed");

    int32_t ret = codecListStub->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Init codeclist stub failed");
    return codecListStub;
}

CodecListServiceStub::CodecListServiceStub()
{
    AVCODEC_LOGD("Create codeclist service stub instance successful");
}

CodecListServiceStub::~CodecListServiceStub()
{
    AVCODEC_LOGD("Destroy codeclist service stub instance successful");
}

int32_t CodecListServiceStub::Init()
{
    codecListServer_ = CodecListServer::Create();
    CHECK_AND_RETURN_RET_LOG(codecListServer_ != nullptr, AVCS_ERR_NO_MEMORY, "Create codecList server failed");
    codecListFuncs_[static_cast<uint32_t>(AVCodecListServiceMsg::FIND_DECODER)] = &CodecListServiceStub::DoFindDecoder;
    codecListFuncs_[static_cast<uint32_t>(AVCodecListServiceMsg::FIND_ENCODER)] = &CodecListServiceStub::DoFindEncoder;
    codecListFuncs_[static_cast<uint32_t>(AVCodecListServiceMsg::GET_CAPABILITY)] =
        &CodecListServiceStub::DoGetCapability;
    codecListFuncs_[static_cast<uint32_t>(AVCodecListServiceMsg::DESTROY)] = &CodecListServiceStub::DoDestroyStub;
    return AVCS_ERR_OK;
}

int32_t CodecListServiceStub::DestroyStub()
{
    codecListServer_ = nullptr;
    AVCodecServerManager::GetInstance().DestroyStubObject(AVCodecServerManager::CODECLIST, AsObject());
    return AVCS_ERR_OK;
}

int CodecListServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                          MessageOption &option)
{
    AVCODEC_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (CodecListServiceStub::GetDescriptor() != remoteDescriptor) {
        AVCODEC_LOGE("Invalid descriptor");
        return AVCS_ERR_INVALID_OPERATION;
    }

    auto itFunc = codecListFuncs_.find(code);
    if (itFunc != codecListFuncs_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            int32_t ret = -1;
            auto itFuncName = CODECLIST_FUNC_NAME.find(code);
            std::string funcName =
                itFuncName != CODECLIST_FUNC_NAME.end() ? itFuncName->second : "CodecListServiceStub OnRemoteRequest";
            COLLIE_LISTEN(ret = (this->*memberFunc)(data, reply), funcName);
            if (ret != AVCS_ERR_OK) {
                AVCODEC_LOGE("Calling memberFunc is failed.");
            }
            return AVCS_ERR_OK;
        }
    }
    AVCODEC_LOGW("CodecListServiceStub: no member func supporting, applying default process");

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

std::string CodecListServiceStub::FindDecoder(const Format &format)
{
    CHECK_AND_RETURN_RET_LOG(codecListServer_ != nullptr, "", "Find decoder failed: avcodeclist server is nullptr");
    return codecListServer_->FindDecoder(format);
}

std::string CodecListServiceStub::FindEncoder(const Format &format)
{
    CHECK_AND_RETURN_RET_LOG(codecListServer_ != nullptr, "", "Find encoder failed: avcodeclist server is nullptr");
    return codecListServer_->FindEncoder(format);
}

CapabilityData CodecListServiceStub::GetCapability(const std::string &mime, const bool isEncoder,
                                                   const AVCodecCategory &category)
{
    CapabilityData capabilityData;
    CHECK_AND_RETURN_RET_LOG(codecListServer_ != nullptr, capabilityData,
                             "Get capability failed: avcodeclist server is null");
    return codecListServer_->GetCapability(mime, isEncoder, category);
}

int32_t CodecListServiceStub::DoFindDecoder(MessageParcel &data, MessageParcel &reply)
{
    Format format;
    (void)AVCodecParcel::Unmarshalling(data, format);
    reply.WriteString(FindDecoder(format));
    return AVCS_ERR_OK;
}

int32_t CodecListServiceStub::DoFindEncoder(MessageParcel &data, MessageParcel &reply)
{
    Format format;
    (void)AVCodecParcel::Unmarshalling(data, format);
    reply.WriteString(FindEncoder(format));
    return AVCS_ERR_OK;
}

int32_t CodecListServiceStub::DoGetCapability(MessageParcel &data, MessageParcel &reply)
{
    std::string mime = data.ReadString();
    bool isEncoder = data.ReadBool();
    AVCodecCategory category = static_cast<AVCodecCategory>(data.ReadInt32());
    CapabilityData capabilityData = GetCapability(mime, isEncoder, category);
    (void)CodecListParcel::Marshalling(reply, capabilityData);
    return AVCS_ERR_OK;
}

int32_t CodecListServiceStub::DoDestroyStub(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    reply.WriteInt32(DestroyStub());
    return AVCS_ERR_OK;
}
} // namespace Media
} // namespace OHOS