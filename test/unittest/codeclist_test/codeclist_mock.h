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

#ifndef CODECLIST_MOCK_H
#define CODECLIST_MOCK_H

#include <string>
#include "avcodec_list.h"
#include "avcodec_info.h"
#include "native_averrors.h"
#include "avformat_mock.h"
#include "native_avcodec_base.h"
#include "avcodec_common.h"
#include "nocopyable.h"
#include "avformat_mock.h"

namespace OHOS {
namespace Media {
class CodecListMock : public NoCopyable {
public:
    virtual ~CodecListMock() = default;
    virtual std::string FindDecoder(std::shared_ptr<FormatMock> &format) = 0;
    virtual std::string FindEncoder(std::shared_ptr<FormatMock> &format) = 0;
    virtual CapabilityData CreateCapability(const std::string codecName) = 0;
};

class __attribute__((visibility("default"))) CodecListMockFactory {
public:
    static std::shared_ptr<CodecListMock> CreateCodecList();
private:
    CodecListMockFactory() = delete;
    ~CodecListMockFactory() = delete;
};
}  // namespace Media
}  // namespace OHOS
#endif // CODECLIST_MOCK_H