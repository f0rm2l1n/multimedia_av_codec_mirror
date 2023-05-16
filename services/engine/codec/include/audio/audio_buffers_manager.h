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

#ifndef AV_CODEC_ENGIN_BFFERS_H
#define AV_CODEC_ENGIN_BFFERS_H

#include "audio_buffer_info.h"
#include "nocopyable.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string_view>

namespace OHOS {
namespace Media {
class AudioBuffersManager : public NoCopyable {
public:
    AudioBuffersManager(const uint32_t &bufferSize, const std::string_view &name, const uint32_t &metaSize = 0,
                        size_t align = 1);

    ~AudioBuffersManager();

    std::shared_ptr<AudioBufferInfo> getMemory(const uint32_t &index) const noexcept;

    bool RelaseBuffer(const uint32_t &index);

    bool SetBufferBusy(const uint32_t &index);

    bool RequestNewBuffer(uint32_t &index, std::shared_ptr<AudioBufferInfo> &buffer);

    bool RequestAvialbaleIndex(uint32_t &index);

    void ReleaseAll();

    void SetRunning();

private:
    void initBuffers();
    std::shared_ptr<AudioBufferInfo> createNewBuffer();

private:
    std::atomic<bool> isRunning_;
    std::mutex avilableMuxt_;
    std::condition_variable avilableCondition_;
    std::queue<uint32_t> inBufIndexQue_;
    std::mutex stateMutex_;
    uint32_t bufferSize_;
    uint32_t metaSize_;
    size_t align_;
    std::string_view name_;
    std::vector<std::shared_ptr<AudioBufferInfo>> bufferInfo_;
};
} // namespace Media
} // namespace OHOS

#endif