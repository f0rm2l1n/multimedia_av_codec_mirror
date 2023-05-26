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

#include <cstring>
#include <mutex>
#include <queue>
#include <vector>
#include <gtest/gtest.h>
#include "fcodec.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/mem.h"
}

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::Codec;
namespace {
const string CODEC_NAME = "video_decoder.avc";
constexpr uint32_t DEFAULT_WIDTH = 480;
constexpr uint32_t DEFAULT_HEIGHT = 272;
} // namespace

namespace OHOS {
namespace Media {
class VDecSignal {
public:
    std::mutex inMutex_;
    std::mutex outMutex_;
    std::condition_variable inCond_;
    std::condition_variable outCond_;
    std::queue<int32_t> inIdxQueue_;
    std::queue<int32_t> outIdxQueue_;
    std::queue<AVCodecBufferInfo> infoQueue_;
    std::queue<AVCodecBufferFlag> flagQueue_;
};

class BufferCallback : public AVCodecCallback {
public:
    explicit BufferCallback(VDecSignal *userData) : userData_(userData) {}
    virtual ~BufferCallback() = default;
    VDecSignal *userData_;
    void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    void OnOutputFormatChanged(const Format &format) override;
    void OnInputBufferAvailable(uint32_t index) override;
    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
};

void BufferCallback::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
    (void)errorType;
    cout << "Error errorCode=" << errorCode << endl;
}

void BufferCallback::OnOutputFormatChanged(const Format &format)
{
    (void)format;
    cout << "Format Changed" << endl;
}

void BufferCallback::OnInputBufferAvailable(uint32_t index)
{
    unique_lock<mutex> lock(userData_->inMutex_);
    userData_->inIdxQueue_.push(index);
    userData_->inCond_.notify_all();
}

void BufferCallback::OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    unique_lock<mutex> lock(userData_->outMutex_);
    userData_->outIdxQueue_.push(index);
    userData_->infoQueue_.push(info);
    userData_->flagQueue_.push(flag);
    userData_->outCond_.notify_all();
    (void)flag;
}

class FCodecUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int32_t ProceFunc();

protected:
    int32_t index_;
    int64_t timeStamp_ = 0;

    VDecSignal *signal_;

    FILE *inFile_ = nullptr;
    FILE *dumpFd_ = nullptr;

    std::string codecName_;
    Format format;
    std::shared_ptr<OHOS::Media::CodecBase> vdec_ = nullptr;
};

void FCodecUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void FCodecUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void FCodecUnitTest::SetUp(void)
{
    codecName_ = "";
    vdec_ = std::make_shared<OHOS::Media::Codec::FCodec>(codecName_);
    ASSERT_EQ(nullptr, vdec_);

    codecName_ = CODEC_NAME;
    vdec_ = std::make_shared<OHOS::Media::Codec::FCodec>(codecName_);
    ASSERT_NE(nullptr, vdec_);

    signal_ = new VDecSignal();
    ASSERT_EQ(AVCS_ERR_OK,
              vdec_->SetCallback(std::shared_ptr<AVCodecCallback>(std::make_shared<BufferCallback>(signal_))));
}

void FCodecUnitTest::TearDown(void)
{
    vdec_->Release();
    cout << "[TearDown]: over!!!" << endl;
}

int32_t FCodecUnitTest::ProceFunc(void)
{
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_HEIGHT);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));

    if (vdec_->Configure(format) != AVCS_ERR_OK) {
        cout << "[Configure]: failed to Configure" << endl;
        return AVCS_ERR_UNKNOWN;
    } else if (vdec_->Start() != AVCS_ERR_OK) {
        cout << "[Start]: failed Start" << endl;
        return AVCS_ERR_UNKNOWN;
    }
    format = Format();
    return AVCS_ERR_OK;
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_01, TestSize.Level1)
{
    EXPECT_NE(AVCS_ERR_INVALID_STATE, vdec_->Configure(format));
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_02, TestSize.Level1)
{
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 0);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_HEIGHT);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_03, TestSize.Level1)
{
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 0);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_04, TestSize.Level1)
{
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_HEIGHT);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, 1);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_05, TestSize.Level1)
{
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_HEIGHT);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, 1);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_06, TestSize.Level1)
{
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_HEIGHT);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE, 1);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_INVALID_STATE, vdec_->SetParameter(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_02, TestSize.Level1)
{
    EXPECT_EQ(ProceFunc(), AVCS_ERR_OK);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_03, TestSize.Level2)
{
    EXPECT_EQ(ProceFunc(), AVCS_ERR_OK);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_04, TestSize.Level3)
{
    EXPECT_EQ(ProceFunc(), AVCS_ERR_OK);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, 1);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_05, TestSize.Level4)
{
    EXPECT_EQ(ProceFunc(), AVCS_ERR_OK);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, 1);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_06, TestSize.Level1)
{
    EXPECT_EQ(ProceFunc(), AVCS_ERR_OK);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE, 1);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_GetInputbuffer_01, TestSize.Level1)
{
    EXPECT_EQ(ProceFunc(), AVCS_ERR_OK);
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;
    index_ = -1;
    buffer = vdec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = 1024;
    buffer = vdec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = signal_->inIdxQueue_.front();
    buffer = vdec_->GetInputBuffer(index_);
    EXPECT_NE(nullptr, buffer);
}

