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

#include "codec_factory.h"
#include <limits>
#include <cinttypes>
#include <dlfcn.h>
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "audio_ffmpeg_adapter.h"
#include "fcodec.h"
#include "codeclist_core.h"
#include "format.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "CodecFactory"};
}

namespace OHOS {
namespace Media {
const std::string VIDEOMIMETYPE = "video/avc";
const std::string VIDEOCODECNAME = "video_decoder.avc";

CodecFactory &CodecFactory::Instance()
{
    static CodecFactory inst;
    return inst;
}

CodecFactory::~CodecFactory()
{
}

std::shared_ptr<CodecBase> CodecFactory::CreateCodecByMime(bool isEncoder, const std::string &mime)
{
    std::shared_ptr<CodecListCore> codecListCore = std::make_shared<CodecListCore>();
    std::string codecname;
    Format format;
    format.PutStringValue("codec_mime", mime);
    if (isEncoder) {
        codecname = codecListCore->FindEncoder(format);
    } else {
        codecname = codecListCore->FindDecoder(format);
    }
    std::shared_ptr<CodecBase> codec = CreateCodecByName(codecname);
    AVCODEC_LOGD("CreateCodecByMime is successful");
    return codec;
}

std::shared_ptr<CodecBase> CodecFactory::CreateCodecByName(const std::string &name)
{
    std::shared_ptr<CodecBase> codec = nullptr;
    if (name == VIDEOCODECNAME) {
        codec = std::make_shared<Codec::FCodec>(name);
    } else {
        codec = std::make_shared<AudioFFMpegAdapter>(name);
    }
    AVCODEC_LOGD("Create %{public}s", name.c_str());
    return codec;
}
} // namespace Media
} // namespace OHOS
