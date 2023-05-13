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

#include "codeclist_inner_mock.h"

namespace OHOS {
namespace Media {
std::string CodecListInnerMock::FindDecoder(std::shared_ptr<FormatMock> &format)
{
    if (codeclist_ != nullptr) {
        auto formatMock = std::static_pointer_cast<AVFormatInnerMock>(format);
        return codeclist_->FindDecoder(formatMock->GetFormat());
    }
    return "";
}

std::string CodecListInnerMock::FindEncoder(std::shared_ptr<FormatMock> &format)
{
    if (codeclist_ != nullptr) {
        auto formatMock = std::static_pointer_cast<AVFormatInnerMock>(format);
        return codeclist_->FindEncoder(formatMock->GetFormat());
    }
    return "";
}

CapabilityData CodecListInnerMock::CreateCapability(const std::string codecName)
{
    CapabilityData capabilityData;
    if (codeclist_ != nullptr) {
        return codeclist_->CreateCapability(codecName);
    }
    return capabilityData;
}
} // namespace Media
} // namespace OHOS