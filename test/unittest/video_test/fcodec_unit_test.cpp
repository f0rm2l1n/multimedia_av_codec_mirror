#include <vector>
#include <queue>
#include <mutex>
#include "fcodec.h"
#include <gtest/gtest.h>
#include "format.h"
#include "surface/surface.h"
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
// #if 0
namespace {
const string CODEC_NAME = "video_decoder.avc";
// const string VIDEOMIMETYPE = "video/avc";
constexpr uint32_t DEFAULT_WIDTH = 480;
constexpr uint32_t DEFAULT_HEIGHT = 272;
const uint32_t YUV420P_ = 3; 
constexpr uint32_t DEFAULT_FRAME_RATE = 30;
const uint32_t FLAG_IS_ASNY = 1;
} // namespace


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
    explicit BufferCallback(VDecSignal *userData) : userData_(userData){}
    virtual ~BufferCallback() = default;
    VDecSignal *userData_;
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


class FCodecUnitTest : public testing::Test {
  public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int32_t ProceFunc();


  protected:
    int32_t index_;
    int64_t timeStamp_{0};

    VDecSignal *signal_;

    FILE *inFile_ {nullptr};
    FILE *dumpFd_ {nullptr};

    std::string codecName_;
    OHOS::Media::Format format_;
    std::shared_ptr<OHOS::Media::CodecBase> vdec_ {nullptr};
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
    ASSERT_EQ(AVCS_ERR_OK, vdec_->SetCallback(std::shared_ptr<AVCodecCallback>(new BufferCallback(signal_))));
}

void FCodecUnitTest::TearDown(void)
{   
  vdec_->Release();
  cout << "[TearDown]: over!!!" << endl;

}

int32_t FCodecUnitTest::ProceFunc(void)
{   
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);

    if (vdec_->Configure(format_) != AVCS_ERR_OK) {
        cout << "[Configure]: failed to Configure" << endl;
        return AVCS_ERR_UNKNOWN;
    }else if (vdec_->Start() != AVCS_ERR_OK) {
        cout << "[Start]: failed Start" << endl;
        return AVCS_ERR_UNKNOWN;
    }
    return AVCS_ERR_OK;
}


HWTEST_F(FCodecUnitTest, fcodec_Configure_01, TestSize.Level1)
{   
    // case1 codecType_
    EXPECT_NE(AVCS_ERR_INVALID_STATE, vdec_->Configure(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_02, TestSize.Level1)
{
    format_.PutIntValue("width", 0);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_03, TestSize.Level1)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", 0);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_04, TestSize.Level1)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", 1);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_05, TestSize.Level1)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", 0);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_06, TestSize.Level1)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_07, TestSize.Level1)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", 0);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_Configure_08, TestSize.Level1)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->Configure(format_));
}


HWTEST_F(FCodecUnitTest, fcodec_SetParameter_01, TestSize.Level1)
{
    EXPECT_NE(AVCS_ERR_INVALID_STATE, vdec_->SetParameter(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_02, TestSize.Level1)
{
    format_.PutIntValue("width", 0);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_03, TestSize.Level2)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", 0);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_04, TestSize.Level3)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", 1);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_05, TestSize.Level4)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", 0);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_06, TestSize.Level1)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_07, TestSize.Level1)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", 0);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format_));
}

HWTEST_F(FCodecUnitTest, fcodec_SetParameter_08, TestSize.Level1)
{
    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->SetParameter(format_));
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
    AVCodecBufferFlag flag;

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
    AVCodecBufferFlag flag;

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

    //case1 传参异常
    index_ = -1;
    buffer = vdec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
    index_ = 1024;
    buffer = vdec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    //case2 传参正常
    index_ = 0;
    buffer = vdec_->GetOutputBuffer(index_);
    EXPECT_NE(nullptr, buffer);
}


HWTEST_F(FCodecUnitTest, fcodec_ReleaseOutputBuffer_01, TestSize.Level1)
{   
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());

    //case1 传参异常
    index_ = -1;
    EXPECT_NE(AVCS_ERR_INVALID_VAL, vdec_->ReleaseOutputBuffer(index_));
    index_ = 1024;
    EXPECT_NE(AVCS_ERR_INVALID_VAL, vdec_->ReleaseOutputBuffer(index_));
    //case2 传参正常
    index_ = 0;
    EXPECT_EQ(AVCS_ERR_INVALID_VAL, vdec_->ReleaseOutputBuffer(index_));
}


HWTEST_F(FCodecUnitTest, fcodec_GetOutputFormat_01, TestSize.Level1)
{   
    //case1 传参异常
    format_.PutIntValue("width", 0);
    format_.PutIntValue("height", 0);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->GetOutputFormat(format_));

    format_.PutIntValue("width", 0);
    format_.PutIntValue("height", 0);
    format_.PutIntValue("pix_fmt", 1);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->GetOutputFormat(format_));

    format_.PutIntValue("width", 0);
    format_.PutIntValue("height", 0);
    format_.PutIntValue("pix_fmt", -1);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->GetOutputFormat(format_));

    format_.PutIntValue("width", -1);
    format_.PutIntValue("height", -1);
    format_.PutIntValue("pix_fmt", -1);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->GetOutputFormat(format_));
    //case2 传参正常

    format_.PutIntValue("width", DEFAULT_WIDTH);
    format_.PutIntValue("height", DEFAULT_HEIGHT);
    format_.PutIntValue("pix_fmt", YUV420P_);
    format_.PutIntValue("frame_rate", DEFAULT_FRAME_RATE);
    format_.PutIntValue("flag_is_async", FLAG_IS_ASNY);
    EXPECT_EQ(AVCS_ERR_OK, vdec_->GetOutputFormat(format_));
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
    ASSERT_EQ(AVCS_ERR_OK, vdec_->SetCallback(std::shared_ptr<AVCodecCallback>(new BufferCallback(signal_))));
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
// 
int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
// #endif