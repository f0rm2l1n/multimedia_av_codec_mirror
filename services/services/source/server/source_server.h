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

#ifndef SOURCE_SERVER_H
#define SOURCE_SERVER_H

#include "i_source_service.h"
#include "nocopyable.h"
#include "i_source_engine.h"

namespace OHOS {
namespace Media {
class SourceServer : public std::enable_shared_from_this<SourceServer>, public ISourceService, public NoCopyable {
public:
    static std::shared_ptr<ISourceService> Create();
    SourceServer();
    ~SourceServer();

    int32_t Init(const std::string &uri) override;
    int32_t GetTrackCount(uint32_t &trackCount) override;
    int32_t SetTrackFormat(const Format &format, uint32_t trackIndex) override;
    int32_t GetTrackFormat(Format &format, uint32_t trackIndex) override;
    int32_t GetSourceFormat(Format &format) override;
    int32_t GetSourceAddr(uintptr_t &addr) override;
    int32_t DumpInfo(int32_t fd);

private:
    int32_t GetDumpInfo(std::string &dumpInfo);
    std::shared_ptr<ISourceEngine> sourceEngine_ = nullptr;
    std::string uri_;
    int32_t appUid_ = 0;
    int32_t appPid_ = 0;
};
}  // namespace Media
}  // namespace OHOS
#endif // SOURCE_SERVER_H