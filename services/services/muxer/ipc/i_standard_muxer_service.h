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

#ifndef I_STANDARD_MUXER_SERVICE_H
#define I_STANDARD_MUXER_SERVICE_H

#include "i_muxer_service.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Media {
class IStandardMuxerService : public IRemoteBroker {
public:
    virtual ~IStandardMuxerService() = default;
    virtual int32_t InitParameter(int32_t fd, OutputFormat format) = 0;
    virtual int32_t SetRotation(int32_t rotation) = 0;
    virtual int32_t AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc) = 0;
    virtual int32_t Start() = 0;
    virtual int32_t WriteSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
        AVCodecBufferInfo info, AVCodecBufferFlag flag) = 0;
    virtual int32_t Stop() = 0;
    virtual void Release() = 0;
    virtual int32_t DestroyStub() = 0;

    enum MuxerServiceMsg {
        INIT_PARAMETER = 0,
        SET_ROTATION,
        ADD_TRACK,
        START,
        WRITE_SAMPLE,
        STOP,
        RELEASE,
		DESTROY,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardMuxerServiceq1a");
};
}  // namespace Media
}  // namespace OHOS
#endif  // I_STANDARD_MUXER_SERVICE_H