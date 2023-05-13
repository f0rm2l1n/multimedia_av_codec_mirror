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

#ifndef I_STANDARD_DEMUXER_SERVICE_H
#define I_STANDARD_DEMUXER_SERVICE_H

#include "avcodec_common.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Media {
class IStandardDemuxerService : public IRemoteBroker {
public:
    virtual ~IStandardDemuxerService() = default;

    // 业务
    virtual int32_t Init(uint64_t sourceAddr) = 0;
    virtual int32_t SelectSourceTrackByID(uint32_t trackIndex) = 0;
    virtual int32_t UnselectSourceTrackByID(uint32_t trackIndex) = 0;
    virtual int32_t CopyNextSample(uint32_t &trackIndex, uint8_t *buffer,
                                    AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag) = 0;
    virtual int32_t SeekToTime(int64_t mSeconds, const AVSeekMode mode) = 0;

    virtual int32_t DestroyStub() = 0;

    enum DemuxerServiceMsg {
        INIT,
        SELECT_SOURCE_TRACK_BY_ID,
        UNSELECT_SOURCE_TRACK_BY_ID,
        COPY_NEXT_SAMPLE,
        SEEK_TO_TIME,

        DESTROY_STUB,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardDemuxerService");
};
}  // namespace Media
}  // namespace OHOS
#endif  // I_STANDARD_DEMUXER_SERVICE_H