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

#ifndef MUXER_SERVICE_STUB_H
#define MUXER_SERVICE_STUB_H

#include "i_standard_muxer_service.h"
#include "muxer_server.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Media {
class MuxerServiceStub : public IRemoteStub<IStandardMuxerService>, public NoCopyable {
public:
    static sptr<MuxerServiceStub> Create();
    virtual ~MuxerServiceStub();
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    using MuxerStubFunc = int32_t(MuxerServiceStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t InitParameter(int32_t fd, OutputFormat format) override;
    int32_t SetRotation(int32_t rotation) override;
    int32_t AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc) override;
    int32_t Start() override;
    int32_t WriteSample(std::shared_ptr<AVSharedMemory> sample, const TrackSampleInfo &info) override;
    int32_t Stop() override;
    void Release() override;
    int32_t DestroyStub() override;
    int32_t DumpInfo(int32_t fd);

private:
    MuxerServiceStub();
    int32_t Init();
    int32_t InitParameter(MessageParcel &data, MessageParcel &reply);
    int32_t SetRotation(MessageParcel &data, MessageParcel &reply);
    int32_t AddTrack(MessageParcel &data, MessageParcel &reply);
    int32_t Start(MessageParcel &data, MessageParcel &reply);
    int32_t WriteSample(MessageParcel &data, MessageParcel &reply);
    int32_t Stop(MessageParcel &data, MessageParcel &reply);
    int32_t Release(MessageParcel &data, MessageParcel &reply);
    int32_t DestroyStub(MessageParcel &data, MessageParcel &reply);

    std::shared_ptr<IMuxerService> muxerServer_ {nullptr};
    std::map<uint32_t, MuxerStubFunc> muxerFuncs_;
};
}  // namespace Media
}  // namespace OHOS
#endif // MUXER_SERVICE_STUB_H