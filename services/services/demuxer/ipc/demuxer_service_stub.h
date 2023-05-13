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
#ifndef DEMUXER_SERVICE_STUB_H
#define DEMUXER_SERVICE_STUB_H

#include <map>
#include <string>
#include "i_standard_demuxer_service.h"
#include "demuxer_server.h"
#include "iremote_stub.h"
#include "avcodec_common.h"

namespace OHOS {
namespace Media {
class DemuxerServiceStub : public IRemoteStub<IStandardDemuxerService>, public NoCopyable {
public:
    static sptr<DemuxerServiceStub> Create();
    virtual ~DemuxerServiceStub();
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    using DemuxerStubFunc = int32_t(DemuxerServiceStub::*)(MessageParcel &data, MessageParcel &reply);

    // 业务
    int32_t Init(uint64_t sourceAddr) override;
    int32_t SelectSourceTrackByID(uint32_t trackIndex) override;
    int32_t UnselectSourceTrackByID(uint32_t trackIndex) override;
    int32_t CopyNextSample(uint32_t &trackIndex, uint8_t *buffer, 
                            AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag) override;
    int32_t SeekToTime(int64_t mSeconds, const AVSeekMode mode) override;

    int32_t DumpInfo(int32_t fd);
    int32_t DestroyStub() override;
private:
    DemuxerServiceStub();
    int32_t InitStub();

    int32_t Init(MessageParcel &data, MessageParcel &reply);
    int32_t SelectSourceTrackByID(MessageParcel &data, MessageParcel &reply);
    int32_t UnselectSourceTrackByID(MessageParcel &data, MessageParcel &reply);
    int32_t CopyNextSample(MessageParcel &data, MessageParcel &reply);
    int32_t SeekToTime(MessageParcel &data, MessageParcel &reply);
    int32_t GetDumpInfo(std::string& dumpInfo);
    int32_t DestroyStub(MessageParcel &data, MessageParcel &reply);

    std::mutex mutex_;
    std::shared_ptr<IDemuxerService> demuxerServer_ = nullptr;
    std::map<uint32_t, DemuxerStubFunc> demuxerFuncs_;
};
}  // namespace Media
}  // namespace OHOS
#endif