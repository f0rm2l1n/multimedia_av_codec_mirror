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

#include "audio_ffmpeg_aac_decoder_plugin.h"
#include "avcodec_dfx.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "media_description.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegAacDecoderPlugin"};
}

namespace OHOS {
namespace Media {

AudioFFMpegAacDecoderPlugin::AudioFFMpegAacDecoderPlugin() : basePlugin(std::make_unique<AudioFfmpegDecoderPlugin>()) {}

AudioFFMpegAacDecoderPlugin::~AudioFFMpegAacDecoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

int32_t AudioFFMpegAacDecoderPlugin::init(const Format &format)
{
    int type;

    format.GetIntValue("aac-type", type);

    int32_t ret = AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    if (type == 1) {
        ret = basePlugin->AllocateContext("aac");
    } else if (type == 2) {
        ret = basePlugin->AllocateContext("aac_latm");
    }
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("AllocateContext failed, ret=%{public}d", ret);
        return ret;
    }
    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("InitContext failed, ret=%{public}d", ret);
        return ret;
    }
    return basePlugin->OpenContext();
}

int32_t AudioFFMpegAacDecoderPlugin::processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegAacDecoderPlugin::processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegAacDecoderPlugin::reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegAacDecoderPlugin::release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegAacDecoderPlugin::flush()
{
    return basePlugin->Flush();
}

uint32_t AudioFFMpegAacDecoderPlugin::getInputBufferSize() const
{
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > 8192) {
        maxSize = 8192;
    }
    return maxSize;
}

uint32_t AudioFFMpegAacDecoderPlugin::getOutputBufferSize() const
{
    return 4 * 1024 * 8;
}

Format AudioFFMpegAacDecoderPlugin::GetFormat() const noexcept
{
    return basePlugin->GetFormat();
}

} // namespace Media
} // namespace OHOS