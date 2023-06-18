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

#include "codec_listener_proxy.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "avcodec_parcel.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecListenerProxy"};
}

namespace OHOS {
namespace MediaAVCodec {
CodecListenerProxy::CodecListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardCodecListener>(impl)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecListenerProxy::~CodecListenerProxy()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void CodecListenerProxy::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool token = data.WriteInterfaceToken(CodecListenerProxy::GetDescriptor());
    CHECK_AND_RETURN_LOG(token, "Write descriptor failed!");

    data.WriteInt32(static_cast<int32_t>(errorType));
    data.WriteInt32(errorCode);
    int error = Remote()->SendRequest(CodecListenerMsg::ON_ERROR, data, reply, option);
    CHECK_AND_RETURN_LOG(error == AVCS_ERR_OK, "Send request failed");
}

void CodecListenerProxy::OnOutputFormatChanged(const Format &format)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool token = data.WriteInterfaceToken(CodecListenerProxy::GetDescriptor());
    CHECK_AND_RETURN_LOG(token, "Write descriptor failed!");

    (void)AVCodecParcel::Marshalling(data, format);
    int error = Remote()->SendRequest(CodecListenerMsg::ON_OUTPUT_FORMAT_CHANGED, data, reply, option);
    CHECK_AND_RETURN_LOG(error == AVCS_ERR_OK, "Send request failed");
}

void CodecListenerProxy::OnInputBufferAvailable(uint32_t index)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool token = data.WriteInterfaceToken(CodecListenerProxy::GetDescriptor());
    CHECK_AND_RETURN_LOG(token, "Write descriptor failed!");

    data.WriteUint32(index);
    int error = Remote()->SendRequest(CodecListenerMsg::ON_INPUT_BUFFER_AVAILABLE, data, reply, option);
    CHECK_AND_RETURN_LOG(error == AVCS_ERR_OK, "Send request failed");
}

void CodecListenerProxy::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool token = data.WriteInterfaceToken(CodecListenerProxy::GetDescriptor());
    CHECK_AND_RETURN_LOG(token, "Write descriptor failed!");

    data.WriteUint32(index);
    data.WriteInt64(info.presentationTimeUs);
    data.WriteInt32(info.size);
    data.WriteInt32(info.offset);
    data.WriteInt32(static_cast<int32_t>(flag));
    int error = Remote()->SendRequest(CodecListenerMsg::ON_OUTPUT_BUFFER_AVAILABLE, data, reply, option);
    CHECK_AND_RETURN_LOG(error == AVCS_ERR_OK, "Send request failed");
}

CodecListenerCallback::CodecListenerCallback(const sptr<IStandardCodecListener> &listener)
    : listener_(listener)
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

CodecListenerCallback::~CodecListenerCallback()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void CodecListenerCallback::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
    if (listener_ != nullptr) {
        listener_->OnError(errorType, errorCode);
    }
}

void CodecListenerCallback::OnOutputFormatChanged(const Format &format)
{
    if (listener_ != nullptr) {
        listener_->OnOutputFormatChanged(format);
    }
}

void CodecListenerCallback::OnInputBufferAvailable(uint32_t index)
{
    if (listener_ != nullptr) {
        listener_->OnInputBufferAvailable(index);
    }
}

void CodecListenerCallback::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    if (listener_ != nullptr) {
        listener_->OnOutputBufferAvailable(index, info, flag);
    }
}
} // namespace MediaAVCodec
} // namespace OHOS
