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
#ifndef AVCODEC_LIST_IMPL_H
#define AVCODEC_LIST_IMPL_H

#include "avcodec_info.h"
#include "avcodec_list.h"
#include "nocopyable.h"
#include "i_codeclist_service.h"

namespace OHOS {
namespace Media {
class AVCodecListImpl : public AVCodecList, public NoCopyable {
public:
    AVCodecListImpl();
    ~AVCodecListImpl();
    int32_t Init();
    // AVCodecList
    std::string FindDecoder(const Format &format) override;
    std::string FindEncoder(const Format &format) override;
    CapabilityData CreateCapability(const std::string codecName) override;
    
private:
    std::shared_ptr<ICodecListService> codecListService_ = nullptr;
};
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_LIST_IMPL_H

