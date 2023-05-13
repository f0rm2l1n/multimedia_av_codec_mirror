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

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <fcntl.h>
#include "codeclist_unit_test.h"

using namespace testing::ext;
using namespace OHOS::Media;

template<typename TCodecMock>
void ReleaseCodec(vector<TCodecMock> &codecLst)
{
    uint32_t ret;
    for (auto &codec : codecLst) {
        if (codec != nullptr) {
            ret = codec->Destroy();
            EXPECT_EQ(AV_ERR_OK, ret);
            codec = nullptr;
        }
    }
    codecLst.swap(vector<TCodecMock>());
}

void CodecListUnitTest::SetUpTestCase() {}

void CodecListUnitTest::TearDownTestCase() {}

void CodecListUnitTest::SetUp()
{
    codeclist_ = CodecListMockFactory::CreateCodecList();
    EXPECT_NE(codeclist_, nullptr);
}

void CodecListUnitTest::TearDown() {}

/**
 * @tc.name: CodecList_FindVideoDecoder_001
 * @tc.desc: CodecList FindVideoDecoder
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindVideoDecoder_001, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    ASSERT_NE(nullptr, format);
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::VIDEO_AVC);
    (void)format->PutIntValue(bitrateKey_, MAX_VIDEO_BITRATE);
    (void)format->PutIntValue(widthKey_, DEFAULT_WIDTH);
    (void)format->PutIntValue(heightKey_, DEFAULT_HEIGHT);
    (void)format->PutIntValue(pixelFormatKey_, VideoPixelFormat::NV12);
    (void)format->PutIntValue(frameRateKey_, MAX_FRAME_RATE);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("video_decoder.avc", codecName);
}

/**
 * @tc.name: CodecList_FindVideoDecoder_002
 * @tc.desc: CodecList FindVideoDecoder, Negative PixelFormat
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindVideoDecoder_002, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    ASSERT_NE(nullptr, format);
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::VIDEO_AVC);
    (void)format->PutIntValue(bitrateKey_, MAX_VIDEO_BITRATE);
    (void)format->PutIntValue(widthKey_, DEFAULT_WIDTH);
    (void)format->PutIntValue(heightKey_, DEFAULT_HEIGHT);
    (void)format->PutIntValue(pixelFormatKey_, VideoPixelFormat::SURFACE_FORMAT);// Negative parameters
    (void)format->PutIntValue(frameRateKey_, MAX_FRAME_RATE);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("", codecName);
}

/**
 * @tc.name: CodecList_FindVideoDecoder_003
 * @tc.desc: CodecList FindVideoDecoder, Negative Width
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindVideoDecoder_003, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    ASSERT_NE(nullptr, format);
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::VIDEO_AVC);
    (void)format->PutIntValue(bitrateKey_, MAX_VIDEO_BITRATE);
    (void)format->PutIntValue(widthKey_, 15361);
    (void)format->PutIntValue(heightKey_, DEFAULT_HEIGHT);
    (void)format->PutIntValue(pixelFormatKey_, VideoPixelFormat::SURFACE_FORMAT);// Negative parameters
    (void)format->PutIntValue(frameRateKey_, MAX_FRAME_RATE);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("", codecName);
}

/**
 * @tc.name: CodecList_FindVideoDecoder_004
 * @tc.desc: CodecList FindVideoDecoder, Negative Height
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindVideoDecoder_004, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    ASSERT_NE(nullptr, format);
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::VIDEO_AVC);
    (void)format->PutIntValue(bitrateKey_, MAX_VIDEO_BITRATE);
    (void)format->PutIntValue(widthKey_, DEFAULT_WIDTH);
    (void)format->PutIntValue(heightKey_, 31);
    (void)format->PutIntValue(pixelFormatKey_, VideoPixelFormat::SURFACE_FORMAT);// Negative parameters
    (void)format->PutIntValue(frameRateKey_, MAX_FRAME_RATE);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("", codecName);
}

// /**
//  * @tc.name: CodecList_FindVideoEncoder_001
//  * @tc.desc: CodecList FindVideoEncoder
//  * @tc.type: FUNC
//  * @tc.require:
//  */
// HWTEST_F(CodecListUnitTest, CodecList_FindVideoEncoder_001, TestSize.Level0)
// {
//     std::string codecName;
//     std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
//     ASSERT_NE(nullptr, format);
//     (void)format->PutStringValue(codecMimeKey_, CodecMimeType::VIDEO_MPEG4);
//     (void)format->PutIntValue(bitrateKey_, MAX_VIDEO_BITRATE);
//     (void)format->PutIntValue(widthKey_, DEFAULT_WIDTH);
//     (void)format->PutIntValue(heightKey_, DEFAULT_HEIGHT);
//     (void)format->PutIntValue(pixelFormatKey_, VideoPixelFormat::NV21);
//     (void)format->PutIntValue(frameRateKey_, MAX_FRAME_RATE);
//     codecName = codeclist_->FindEncoder(format);
//     EXPECT_EQ("video/mp4v-es", codecName);
// }

