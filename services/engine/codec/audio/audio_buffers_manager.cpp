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

#include "audio_buffers_manager.h"
#include "avcodec_log.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioBuffersManager"};
}

namespace OHOS {
namespace Media {
constexpr short DEFALT_BUFFER_LENGTH{8};
constexpr short DEFALT_SLEEP_TIME{500};

AudioBuffersManager::~AudioBuffersManager() {}

AudioBuffersManager::AudioBuffersManager(const uint32_t &bufferSize, const std::string_view &name,
                                         const uint32_t &metaSize, size_t align)
    : isRunning_(true),
      bufferSize_(bufferSize),
      metaSize_(metaSize),
      align_(align),
      name_(name),
      bufferInfo_(DEFALT_BUFFER_LENGTH)
{
    initBuffers();
}

std::shared_ptr<AudioBufferInfo> AudioBuffersManager::getMemory(const uint32_t &index) const noexcept
{
    if (index >= bufferInfo_.size()) {
        return nullptr;
    }
    auto bufferInfo = bufferInfo_[index];
    return bufferInfo;
}

bool AudioBuffersManager::SetBufferBusy(const uint32_t &index)
{
    if (index < bufferInfo_.size()) {
        bufferInfo_[index]->SetBufferOwned();
        return true;
    }
    return false;
}

void AudioBuffersManager::initBuffers()
{
    std::unique_lock lock(stateMutex_);
    AVCODEC_LOGI("start allocate %{public}s buffers,buffer size:%{public}d", name_.data(), bufferSize_);
    for (size_t i = 0; i < DEFALT_BUFFER_LENGTH; i++) {
        bufferInfo_[i] = std::make_shared<AudioBufferInfo>(bufferSize_, name_, metaSize_, align_);
        inBufIndexQue_.push(i);
    }
    AVCODEC_LOGI("end allocate %{public}s buffers", name_.data());
}

bool AudioBuffersManager::RequestNewBuffer(uint32_t *index, std::shared_ptr<AudioBufferInfo> &buffer)
{
    buffer = createNewBuffer();
    if (buffer == nullptr) {
        return false;
    }
    *index = bufferInfo_.size() - 1;
    return true;
}

bool AudioBuffersManager::RequestAvialbaleIndex(uint32_t *index)
{
    while (inBufIndexQue_.empty() && isRunning_) {
        AVCODEC_LOGD("Request empty %{public}s buffer", name_.data());
        std::unique_lock aLock(avilableMuxt_);
        avilableCondition_.wait_for(aLock, std::chrono::milliseconds(DEFALT_SLEEP_TIME),
                                    [this] { return !inBufIndexQue_.empty() || !isRunning_; });
    }

    if (!isRunning_) {
        return false;
    }

    std::unique_lock lock(stateMutex_);
    *index = inBufIndexQue_.front();
    bufferInfo_[*index]->SetBufferOwned();
    inBufIndexQue_.pop();
    return true;
}

void AudioBuffersManager::ReleaseAll()
{
    AVCODEC_LOGI("step in release all %{public}s buffer.", name_.data());
    for (uint32_t i = 0; i < bufferInfo_.size(); ++i) {
        bufferInfo_[i]->ResetBuffer();
        inBufIndexQue_.push(i);
    }
    AVCODEC_LOGI("step out release all %{public}s buffer.", name_.data());
    isRunning_ = false;
    avilableCondition_.notify_all();
}

void AudioBuffersManager::SetRunning()
{
    isRunning_ = true;
}

bool AudioBuffersManager::RelaseBuffer(const uint32_t &index)
{
    if (index < bufferInfo_.size()) {
        std::unique_lock lock(avilableMuxt_);
        bufferInfo_[index]->ResetBuffer();
        inBufIndexQue_.push(index);
        avilableCondition_.notify_all();
        return true;
    }
    return false;
}

std::shared_ptr<AudioBufferInfo> AudioBuffersManager::createNewBuffer()
{
    std::shared_ptr<AudioBufferInfo> buffer = std::make_shared<AudioBufferInfo>(bufferSize_, name_, metaSize_, align_);
    bufferInfo_.emplace_back(buffer);
    return buffer;
}
} // namespace Media
} // namespace OHOS