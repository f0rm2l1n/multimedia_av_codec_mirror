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

#include "codeclist_capi_mock.h"
#include "native_avcodec_list.h"

namespace OHOS {
namespace Media {
std::string CodecListCapiMock::FindDecoder(std::shared_ptr<FormatMock> &format)
{
    auto formatMock = std::static_pointer_cast<AVFormatCapiMock>(format);
    std::string codecName = OH_AVCodec_FindDecoder(formatMock->GetFormat());
    return codecName;
}

std::string CodecListCapiMock::FindEncoder(std::shared_ptr<FormatMock> &format)
{
    auto formatMock = std::static_pointer_cast<AVFormatCapiMock>(format);
    std::string codecName = OH_AVCodec_FindEncoder(formatMock->GetFormat());
    return codecName;
}

CapabilityData CodecListCapiMock::CreateCapability(const std::string codecName)
{
    // TODO: 接口还在评审
    // return *OH_AVCodec_CreateCapability(codecName.c_str());
    CapabilityData capability;
    return capability;
}
} // namespace Media
} // namespace OHOS