HWTEST_F(FCodecUnitTest, fcodec_QueueInputbuffer_01, TestSize.Level1)
{
    AVCodecBufferInfo info;
    AVCodecBufferFlag flag = AVCODEC_BUFFER_FLAG_NONE;

    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    // case0 传参异常
    index_ = -1;
    EXPECT_EQ(AVCS_ERR_INVALID_VAL, vdec_->QueueInputBuffer(index_, info, flag));
    // case1 info未赋值
    index_ = 1024;
    EXPECT_EQ(AVCS_ERR_INVALID_VAL, vdec_->QueueInputBuffer(index_, info, flag));

    // case2 EOS帧数据
    index_ = 0;
    info.presentationTimeUs = 0;
    info.size = -1;
    info.offset = 0;
    flag = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_EQ(AVCS_ERR_OK, vdec_->QueueInputBuffer(index_, info, flag));
}

HWTEST_F(FCodecUnitTest, fcodec_QueueInputbuffer_02, TestSize.Level1)
{
    AVCodecBufferInfo info;
    AVCodecBufferFlag flag = AVCODEC_BUFFER_FLAG_NONE;

    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    // case0 传参异常
    index_ = -1;
    EXPECT_EQ(AVCS_ERR_INVALID_VAL, vdec_->QueueInputBuffer(index_, info, flag));
    // case1 info未赋值
    index_ = 1024;
    EXPECT_EQ(AVCS_ERR_INVALID_VAL, vdec_->QueueInputBuffer(index_, info, flag));

    // case3 正常帧数据
    index_ = 0;
    info.presentationTimeUs = timeStamp_;
    info.size = 1024;
    info.offset = 0;
    flag = AVCODEC_BUFFER_FLAG_NONE;
    EXPECT_EQ(AVCS_ERR_OK, vdec_->QueueInputBuffer(index_, info, flag));
}

HWTEST_F(FCodecUnitTest, fcodec_GetOutputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;

    // case1 传参异常
    index_ = -1;
    buffer = vdec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
    index_ = 1024;
    buffer = vdec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    // case2 传参正常
    index_ = 0;
    buffer = vdec_->GetOutputBuffer(index_);
    EXPECT_NE(nullptr, buffer);
}

HWTEST_F(FCodecUnitTest, fcodec_ReleaseOutputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());

    // case1 传参异常
    index_ = -1;
    EXPECT_NE(AVCS_ERR_INVALID_VAL, vdec_->ReleaseOutputBuffer(index_));
    index_ = 1024;
    EXPECT_NE(AVCS_ERR_INVALID_VAL, vdec_->ReleaseOutputBuffer(index_));
    // case2 传参正常
    index_ = 0;
    EXPECT_EQ(AVCS_ERR_INVALID_VAL, vdec_->ReleaseOutputBuffer(index_));
}

HWTEST_F(FCodecUnitTest, fcodec_GetOutputFormat_01, TestSize.Level1)
{
    // case1 传参异常
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 0);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 0);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, 1);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format));
    format = Format();
    EXPECT_EQ(AVCS_ERR_OK, vdec_->GetOutputFormat(format));
    format = Format();

    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 0);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 0);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, -1);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format));
    format = Format();
    EXPECT_EQ(AVCS_ERR_OK, vdec_->GetOutputFormat(format));
    format = Format();

    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, -1);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, -1);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, -1);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format));
    format = Format();
    EXPECT_EQ(AVCS_ERR_OK, vdec_->GetOutputFormat(format));
    format = Format();

    // case2 传参正常
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, DEFAULT_WIDTH);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, DEFAULT_HEIGHT);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE,
                       static_cast<int32_t>(VideoRotation::VIDEO_ROTATION_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE,
                       static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format));
    format = Format();
    EXPECT_EQ(AVCS_ERR_OK, vdec_->GetOutputFormat(format));
    format = Format();
}

HWTEST_F(FCodecUnitTest, fcodec_Operating_procedures_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Stop());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Start());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Stop());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Release());
}

HWTEST_F(FCodecUnitTest, fcodec_Operating_procedures_02, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Stop());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Start());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Reset());
    ASSERT_EQ(AVCS_ERR_OK,
              vdec_->SetCallback(std::shared_ptr<AVCodecCallback>(std::make_shared<BufferCallback>(signal_))));
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Stop());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Release());
}

HWTEST_F(FCodecUnitTest, fcodec_Operating_procedures_03, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Flush());
}

HWTEST_F(FCodecUnitTest, fcodec_Operating_procedures_04, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    ASSERT_EQ(AVCS_ERR_INVALID_STATE, vdec_->Start());
}

HWTEST_F(FCodecUnitTest, fcodec_Operating_procedures_05, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Stop());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Stop());
}

HWTEST_F(FCodecUnitTest, fcodec_Operating_procedures_06, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Reset());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Reset());
}

HWTEST_F(FCodecUnitTest, fcodec_Operating_procedures_07, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Flush());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Flush());
}

HWTEST_F(FCodecUnitTest, fcodec_Operating_procedures_08, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Stop());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Release());
    ASSERT_EQ(AVCS_ERR_OK, vdec_->Release());
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
} // namespace Media
} // namespace OHOS
