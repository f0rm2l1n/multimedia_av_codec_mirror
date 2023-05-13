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

#ifndef CODECLIST_INNER_MOCK_H
#define CODECLIST_INNER_MOCK_H

#include "codeclist_mock.h"
#include "avformat_inner_mock.h"
#include "avcodec_common.h"
#include "avcodec_list.h"

namespace OHOS {
namespace Media {
class CodecListInnerMock : public CodecListMock {
public:
    explicit CodecListInnerMock(std::shared_ptr<AVCodecList> codeclist) : codeclist_(codeclist) {}
    ~CodecListInnerMock() = default;
    std::string FindDecoder(std::shared_ptr<FormatMock> &format) override;
    std::string FindEncoder(std::shared_ptr<FormatMock> &format) override;
    CapabilityData CreateCapability(const std::string codecName) override;
private:
    std::shared_ptr<AVCodecList> codeclist_ = nullptr;
};
} // namespace Media
} // namespace OHOS
#endif // CODECLIST_INNER_MOCK_H