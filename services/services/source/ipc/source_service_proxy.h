/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef SOURCE_SERVICE_PROXY_H
#define SOURCE_SERVICE_PROXY_H

#include "i_standard_source_service.h"

namespace OHOS {
namespace Media {
class SourceServiceProxy : public IRemoteProxy<IStandardSourceService>, public NoCopyable {
public:
    explicit SourceServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~SourceServiceProxy();

    int32_t Init(const std::string &uri) override;
    int32_t GetTrackCount(uint32_t &trackCount) override;
    int32_t SetTrackFormat(const Format &format, uint32_t trackIndex) override;
    int32_t GetTrackFormat(Format &format, uint32_t trackIndex) override;
    int32_t GetSourceFormat(Format &format) override;
    uint64_t GetSourceAddr() override;

    int32_t DestroyStub() override;
private:
    static inline BrokerDelegator<SourceServiceProxy> delegator_;
};
}  // namespace Media
}  // namespace OHOS
#endif  // SOURCE_SERVICE_PROXY_H