/**
 * @tc.name: CodecList_FindAudioDecoder_001
 * @tc.desc: CodecList FindAudioDecoder
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioDecoder_001, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_MPEG);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE);
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT);
    (void)format->PutIntValue(sampleRateKey_, DEFAULT_SAMPLE_RATE);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("avdec_mp3", codecName);
}

/**
 * @tc.name: CodecList_FindAudioDecoder_002
 * @tc.desc: CodecList FindAudioDecoder
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioDecoder_002, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_AAC);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE);
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT);
    (void)format->PutIntValue(sampleRateKey_, DEFAULT_SAMPLE_RATE);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("avdec_aac", codecName);
}

/**
 * @tc.name: CodecList_FindAudioDecoder_003
 * @tc.desc: CodecList FindAudioDecoder
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioDecoder_003, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_VORBIS);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE);
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT);
    (void)format->PutIntValue(sampleRateKey_, DEFAULT_SAMPLE_RATE);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("avdec_vorbis", codecName);
}

/**
 * @tc.name: CodecList_FindAudioDecoder_004
 * @tc.desc: CodecList FindAudioDecoder
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioDecoder_004, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_FLAC);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE);
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT);
    (void)format->PutIntValue(sampleRateKey_, DEFAULT_SAMPLE_RATE);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("avdec_flac", codecName);
}

/**
 * @tc.name: CodecList_FindAudioDecoder_005
 * @tc.desc: CodecList FindAudioDecoder
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioDecoder_005, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_OPUS);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE);
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT);
    (void)format->PutIntValue(sampleRateKey_, 48000);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("avdec_opus", codecName);
}

/**
 * @tc.name: CodecList_FindAudioDecoder_006
 * @tc.desc: CodecList FindAudioDecoder, Negative ChannelCount
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioDecoder_006, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_OPUS);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE);
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT + 1);// Negative parameters
    (void)format->PutIntValue(sampleRateKey_, DEFAULT_SAMPLE_RATE);
    codecName = codeclist_->FindDecoder(format);
    EXPECT_EQ("", codecName);
}

/**
 * @tc.name: CodecList_FindAudioEncoder_001
 * @tc.desc: CodecList FindAudioEncoder
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioEncoder_001, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_AAC);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE);
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT);
    (void)format->PutIntValue(sampleRateKey_, DEFAULT_SAMPLE_RATE);
    codecName = codeclist_->FindEncoder(format);
    EXPECT_EQ("avenc_aac", codecName);
}

/**
 * @tc.name: CodecList_FindAudioEncoder_002
 * @tc.desc: CodecList FindAudioEncoder
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioEncoder_002, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_OPUS);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE);
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT);
    (void)format->PutIntValue(sampleRateKey_, 48000);
    codecName = codeclist_->FindEncoder(format);
    EXPECT_EQ("avenc_opus", codecName);
}

/**
 * @tc.name: CodecList_FindAudioEncoder_003
 * @tc.desc: CodecList FindAudioEncoder, Negative SampleRate
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioEncoder_003, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_OPUS);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE);
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT);
    (void)format->PutIntValue(sampleRateKey_, DEFAULT_SAMPLE_RATE);// Negative parameters
    codecName = codeclist_->FindEncoder(format);
    EXPECT_EQ("", codecName);
}

/**
 * @tc.name: CodecList_FindAudioEncoder_004
 * @tc.desc: CodecList FindAudioEncoder, Negative Bitrate
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CodecListUnitTest, CodecList_FindAudioEncoder_004, TestSize.Level0)
{
    std::string codecName;
    std::shared_ptr<FormatMock> format = FormatMockFactory::CreateFormat();
    (void)format->PutStringValue(codecMimeKey_, CodecMimeType::AUDIO_OPUS);
    (void)format->PutIntValue(bitrateKey_, MAX_AUDIO_BITRATE + 1); // Negative parameters
    (void)format->PutIntValue(channelCountKey_, MAX_CHANNEL_COUNT);
    (void)format->PutIntValue(sampleRateKey_, 48000);
    codecName = codeclist_->FindEncoder(format);
    EXPECT_EQ("", codecName);
}
