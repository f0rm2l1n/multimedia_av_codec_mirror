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

#ifndef AVDEMUXER_IMPL_H
#define AVDEMUXER_IMPL_H

#include <memory>
#include "avdemuxer.h"
#include "i_demuxer_service.h"
#include "nocopyable.h"
#include "avsource_impl.h"

namespace OHOS {
namespace Media {
class AVDemuxerImpl : public AVDemuxer, public NoCopyable {
public:
    AVDemuxerImpl();
    ~AVDemuxerImpl();

    int32_t SelectTrackByID(uint32_t trackIndex) override;
    int32_t UnselectTrackByID(uint32_t trackIndex) override;
    int32_t ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
        AVCodecBufferInfo &info, AVCodecBufferFlag &flag) override;
    int32_t SeekToTime(int64_t mSeconds, const AVSeekMode mode) override;
    int32_t Init(AVSource &source);

private:
    std::shared_ptr<IDemuxerService> demuxerClient_ = nullptr;
    std::string sourceUri_;
};
} // namespace Media
} // namespace OHOS
#endif // AVDEMUXER_IMPL_H