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
#ifndef DEMUXER_SERVER_H
#define DEMUXER_SERVER_H

#include "i_demuxer_service.h"
#include "nocopyable.h"
#include "i_demuxer_engine.h"

namespace OHOS {
namespace Media {
class DemuxerServer : public std::enable_shared_from_this<DemuxerServer>, public IDemuxerService, public NoCopyable {
public:
    static std::shared_ptr<IDemuxerService> Create();
    DemuxerServer();
    ~DemuxerServer();

    // 业务
    int32_t Init(uintptr_t sourceAddr) override;
    int32_t SelectTrackByID(uint32_t trackIndex) override;
    int32_t UnselectTrackByID(uint32_t trackIndex) override;
    int32_t ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
        AVCodecBufferInfo &info, AVCodecBufferFlag &flag) override;
    int32_t SeekToTime(int64_t millisecond, const AVSeekMode mode) override;

private:
    std::shared_ptr<IDemuxerEngine> demuxerEngine_ = nullptr;
    int32_t appUid_ = 0;
    int32_t appPid_ = 0;
};
}  // namespace Media
}  // namespace OHOS
#endif  // DEMUXER_SERVER_H