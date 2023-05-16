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

#ifndef MUXER_SERVER_H
#define MUXER_SERVER_H

#include <mutex>
#include "i_muxer_service.h"
#include "i_muxer_engine.h"
#include "nocopyable.h"

namespace OHOS {
namespace Media {
class MuxerServer : public IMuxerService, public NoCopyable {
public:
    static std::shared_ptr<IMuxerService> Create();
    MuxerServer();
    ~MuxerServer();

    int32_t InitParameter(int32_t fd, OutputFormat format) override;
    int32_t SetLocation(float latitude, float longitude) override;
    int32_t SetRotation(int32_t rotation) override;
    int32_t AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc) override;
    int32_t Start() override;
    int32_t WriteSample(std::shared_ptr<AVSharedMemory> sample, const TrackSampleInfo &info) override;
    int32_t Stop() override;
    void Release() override;
    int32_t DumpInfo(int32_t fd);

private:
    std::shared_ptr<IMuxerEngine> muxerEngine_ = nullptr;
    int32_t appUid_ = 0;
    int32_t appPid_ = 0;
};
}  // namespace Media
}  // namespace OHOS
#endif