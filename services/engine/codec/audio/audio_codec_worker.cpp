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

#include "audio_codec_worker.h"
#include "avcodec_dfx.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "utils.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioCodecWorker"};
constexpr uint8_t LOGD_FREQUENCY = 5;
} // namespace

namespace OHOS {
namespace Media {
constexpr short DEFAULT_TRY_DECODE_TIME = 10;
constexpr int timeoutMs = 1000;
const std::string_view INPUT_BUFFER = "inputBuffer";
const std::string_view OUTPUT_BUFFER = "outputBuffer";
const std::string_view ASYNC_HANDLE_INPUT = "AsyncHandleInput";
const std::string_view ASYNC_DECODE_FRAME = "AsyncDecodeFrame";

AudioCodecWorker::AudioCodecWorker(const std::shared_ptr<AudioFFMpegBaseCodec> &codec,
                                   const std::shared_ptr<AVCodecCallback> &callback)
    : isFirFrame_(true),
      isRunning(true),
      isProduceInput(true),
      codec_(codec),
      inputBufferSize(codec_->getInputBufferSize()),
      outputBufferSize(codec_->getOutputBufferSize()),
      inputTask_(std::make_unique<TaskThread>(ASYNC_HANDLE_INPUT)),
      outputTask_(std::make_unique<TaskThread>(ASYNC_DECODE_FRAME)),
      callback_(callback),
      inputBuffer_(std::make_shared<AudioBuffersManager>(inputBufferSize, INPUT_BUFFER, 0, 0)),
      outputBuffer_(std::make_shared<AudioBuffersManager>(outputBufferSize, OUTPUT_BUFFER, 0, 0))
{
    inputTask_->RegisterHandler([this] { produceInputBuffer(); });
    outputTask_->RegisterHandler([this] { consumerOutputBuffer(); });
}

AudioCodecWorker::~AudioCodecWorker()
{
    AVCODEC_LOGD("release all data of codec worker in destructor.");
    dispose();

    if (inputTask_) {
        inputTask_->Stop();
        inputTask_.reset();
        inputTask_ = nullptr;
    }
    if (outputTask_) {
        outputTask_->Stop();
        outputTask_.reset();
        outputTask_ = nullptr;
    }

    if (codec_) {
        codec_ = nullptr;
    }

    if (callback_) {
        callback_.reset();
        callback_ = nullptr;
    }
}

bool AudioCodecWorker::PushInputData(const uint32_t &index)
{
    AVCODEC_LOGD("Worker PushInputData enter");

    if (!isRunning) {
        return true;
    }

    if (!callback_) {
        AVCODEC_LOGE("push input buffer failed in worker, callback is nullptr, please check the callback.");
        dispose();
        return false;
    }
    if (!codec_) {
        AVCODEC_LOGE("push input buffer failed in worker, codec is nullptr, please check the codec.");
        dispose();
        return false;
    }

    {
        std::unique_lock lock(stateMutex_);
        inBufIndexQue_.push(index);
    }

    isProduceInput = true;
    inputCondition_.notify_all();
    outputCondition_.notify_all();
    return true;
}

bool AudioCodecWorker::Configure()
{
    AVCODEC_LOGD("Worker Configure enter");
    if (!codec_) {
        AVCODEC_LOGE("Configure failed in worker, codec is nullptr, please check the codec.");
        return false;
    }
    if (inputTask_ != nullptr) {
        inputTask_->RegisterHandler([this] { produceInputBuffer(); });
    } else {
        AVCODEC_LOGE("Configure failed in worker, inputTask_ is nullptr, please check the inputTask_.");
        return false;
    }
    if (outputTask_ != nullptr) {
        outputTask_->RegisterHandler([this] { consumerOutputBuffer(); });
    } else {
        AVCODEC_LOGE("Configure failed in worker, outputTask_ is nullptr, please check the outputTask_.");
        return false;
    }
    return true;
}

bool AudioCodecWorker::Start()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("Worker Start enter");
    if (!callback_) {
        AVCODEC_LOGE("Start failed in worker, callback is nullptr, please check the callback.");
        return false;
    }
    if (!codec_) {
        AVCODEC_LOGE("Start failed in worker, codec_ is nullptr, please check the codec_.");
        return false;
    }
    bool result = begin();
    return result;
}

bool AudioCodecWorker::Stop()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("Worker Stop enter");
    dispose();

