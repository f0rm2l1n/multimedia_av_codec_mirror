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

#ifndef I_SOURCE_SERVICE_H
#define I_SOURCE_SERVICE_H

#include <string>
#include <memory>
#include "format.h"

namespace OHOS {
namespace Media {
class ISourceService {
public:
    virtual ~ISourceService() = default;

    // 业务
    virtual int32_t Init(const std::string &uri) = 0;
    virtual int32_t GetTrackCount(uint32_t &trackCount) = 0;
    virtual int32_t SetTrackFormat(const Format &format, uint32_t trackIndex) = 0;
    virtual int32_t GetTrackFormat(Format &format, uint32_t trackIndex) = 0;
    virtual int32_t GetSourceFormat(Format &format) = 0;
    virtual uint64_t GetSourceAddr() = 0;
};
} // namespace Media
} // namespace OHOS
#endif // I_SOURCE_SERVICE_H
