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

#ifndef CODECLIST_UNIT_TEST_H
#define CODECLIST_UNIT_TEST_H

#include <vector>
#include "gtest/gtest.h"
#include "avcodec_info.h"
#include "codeclist_mock.h"
#include "media_description.h"

namespace OHOS {
namespace Media {
class CodecListUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);

protected:
    std::string codecMimeKey_{ MediaDescriptionKey::MD_KEY_CODEC_MIME };
    std::string bitrateKey_{ MediaDescriptionKey::MD_KEY_BITRATE };
    std::string widthKey_{ MediaDescriptionKey::MD_KEY_WIDTH };
    std::string heightKey_{ MediaDescriptionKey::MD_KEY_HEIGHT };
    std::string pixelFormatKey_{ MediaDescriptionKey::MD_KEY_PIXEL_FORMAT };
    std::string frameRateKey_{ MediaDescriptionKey::MD_KEY_FRAME_RATE };
    std::string channelCountKey_{ MediaDescriptionKey::MD_KEY_CHANNEL_COUNT };
    std::string sampleRateKey_{ "samplerate" }; // { MediaDescriptionKey::MD_KEY_SAMPLE_RATE }
    std::string bitDepthKey_{ MediaDescriptionKey::MD_KEY_SAMPLE_RATE };
    std::shared_ptr<CodecListMock> codeclist_{ nullptr };
};
} // namespace Media
} // namespace OHOS
#endif // CODECLIST_UNIT_TEST_H