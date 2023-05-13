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

#ifndef BASE_CODER_PLUGIN
#define BASE_CODER_PLUGIN

#include "audio_buffer_info.h"
#include "av_codec_base_factory.h"
#include "format.h"
#include "nocopyable.h"
#include <string>

namespace OHOS {
namespace Media {

class IAudioFFMpegBaseCodec : public AVCodecBaseFactory<IAudioFFMpegBaseCodec, std::string>, public NoCopyable {
private:
public:
    IAudioFFMpegBaseCodec() = default;

    virtual ~IAudioFFMpegBaseCodec() = default;

    virtual int32_t init(const Format &format) = 0;

    virtual int32_t processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer) = 0;

    virtual int32_t processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer) = 0;

    virtual int32_t reset() = 0;

    virtual int32_t release() = 0;

    virtual int32_t flush() = 0;

    virtual uint32_t getInputBufferSize() const = 0;

    virtual uint32_t getOutputBufferSize() const = 0;

    virtual Format GetFormat() const noexcept = 0;
};

} // namespace Media
} // namespace OHOS

#endif