    if (inputTask_) {
        inputTask_->Stop();
    } else {
        AVCODEC_LOGE("Stop failed in worker, inputTask_ is nullptr, please check the inputTask_.");
        return false;
    }
    if (outputTask_) {
        outputTask_->Stop();
    } else {
        AVCODEC_LOGE("Stop failed in worker, outputTask_ is nullptr, please check the outputTask_.");
        return false;
    }
    return true;
}

bool AudioCodecWorker::Pause()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("Worker Pause enter");
    dispose();

    if (inputTask_) {
        inputTask_->Pause();
    } else {
        AVCODEC_LOGE("Pause failed in worker, inputTask_ is nullptr, please check the inputTask_.");
        return false;
    }
    if (outputTask_) {
        outputTask_->Pause();
    } else {
        AVCODEC_LOGE("Pause failed in worker, outputTask_ is nullptr, please check the outputTask_.");
        return false;
    }
    return true;
}

bool AudioCodecWorker::Resume()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("Worker Resume enter");
    if (!callback_) {
        AVCODEC_LOGE("Resume failed in worker, callback_ is nullptr, please check the callback_.");
        return false;
    }
    if (!codec_) {
        AVCODEC_LOGE("Resume failed in worker, codec_ is nullptr, please check the codec_.");
        return false;
    }
    bool result = begin();
    return result;
}

bool AudioCodecWorker::Release()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD("Worker Release enter");
    dispose();

    if (inputTask_) {
        inputTask_->Stop();
        inputTask_.reset();
        inputTask_ = nullptr;
    }
    if (outputTask_) {
        outputTask_->Stop();
        outputTask_.reset();
        outputTask_ = nullptr;
    }
    if (codec_) {
        codec_ = nullptr;
    }
    if (callback_) {
        callback_.reset();
        callback_ = nullptr;
    }
    AVCODEC_LOGD("Worker Release end");
    return true;
}

std::shared_ptr<AudioBuffersManager> AudioCodecWorker::GetInputBuffer() const noexcept
{
    AVCODEC_LOGD("Worker GetInputBuffer enter");
    return inputBuffer_;
}

std::shared_ptr<AudioBuffersManager> AudioCodecWorker::GetOutputBuffer() const noexcept
{
    AVCODEC_LOGD("Worker GetOutputBuffer enter");
    return outputBuffer_;
}

std::shared_ptr<AudioBufferInfo> AudioCodecWorker::GetOutputBufferInfo(const uint32_t &index) const noexcept
{
    return outputBuffer_->getMemory(index);
}

std::shared_ptr<AudioBufferInfo> AudioCodecWorker::GetInputBufferInfo(const uint32_t &index) const noexcept
{
    return inputBuffer_->getMemory(index);
}

void AudioCodecWorker::produceInputBuffer()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "Worker produceInputBuffer enter");
    if (!isRunning) {
        SleepFor(DEFAULT_TRY_DECODE_TIME);
        return;
    }
    if (isProduceInput) {
        isProduceInput = false;
        uint32_t index;
        if (inputBuffer_->RequestAvialbaleIndex(index)) {
            AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "produceInputBuffer request success.");
            auto inputBuffer = GetInputBufferInfo(index);
            callback_->OnInputBufferAvailable(index);
        } else {
            AVCODEC_LOGD("produceInputBuffer request failed.");
            SleepFor(DEFAULT_TRY_DECODE_TIME);
            isProduceInput = true;
        }
    }

    std::unique_lock lock(inputMuxt_);
    inputCondition_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                             [this] { return (isProduceInput.load() || !isRunning); });
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "Worker produceInputBuffer exit");
}

