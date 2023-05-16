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

#ifndef I_MUXER_SERVICE_H
#define I_MUXER_SERVICE_H

#include <string>
#include <memory>
#include "avsharedmemory.h"
#include "media_description.h"
#include "av_common.h"

namespace OHOS {
namespace Media {
class IMuxerService {
public:
    virtual ~IMuxerService() = default;

    virtual int32_t InitParameter(int32_t fd, OutputFormat format) = 0;
    virtual int32_t SetLocation(float latitude, float longitude) = 0;
    virtual int32_t SetRotation(int32_t rotation) = 0;
    virtual int32_t AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc) = 0;
    virtual int32_t Start() = 0;
    virtual int32_t WriteSample(std::shared_ptr<AVSharedMemory> sample, const TrackSampleInfo &info) = 0;
    virtual int32_t Stop() = 0;
    virtual void Release() = 0;
};
} // namespace Media
} // namespace OHOS
#endif // I_MUXER_SERVICE_H