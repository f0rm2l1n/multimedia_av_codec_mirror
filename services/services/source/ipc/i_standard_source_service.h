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

#ifndef I_STANDARD_SOURCE_SERVICE_H
#define I_STANDARD_SOURCE_SERVICE_H

#include "iremote_proxy.h"
#include "format.h"

namespace OHOS {
namespace Media {
class IStandardSourceService : public IRemoteBroker {
public:
    virtual ~IStandardSourceService() = default;

    virtual int32_t InitWithURI(const std::string &uri) = 0;
    virtual int32_t InitWithFD(int32_t fd, int64_t offset, int64_t size) = 0;
    virtual int32_t GetTrackCount(uint32_t &trackCount) = 0;
    virtual int32_t GetTrackFormat(Format &format, uint32_t trackIndex) = 0;
    virtual int32_t GetSourceFormat(Format &format) = 0;
    virtual int32_t GetSourceAddr(uintptr_t &addr) = 0;

    virtual int32_t DestroyStub() = 0;
    enum SourceServiceMsg {
        INIT_WITH_URI,
        INIT_WITH_FD,
        GET_TRACK_COUNT,
        DESTROY,
        GET_TRACK_FORMAT,
        GET_SOURCE_FORMAT,
        GET_SOURCE_ADDR,
        DESTROY_STUB,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardSourceService");
};
}  // namespace Media
}  // namespace OHOS
#endif  // I_AVSOURCE_SERVICE_H