#include <vector>
#include <queue>
#include <mutex>
#include <gtest/gtest.h>
#include "native_avcodec_audiodecoder.h"
#include "audio_ffmpeg_adapter.h"
#include "format.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"

extern "C" {
#include <string.h>
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavcodec/avcodec.h"
}

using namespace std;
using namespace testing::ext;
using namespace OHOS::Media;

namespace {
const string CODEC_NAME = "OH.Media.Codec.MP3.FFMPEGMp3";
} // namespace

class ADecSignal {
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
    explicit BufferCallback(ADecSignal *userData) : userData_(userData){}
    virtual ~BufferCallback() = default;
    ADecSignal *userData_;
    virtual void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    virtual void OnOutputFormatChanged(const Format &format) override;
    virtual void OnInputBufferAvailable(size_t index) override;
    virtual void OnOutputBufferAvailable(size_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
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

void BufferCallback::OnInputBufferAvailable(size_t index)
{
    unique_lock<mutex> lock(userData_->inMutex_);
    userData_->inIdxQueue_.push(index);
    userData_->inCond_.notify_all();
}

void BufferCallback::OnOutputBufferAvailable(size_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    unique_lock<mutex> lock(userData_->outMutex_);
    userData_->outIdxQueue_.push(index);
    userData_->infoQueue_.push(info);
    userData_->flagQueue_.push(flag);
    userData_->outCond_.notify_all();
    (void)flag;
}


class AudioCodeDecoderUnitTest : public testing::Test {
  public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int32_t ProceFunc();

  
  protected:
    int32_t index_;
    int64_t timeStamp_{0};

    ADecSignal *signal_;

    FILE *inFile_ {nullptr};
    FILE *dumpFd_ {nullptr};
    std::string codecName_;

    OHOS::Media::Format format_;
    std::shared_ptr<OHOS::Media::CodecBase> adec_ {nullptr};
};

void AudioCodeDecoderUnitTest::SetUpTestCase(void)
{   
    cout << "[SetUpTestCase]: " << endl;
}

void AudioCodeDecoderUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void AudioCodeDecoderUnitTest::SetUp(void)
{
    //TODO:need help
    cout << "[SetUp]: SetUp!!!" << endl;
    codecName_ = CODEC_NAME;
    adec_ = std::make_shared<OHOS::Media::AudioFFMpegAdapter>(codecName_);
    ASSERT_NE(nullptr, adec_);

    signal_ = new ADecSignal();
    ASSERT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->SetCallback(std::shared_ptr<AVCodecCallback>(new BufferCallback(signal_))));
}

void AudioCodeDecoderUnitTest::TearDown(void)
{
    adec_->Release();
    cout << "[TearDown]: over!!!" << endl;
}


int32_t AudioCodeDecoderUnitTest::ProceFunc(void)
{   
    format_.PutIntValue("channel_count", 2);
    format_.PutIntValue("sample_rate", 8000);
    format_.PutLongValue("bitrate", 128000);

    if (adec_->Configure(format_) != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }else if (adec_->Start() != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Configure_01, TestSize.Level1)
{ 
    //lack of correct key
    format_.PutIntValue("width", 0);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Configure_02, TestSize.Level1)
{
    //correct key input
    format_.PutIntValue("channel_count", 2);
    format_.PutIntValue("sample_rate", 8000);
    format_.PutLongValue("bitrate", 128000);

    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Configure_03, TestSize.Level1)
{
    //correct key input with redundancy key input
    format_.PutIntValue("channel_count", 2);
    format_.PutIntValue("sample_rate", 8000);
    format_.PutLongValue("bitrate", 128000);
    format_.PutIntValue("width", 0);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Configure_04, TestSize.Level1)
{
    //correct key input with wrong value type input
    format_.PutIntValue("channel_count", 2);
    format_.PutIntValue("sample_rate", 8000);
    format_.PutIntValue("bitrate", 128000);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Configure_05, TestSize.Level1)
{
    //correct key input with wrong value type input
    format_.PutIntValue("channel_count", 3);
    format_.PutIntValue("sample_rate", 9999990);
    format_.PutLongValue("bitrate", 128000);
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Configure_06, TestSize.Level1)
{
    //empty format input
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Start_01, TestSize.Level1)
{ 
    //correct flow 1
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Start_02, TestSize.Level1)
{
    //correct flow 2
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Start_03, TestSize.Level1)
{
    //wrong flow 1
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Start_04, TestSize.Level1)
{
    //wrong flow 2
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Start());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Pause_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Pause());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Resume_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Pause());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Resume());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Stop_01, TestSize.Level1)
{ 
    //correct flow 1
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Stop_02, TestSize.Level1)
{
    //correct flow 2
    format_.PutIntValue("channel_count", 2);
    format_.PutIntValue("sample_rate", 8000);
    format_.PutLongValue("bitrate", 128000);
    format_.PutIntValue("width", 0);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Flush_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Flush());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Flush_02, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Flush());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Reset_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Reset_02, TestSize.Level1)
{
    format_.PutIntValue("channel_count", 2);
    format_.PutIntValue("sample_rate", 8000);
    format_.PutLongValue("bitrate", 128000);
    format_.PutIntValue("width", 0);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Reset_03, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Release_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Release_02, TestSize.Level1)
{
    format_.PutIntValue("channel_count", 2);
    format_.PutIntValue("sample_rate", 8000);
    format_.PutLongValue("bitrate", 128000);
    format_.PutIntValue("width", 0);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Release_03, TestSize.Level1)
{
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_NotifyEos_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->NotifyEos());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_SetParameter_01, TestSize.Level1)
{ 
    //尚未实现
    format_.PutIntValue("channel_count", 2);
    format_.PutIntValue("sample_rate", 8000);
    format_.PutLongValue("bitrate", 128000);
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE, adec_->SetParameter(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_GetOutputFormat_01, TestSize.Level1)
{ 
    //尚未实现
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->GetOutputFormat(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_GetInputBuffer_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    sleep(1);
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;
    index_ = -1;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = 1024;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = 0;
    signal_->inIdxQueue_.push(index_);
    index_ = signal_->inIdxQueue_.front();
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_NE(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_QueueInputBuffer_01, TestSize.Level1)
{ 
    AVCodecBufferInfo info;
    AVCodecBufferFlag flag;

    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    sleep(1);
    // case0 传参异常
    index_ = -1;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY, adec_->QueueInputBuffer(index_, info, flag));
    // case1 info未赋值
    index_ = 1024;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY, adec_->QueueInputBuffer(index_, info, flag));

    // case2 EOS帧数据
    index_ = 0;
    info.presentationTimeUs = 0;
    info.size = -1;
    info.offset = 0;
    flag = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->QueueInputBuffer(index_, info, flag));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_GetOutputBuffer_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;

    //case1 传参异常
    index_ = -1;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
    index_ = 1024;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_ReleaseOutputBuffer_01, TestSize.Level1)
{
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());

    //case1 传参异常
    index_ = -1;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
    index_ = 1024;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
    // case2 传参正常
    index_ = 0;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->ReleaseOutputBuffer(index_));
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}