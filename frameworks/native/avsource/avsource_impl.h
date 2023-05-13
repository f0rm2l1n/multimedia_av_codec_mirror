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

#ifndef AVSOURCE_IMPL_H
#define AVSOURCE_IMPL_H

#include "avsource.h"
#include "nocopyable.h"
#include "i_source_service.h"

namespace OHOS {
namespace Media {
class AVSourceImpl : public AVSource, public NoCopyable {
public:
    AVSourceImpl();
    ~AVSourceImpl() override;

    int32_t GetTrackCount(uint32_t &trackCount) override;
    std::shared_ptr<AVSourceTrack> GetSourceTrackByID(uint32_t trackIndex) override;
    uintptr_t GetSourceAddr() override;
    int32_t GetSourceFormat(Format &format) override;
    int32_t GetTrackFormat(Format &format, uint32_t trackIndex);
    int32_t SetTrackFormat(const Format &format, uint32_t trackIndex);
    int32_t Init(const std::string &uri);

    std::string sourceUri;

private:
    std::shared_ptr<ISourceService> sourceClient_ = nullptr;
    std::vector<std::shared_ptr<AVSourceTrack>> tracks_ {};
    int32_t trackCount_ = -1;
    bool TrackIndexIsValid(uint32_t trackIndex);
};

class AVSourceTrackImpl : public AVSourceTrack, public NoCopyable {
public:
    AVSourceTrackImpl(AVSourceImpl *source, uint32_t trackIndex);
    ~AVSourceTrackImpl();

    int32_t SetTrackFormat(const Format &format) override;
    int32_t GetTrackFormat(Format &format) override;
private:
    uint32_t trackIndex_;
    AVSourceImpl* sourceImpl_;
};
} // namespace Media
} // namespace OHOS
#endif // AVSOURCE_IMPL_H

