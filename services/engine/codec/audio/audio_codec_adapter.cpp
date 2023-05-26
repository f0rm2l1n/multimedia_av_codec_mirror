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

#include "audio_codec_adapter.h"
#include "avcodec_dfx.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "media_description.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioCodecAdapter"};
}

namespace OHOS {
namespace Media {
AudioCodecAdapter::AudioCodecAdapter(const std::string &name) : state_(CodecState::RELEASED), name_(name)
{
    AVCODEC_LOGD("enter constructor of adapter,name:%{public}s,name after:%{public}s", name.data(), name_.data());
}

AudioCodecAdapter::~AudioCodecAdapter()
{
    callback_ = nullptr;
    if (audioCodec) {
        audioCodec->Release();
        audioCodec.reset();
        audioCodec = nullptr;
    }
    if (worker_) {
        worker_->Release();
        worker_.reset();
        worker_ = nullptr;
    }
    state_ = CodecState::RELEASED;
}

int32_t AudioCodecAdapter::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    AVCODEC_SYNC_TRACE;
    if (state_ != CodecState::RELEASED && state_ != CodecState::INITIALIZED && state_ != CodecState::INITIALIZING) {
        AVCODEC_LOGE("SetCallback failed, state = %{public}s .", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }
    if (!callback) {
        AVCODEC_LOGE("SetCallback failed, callback is nullptr.");
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    callback_ = callback;
    AVCODEC_LOGD("SetCallback success");
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodecAdapter::Configure(const Format &format)
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("Configure enter");
    if (!format.ContainKey(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT)) {
        AVCODEC_LOGE("Configure failed, missing channel count key in format.");
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT;
    }

    if (!format.ContainKey(MediaDescriptionKey::MD_KEY_SAMPLE_RATE)) {
        AVCODEC_LOGE("Configure failed,missing sample rate key in format.");
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_SAMPLE_RATE;
    }

    if (!format.ContainKey(MediaDescriptionKey::MD_KEY_BITRATE)) {
        AVCODEC_LOGE("adapter configure error,missing bits rate key in format.");
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_BIT_RATE;
    }

    if (state_ != CodecState::RELEASED) {
        AVCODEC_LOGE("Configure failed, state = %{public}s .", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }

    AVCODEC_LOGI("state from %{public}s to INITIALIZING, name:%{public}s, name size:%{public}zu",
                 stateToString(state_).data(), name_.data(), name_.size());
    state_ = CodecState::INITIALIZING;
    auto ret = doInit();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return ret;
    }

    if (state_ != CodecState::INITIALIZED) {
        AVCODEC_LOGE("Configure failed, state =%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_ERROR;
    }

    ret = doConfigure(format);
    AVCODEC_LOGD("Configure exit");
    return ret;
}

int32_t AudioCodecAdapter::Start()
{
    AVCODEC_LOGD("Start enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter start error, callback not initialized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    if (!audioCodec) {
        AVCODEC_LOGE("adapter start error, audio codec not initialized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    if (state_ == CodecState::FLUSHED) {
        AVCODEC_LOGI("Start, doResume");
        return doResume();
    }

    if (state_ != CodecState::INITIALIZED) {
        AVCODEC_LOGE("Start is incorrect, state = %{public}s .", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }
    AVCODEC_LOGI("Start, state from %{public}s to STARTING", stateToString(state_).data());
    state_ = CodecState::STARTING;
    auto ret = doStart();
    return ret;
}

int32_t AudioCodecAdapter::Stop()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("Stop enter");
    if (!callback_) {
        AVCODEC_LOGE("Stop failed, call back not initialized.");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    if (state_ == CodecState::INITIALIZED || state_ == CodecState::RELEASED || state_ == CodecState::STOPPING ||
        state_ == CodecState::RELEASING) {
        AVCODEC_LOGD("Stop, state_=%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }
    state_ = CodecState::STOPPING;
    auto ret = doStop();
    AVCODEC_LOGI("adapter Stop, state from %{public}s to INITIALIZED", stateToString(state_).data());
    state_ = CodecState::INITIALIZED;
    return ret;
}

int32_t AudioCodecAdapter::Flush()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("adapter Flush enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter flush error, call back not initialized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    if (state_ == CodecState::FLUSHED) {
        AVCODEC_LOGW("Flush, state is already flushed, state_=%{public}s .", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }
    if (state_ != CodecState::RUNNING) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("Flush failed, state =%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }
    AVCODEC_LOGI("Flush, state from %{public}s to FLUSHING", stateToString(state_).data());
    state_ = CodecState::FLUSHING;
    auto ret = doFlush();
    return ret;
}

int32_t AudioCodecAdapter::Reset()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("adapter Reset enter");
    if (state_ == CodecState::RELEASED || state_ == CodecState::RELEASING) {
        AVCODEC_LOGW("adapter reset, state is already released, state =%{public}s .", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }
    if (state_ == CodecState::INITIALIZING) {
        AVCODEC_LOGW("adapter reset, state is initialized, state =%{public}s .", stateToString(state_).data());
        state_ = CodecState::RELEASED;
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }
    if (worker_) {
        worker_->Release();
        worker_.reset();
        worker_ = nullptr;
    }
    int32_t status = AVCodecServiceErrCode::AVCS_ERR_OK;
    if (audioCodec) {
        status = audioCodec->Reset();
        audioCodec.reset();
        audioCodec = nullptr;
    }
    state_ = CodecState::RELEASED;
    AVCODEC_LOGI("adapter Reset, state from %{public}s to INITIALIZED", stateToString(state_).data());
    return status;
}

int32_t AudioCodecAdapter::Release()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("adapter Release enter");
    if (state_ == CodecState::RELEASED || state_ == CodecState::RELEASING) {
        AVCODEC_LOGW("adapter Release, state isnot completely correct, state =%{public}s .",
                     stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }

    if (state_ == CodecState::INITIALIZING) {
        AVCODEC_LOGW("adapter Release, state isnot completely correct, state =%{public}s .",
                     stateToString(state_).data());
        state_ = CodecState::RELEASING;
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }

    if (state_ == CodecState::STARTING || state_ == CodecState::RUNNING || state_ == CodecState::STOPPING) {
        AVCODEC_LOGE("adapter Release, state is running, state =%{public}s .", stateToString(state_).data());
    }
    AVCODEC_LOGI("adapter Release, state from %{public}s to RELEASING", stateToString(state_).data());
    state_ = CodecState::RELEASING;
    auto ret = doRelease();
    return ret;
}

int32_t AudioCodecAdapter::NotifyEos()
{
    AVCODEC_SYNC_TRACE;
    Flush();
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodecAdapter::SetParameter(const Format &format)
{
    AVCODEC_SYNC_TRACE;
    (void)format;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodecAdapter::GetOutputFormat(Format &format)
{
    AVCODEC_SYNC_TRACE;
    format = audioCodec->GetFormat();
    if (!format.ContainKey(MediaDescriptionKey::MD_KEY_CODEC_NAME)) {
        format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_NAME, name_);
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

std::shared_ptr<AVSharedMemoryBase> AudioCodecAdapter::GetInputBuffer(uint32_t index)
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("adapter GetInputBuffer enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter get input buffer error, call back not initialized .");
        return nullptr;
    }

    std::shared_ptr<AudioBufferInfo> result = worker_->GetInputBufferInfo(index);
    if (result == nullptr) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        AVCODEC_LOGE("getMemory failed");
        return nullptr;
    }

    if (result->GetStatus() == BufferStatus::IDEL) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("GetStatus is IDEL");
        return nullptr;
    }

    return result->GetBuffer();
}

int32_t AudioCodecAdapter::QueueInputBuffer(uint32_t index, const AVCodecBufferInfo &info, AVCodecBufferFlag flag)
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("adapter QueueInputBuffer enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter queue input buffer error, call back not initialized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    auto result = worker_->GetInputBufferInfo(index);
    if (result == nullptr) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        AVCODEC_LOGE("getMemory failed");
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }

    if (result->GetStatus() != BufferStatus::OWNE_BY_CLIENT) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        AVCODEC_LOGE("GetStatus failed");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    result->SetBufferAttr(info);
    if (flag == AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_EOS) {
        result->SetEos(true);
    }
    worker_->PushInputData(index);
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

std::shared_ptr<AVSharedMemoryBase> AudioCodecAdapter::GetOutputBuffer(uint32_t index)
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("adapter GetOutputBuffer enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter get output buffer error, call back not initialized .");
        return nullptr;
    }

    auto result = worker_->GetOutputBufferInfo(index);
    if (result == nullptr) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        AVCODEC_LOGE("getMemory failed");
        return nullptr;
    }

    if (result->GetStatus() == BufferStatus::IDEL) {
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE);
        return nullptr;
    }

    return result->GetBuffer();
}

int32_t AudioCodecAdapter::ReleaseOutputBuffer(uint32_t index)
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("adapter ReleaseOutputBuffer enter");
    if (!callback_) {
        AVCODEC_LOGE("adapter release output buffer error, call back not initialized .");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    auto outBufferInfo = worker_->GetOutputBufferInfo(index);
    if (outBufferInfo == nullptr) {
        AVCODEC_LOGE("index=%{public}u error", index);
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }
    bool isEos = outBufferInfo->CheckIsEos();

    auto outBuffer = worker_->GetOutputBuffer();
    if (outBuffer == nullptr) {
        AVCODEC_LOGE("index=%{public}u error", index);
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }
    bool result = outBuffer->ReleaseBuffer(index);
    if (!result) {
        AVCODEC_LOGE("ReleaseBuffer failed");
        callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY);
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }

    if (isEos) {
        NotifyEos();
    }

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodecAdapter::doInit()
{
    AVCODEC_SYNC_TRACE;
    if (name_.empty()) {
        state_ = CodecState::RELEASED;
        AVCODEC_LOGE("doInit failed, because name is empty");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    AVCODEC_LOGI("adapter doInit, codec name:%{public}s", name_.data());
    audioCodec = AudioBaseCodec::make_sharePtr(name_);
    if (audioCodec == nullptr) {
        state_ = CodecState::RELEASED;
        AVCODEC_LOGE("Initlize failed, because create codec failed. name: %{public}s.", name_.data());
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("adapter doInit, state from %{public}s to INITIALIZED", stateToString(state_).data());
    state_ = CodecState::INITIALIZED;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodecAdapter::doConfigure(const Format &format)
{
    AVCODEC_SYNC_TRACE;
    if (state_ != CodecState::INITIALIZED) {
        AVCODEC_LOGE("adapter configure failed because state is incorrect,state:%{public}d.",
                     static_cast<int>(state_.load()));
        state_ = CodecState::RELEASED;
        AVCODEC_LOGE("state_=%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }
    int32_t ret = audioCodec->Init(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("configure failed, because codec init failed,error:%{public}d.", static_cast<int>(ret));
        state_ = CodecState::RELEASED;
        return ret;
    }
    return ret;
}

int32_t AudioCodecAdapter::doStart()
{
    AVCODEC_SYNC_TRACE;
    if (state_ != CodecState::STARTING) {
        AVCODEC_LOGE("doStart failed, state = %{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }

    AVCODEC_LOGI("adapter doStart, state from %{public}s to RUNNING", stateToString(state_).data());
    state_ = CodecState::RUNNING;
    worker_ = std::make_shared<AudioCodecWorker>(audioCodec, callback_);
    worker_->Start();
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodecAdapter::doResume()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGI("adapter doResume, state from %{public}s to RESUMING", stateToString(state_).data());
    state_ = CodecState::RESUMING;
    worker_->Resume();
    AVCODEC_LOGI("adapter doResume, state from %{public}s to RUNNING", stateToString(state_).data());
    state_ = CodecState::RUNNING;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodecAdapter::doStop()
{
    AVCODEC_SYNC_TRACE;
    if (state_ == CodecState::RELEASING) {
        AVCODEC_LOGW("adapter doStop, state is already release, state_=%{public}s .", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }

    if (state_ != CodecState::STOPPING) {
        AVCODEC_LOGE("doStop failed, state =%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }
    worker_->Stop();
    AVCODEC_LOGI("adapter doStop, state from %{public}s to INITIALIZED", stateToString(state_).data());
    state_ = CodecState::INITIALIZED;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodecAdapter::doFlush()
{
    AVCODEC_SYNC_TRACE;
    if (state_ != CodecState::FLUSHING) {
        AVCODEC_LOGE("doFlush failed, state_=%{public}s", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }

    int32_t status = audioCodec->Flush();

    worker_->Pause();

    state_ = CodecState::FLUSHED;
    if (status != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("status=%{public}d", static_cast<int>(status));
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioCodecAdapter::doRelease()
{
    AVCODEC_SYNC_TRACE;
    if (state_ == CodecState::RELEASED) {
        AVCODEC_LOGW("adapter doRelease, state is already released, state_=%{public}s .", stateToString(state_).data());
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }
    if (audioCodec != nullptr) {
        audioCodec->Release();
        audioCodec.reset();
        audioCodec = nullptr;
    }
    if (worker_ != nullptr) {
        worker_->Release();
        worker_.reset();
        worker_ = nullptr;
    }
    AVCODEC_LOGI("adapter doRelease, state from %{public}s to RELEASED", stateToString(state_).data());
    state_ = CodecState::RELEASED;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

std::string_view AudioCodecAdapter::stateToString(CodecState state)
{
    std::map<CodecState, std::string_view> stateStrMap = {
        {CodecState::RELEASED, " RELEASED"},         {CodecState::INITIALIZED, " INITIALIZED"},
        {CodecState::FLUSHED, " FLUSHED"},           {CodecState::RUNNING, " RUNNING"},
        {CodecState::INITIALIZING, " INITIALIZING"}, {CodecState::STARTING, " STARTING"},
        {CodecState::STOPPING, " STOPPING"},         {CodecState::FLUSHING, " FLUSHING"},
        {CodecState::RESUMING, " RESUMING"},         {CodecState::RELEASING, " RELEASING"},
    };
    return stateStrMap[state];
}
} // namespace Media
} // namespace OHOS