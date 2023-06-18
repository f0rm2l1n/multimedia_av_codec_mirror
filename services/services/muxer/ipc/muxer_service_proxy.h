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

#ifndef MUXER_SERVICE_PROXY_H
#define MUXER_SERVICE_PROXY_H

#include "i_standard_muxer_service.h"

namespace OHOS {
namespace MediaAVCodec {
class MuxerServiceProxy : public IRemoteProxy<IStandardMuxerService>, public NoCopyable {
public:
    explicit MuxerServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~MuxerServiceProxy();

    int32_t InitParameter(int32_t fd, OutputFormat format) override;
    int32_t SetRotation(int32_t rotation) override;
    int32_t AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc) override;
    int32_t Start() override;
    int32_t WriteSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
        AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
    int32_t Stop() override;
    void Release() override;
    int32_t DestroyStub() override;
private:
    static inline BrokerDelegator<MuxerServiceProxy> delegator_;
};
}  // namespace MediaAVCodec
}  // namespace OHOS
#endif  // MUXER_SERVICE_PROXY_H