bool AudioCodecWorker::handInputBuffer(int32_t &ret)
{
    uint32_t inputIndex = inBufIndexQue_.front();
    inBufIndexQue_.pop();
    auto inputBuffer = GetInputBufferInfo(inputIndex);
    bool isEos = inputBuffer->CheckIsEos();
    ret = codec_->processSendData(inputBuffer);
    inputBuffer_->RelaseBuffer(inputIndex);
    return isEos;
}

void AudioCodecWorker::consumerOutputBuffer()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "Worker consumerOutputBuffer enter");
    if (!isRunning) {
        SleepFor(DEFAULT_TRY_DECODE_TIME);
        return;
    }
    while (!inBufIndexQue_.empty() && isRunning) {
        uint32_t index;
        if (outputBuffer_->RequestAvialbaleIndex(index)) {
            int32_t ret;
            bool isEos = handInputBuffer(ret);
            if (ret == AVCodecServiceErrCode::AVCS_ERR_NOT_ENOUGH_DATA) {
                AVCODEC_LOGW("current input buffer is not enough,skip this frame.");
                outputBuffer_->RelaseBuffer(index);
                continue;
            }
            if (ret != AVCodecServiceErrCode::AVCS_ERR_OK && ret != AVCodecServiceErrCode::AVCS_ERR_END_OF_STREAM) {
                AVCODEC_LOGE("process input buffer error!");
                outputBuffer_->RelaseBuffer(index);
                callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, ret);
                return;
            }
            auto outBuffer = GetOutputBufferInfo(index);
            if (isEos) {
                outBuffer->SetEos(isEos);
            }
            if (isFirFrame_) {
                outBuffer->SetFirstFrame();
                isFirFrame_ = false;
            }
            ret = codec_->processRecieveData(outBuffer);
            if (ret == AVCodecServiceErrCode::AVCS_ERR_NOT_ENOUGH_DATA) {
                AVCODEC_LOGW("current ouput buffer is not enough,skip this frame.");
                outputBuffer_->RelaseBuffer(index);
                continue;
            }
            if (ret != AVCodecServiceErrCode::AVCS_ERR_OK && ret != AVCodecServiceErrCode::AVCS_ERR_END_OF_STREAM) {
                AVCODEC_LOGE("process output buffer error!");
                outputBuffer_->RelaseBuffer(index);
                callback_->OnError(AVCodecErrorType::AVCODEC_ERROR_INTERNAL, ret);
                return;
            }
            AVCODEC_LOGD("Work consumerOutputBuffer callback_");
            callback_->OnOutputBufferAvailable(index, outBuffer->GetBufferAttr(), outBuffer->GetFlag());
        }
    }
    std::unique_lock lock(outputMuxt_);
    outputCondition_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                              [this] { return (inBufIndexQue_.size() > 0 || !isRunning); });
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "Work consumerOutputBuffer exit");
}

void AudioCodecWorker::dispose()
{
    AVCODEC_LOGD("Worker dispose enter");
    isRunning = false;
    isProduceInput = false;

    while (!inBufIndexQue_.empty()) {
        inBufIndexQue_.pop();
    }

    inputCondition_.notify_all();
    outputCondition_.notify_all();

    inputBuffer_->ReleaseAll();
    outputBuffer_->ReleaseAll();
}

bool AudioCodecWorker::begin()
{
    AVCODEC_LOGD("Worker begin enter");
    isRunning = true;
    isProduceInput = true;

    inputBuffer_->SetRunning();
    outputBuffer_->SetRunning();

    if (inputTask_) {
        inputTask_->Start();
    } else {
        return false;
    }
    if (outputTask_) {
        outputTask_->Start();
    } else {
        return false;
    }
    inputCondition_.notify_all();
    outputCondition_.notify_all();
    return true;
}
} // namespace Media
} // namespace OHOS