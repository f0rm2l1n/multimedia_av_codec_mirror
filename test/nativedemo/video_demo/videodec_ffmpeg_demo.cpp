
#include <ctime>
#include "videodec_ffmpeg_demo.h"
#include "avcodec_common.h" //AVCodecBufferInfo
#include "securec.h"

extern "C" {
#include <string.h>
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavcodec/avcodec.h"
}

using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::Codec;
using namespace std;

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "FCodec"};

const string MIME_TYPE = "video/avc";
const string CODEC_NAME = "video_decoder.avc";

const int32_t VIDEO_INBUF_SIZE = 10240;
const int32_t VIDEO_REFILL_THRESH = 4096;
constexpr uint32_t DEFAULT_FRAME_RATE = 30;
constexpr uint32_t SAMPLE_DURATION_US = 23000;
constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;
} // namespace

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
    AVCODEC_LOGI("Error errorCode= %{public}d", errorCode);
}

void BufferCallback::OnOutputFormatChanged(const Format &format)
{
    (void)format;
    AVCODEC_LOGI("Format Changed");
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
}

VDecFfmpegSample::~VDecFfmpegSample()
{}

int64_t VDecFfmpegSample::GetSystemTimeUs()
{
    struct timespec now;
    (void)clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t nanoTime = (int64_t)now.tv_sec * NANOS_IN_SECOND + now.tv_nsec;

    return nanoTime / NANOS_IN_MICRO;
}

void VDecFfmpegSample::RunVideoDec(FILE *inFp, FILE *outFp, int32_t width, int32_t height, sptr<Surface> surface, std::string codeName)
{
    inFile_ = inFp;
    dumpFd_ = outFp;
    width_ = width;
    height_ = height;
    surface_ = surface;

    int err = CreateVideoDecoder(codeName);
    if (err != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: Failed to create video decoder");
        return;
    }
    AVCODEC_LOGI("[RunVideoDec]: Success to create video decoder");
    err = SetVideoDecoderCallback();
    if (err != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: Failed to setCallback");
        Release();
        return;
    }
    AVCODEC_LOGI("[RunVideoDec]: Success to setCallback");
    err = ConfigureVideoDecoder();
    if (err != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: Failed to configure video decoder");
        Release();
        return;
    }
    AVCODEC_LOGI("[RunVideoDec]: Success to configure video decoder");

    GetoutputformatVideoDecoder();

    if (surface_) {
        err = vdec_->SetOutputSurface(surface);
        if (err != AVCS_ERR_OK) {
            AVCODEC_LOGE("[RunVideoDec]: Failed to set output surface");
            Release();
            return;
        }
        AVCODEC_LOGI("[RunVideoDec]: Success to set output surface");
    }
    
    if (Start() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: Success to start");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    if (SetparameterVideoDecoder() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]:  Success to Setparameter");
    }
    
    GetoutputformatVideoDecoder();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    if (Stop() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: stop codec failed");
    }
    AVCODEC_LOGI("[RunVideoDec]: stop codec success");
    Release();
    AVCODEC_LOGI("[RunVideoDec]: Release codec success");
    // switch (val){
    // case 1:
    //     BasicTest1();
    //     break;
    // case 2:
    //     BasicTest2(surface);
    //     break;
    // case 3:
    //     BasicTest3();
    //     break;
    // case 4:
    //     BasicTest4();
    //     break;
    // default:
    //     AVCODEC_LOGI("[RunVideoDec]: End");
    // }
}

// 1. Start -> Stop -> Start -> Stop -> Release 流程OK
void VDecFfmpegSample::BasicTest1(){
    if (Start() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: Success to start");
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    AVCODEC_LOGI("[RunVideoDec]: End of running");
    if (Stop() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: stop codec failed");
    }
    if (Start() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: Success to restart");
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if (Stop() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: stop codec failed");
    }
    AVCODEC_LOGI("[RunVideoDec]: stop codec success");
    Release();
    AVCODEC_LOGI("[RunVideoDec]: Release codec success");
}
// // 2. Start -> Reset：codec回到initialized状态 -> 重新配置callback/config/
void VDecFfmpegSample::BasicTest2(){
    if (Start() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: Success to start");
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (Reset() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: Success to Reset");
    }
    int err;
    err = SetVideoDecoderCallback();
    if (err != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: Failed to setCallback");
        Release();
        return;
    }
    AVCODEC_LOGI("[RunVideoDec]: Success to setCallback");
    err = ConfigureVideoDecoder();
    if (err != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: Failed to configure video decoder");
        Release();
        return;
    }
    AVCODEC_LOGI("[RunVideoDec]: Success to configure video decoder");

    if (Start() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: Success to start");
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if (Stop() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: stop codec failed");
    }
    Release();
    AVCODEC_LOGI("[RunVideoDec]: Release codec success");
}

// 3. Flush: EOS调用，异步进入flush，可调用start； 同步还是running；FLAG_IS_ASNY = 0/1两种情况
void VDecFfmpegSample::BasicTest3(){
    if (Start() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: Success to start");
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (Flush() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: Flush codec failed");
    }
    if (Start() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: Success to start");
    }
    sleep(5);
    if (Stop() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: stop codec failed");
    }
    if (GetoutputformatVideoDecoder() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: get outputformat success");
    }
    Release();
}
// 4. 参数相关
void VDecFfmpegSample::BasicTest4(){
    if (Start() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: Success to start");
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (SetparameterVideoDecoder() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]:  fail to Setparameter");
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if (Stop() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[RunVideoDec]: stop codec failed");
    }
    if (GetoutputformatVideoDecoder() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[RunVideoDec]: get outputformat success");
    }
    Release();
    AVCODEC_LOGI("[RunVideoDec]: Release codec success");
}

int32_t VDecFfmpegSample::CreateVideoDecoder(string codeName)
{
    if (!codeName.empty()) {
        vdec_ = std::make_shared<FCodec>(codeName);// 当传入codecname，调用byname创建
    } else {
        vdec_ = std::make_shared<FCodec>(false, MIME_TYPE); // codecname为空，调用bymine创建
    }
    return vdec_ == nullptr ? AVCS_ERR_UNKNOWN : AVCS_ERR_OK;
}

int32_t VDecFfmpegSample::SetVideoDecoderCallback()
{
    signal_ = new VDecSignal();
    if (signal_ == nullptr) {
        AVCODEC_LOGE("[SetVideoDecoderCallback]: Failed to create new VDecSignal");
        return AVCS_ERR_UNKNOWN;
    }
    std::shared_ptr<BufferCallback> callback(new BufferCallback(signal_));
    return vdec_->SetCallback(callback);
}

int32_t VDecFfmpegSample::ConfigureVideoDecoder()
{
    Format format;
    format.PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, width_);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, height_);
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::BGRA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, static_cast<int32_t>(GraphicTransformType::GRAPHIC_ROTATE_90));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE, static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_TO_WINDOW));
    

    // 配置config信息
    if (vdec_->Configure(format) != AVCS_ERR_OK) {
        AVCODEC_LOGE("[ConfigureVideoDecoder]: Failed to config vdec_");
        return AVCS_ERR_UNKNOWN;
    }
    return AVCS_ERR_OK;
}

int32_t VDecFfmpegSample::SetparameterVideoDecoder()
{
    Format format;
    format.PutIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, static_cast<int32_t>(GraphicTransformType::GRAPHIC_ROTATE_180));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, static_cast<int32_t>(VideoPixelFormat::RGBA));
    format.PutIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE, static_cast<int32_t>(ScalingMode::SCALING_MODE_SCALE_CROP));

    // 配置config信息
    if (vdec_->SetParameter(format) != AVCS_ERR_OK) {
        AVCODEC_LOGE("[SetparameterVideoDecoder]: Failed to Setparameter");
        return AVCS_ERR_UNKNOWN;
    }
    return AVCS_ERR_OK;
}

int32_t VDecFfmpegSample::GetoutputformatVideoDecoder()
{
    Format format;
    if (vdec_->GetOutputFormat(format) != AVCS_ERR_OK) {
        AVCODEC_LOGE("[GetoutputformatVideoDecoder]: GetOutputFormat");
        return AVCS_ERR_UNKNOWN;
    }

    AVCODEC_LOGI("[GetoutputformatVideoDecoder]: %{public}s", format.Stringify().c_str());
    return AVCS_ERR_OK;
}

int32_t VDecFfmpegSample::Start()
{
    AVCODEC_LOGI("[Start]: ============start===========");
    int ret = vdec_->Start();
    if (ret != AVCS_ERR_OK) {
        AVCODEC_LOGE("[Start]: Failed to start codec");
        return ret;
    }
    AVCODEC_LOGI("[Start]: Success to start codec");
	
    AVCODEC_LOGI("[Start]: start to Create Extractor");
    if (CreateExtract() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[Start]: Create Extractor failed");
        isRunning_.store(false);
        (void)vdec_->Stop();
        ReleaseFile();
        CloseExtract();
        return AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("[Start]: Success to Create Extractor");
	
    isRunning_.store(true);
    if (surface_) {
        outputLoop_ = make_unique<thread>(&VDecFfmpegSample::OutputSurfaceFunc, this);
    }else {
        outputLoop_ = make_unique<thread>(&VDecFfmpegSample::OutputBufferFunc, this);
    }
    
    if (outputLoop_ == nullptr) {
        AVCODEC_LOGE("[Start]: Failed to create output loop");
        isRunning_.store(false);
        (void)vdec_->Stop();
        ReleaseFile();
        CloseExtract();
        StopInloop();
        return AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("[Start]: Success to create output loop");
    inputLoop_ = make_unique<thread>(&VDecFfmpegSample::InputFunc, this);
    if (inputLoop_ == nullptr) {
        AVCODEC_LOGE("[Start]: Failed to create input loop");
        isRunning_.store(false);
        (void)vdec_->Stop();
        ReleaseFile();
        CloseExtract();
        return AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("[Start]: Success to create input loop");
    return AVCS_ERR_OK;
}

int32_t VDecFfmpegSample::CreateExtract()
{
    // 查找码流解析器
    // AVCODEC_LOGI("====================== [CreateExtract] ======================");

    if (codec_ != nullptr || parser_ != nullptr || codec_ctx_ != nullptr || pkt_ != nullptr) {
        // AVCODEC_LOGI("[CreateExtract]: Extractor already create");
        return AVCS_ERR_UNKNOWN;
    }

    codec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (codec_ == nullptr) {
        return AVCS_ERR_UNKNOWN;
    }

    parser_ = av_parser_init(codec_->id);
    if (parser_ == nullptr) {
        return AVCS_ERR_UNKNOWN;
    }

    codec_ctx_ = avcodec_alloc_context3(codec_); // 分配codec使用的上下文
    if (codec_ctx_ == nullptr) {
        return AVCS_ERR_UNKNOWN;
    }

    if (avcodec_open2(codec_ctx_, codec_, NULL) < 0) // 将解码器和解码使用的上下文关联
    {
        return AVCS_ERR_UNKNOWN;
    }

    pkt_ = av_packet_alloc(); // 申请AVPacket本身的内存
    if (pkt_ == nullptr) {
        return AVCS_ERR_UNKNOWN;
    }

    return AVCS_ERR_OK;
}

int32_t VDecFfmpegSample::ExtractFrame()
{
    int32_t len = 0;
    int32_t ret = 0;

    if (inFile_ == nullptr) {
        AVCODEC_LOGE("[CreateExtract]: inFile_ is null!");
        return AVCS_ERR_UNKNOWN;
    }

    if (data_ == nullptr) {
        data_ = inbuf_;
        data_size_ = fread(inbuf_, 1, VIDEO_INBUF_SIZE, inFile_);
    }

    if ((data_size_ < VIDEO_REFILL_THRESH) && !file_end_) {
        memmove(inbuf_, data_, data_size_); // 剩余数据移动缓冲区前
        data_ = inbuf_;
        len = fread(data_ + data_size_, 1, VIDEO_INBUF_SIZE - data_size_, inFile_);
        if (len > 0) {
            data_size_ += len;
        } else if (len == 0 && data_size_ == 0) { // 完全没有数据设置为end
            file_end_ = 1;
            AVCODEC_LOGI("[ExtractFrame]: file end!");
        }
    }
    // 获取一个packet的数据量
    if (data_size_ > 0) {
        ret = av_parser_parse2(parser_, codec_ctx_, &pkt_->data, &pkt_->size, data_, data_size_, AV_NOPTS_VALUE,
                               AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            AVCODEC_LOGE("[ExtractFrame]: av_parser_parser2 Error!");
        }
        data_ += ret;
        data_size_ -= ret;
        if (pkt_->size) {
            return AVCS_ERR_OK;
        } else {
            return AVCS_ERR_UNKNOWN;
        }
    }
    return AVCS_ERR_UNKNOWN;
}

void VDecFfmpegSample::CloseExtract()
{
    unique_lock<mutex> lock(signal_->inMutex_);
    if (codec_ctx_ != nullptr) {
        avcodec_free_context(&codec_ctx_);

        codec_ctx_ = nullptr;
    }
    if (parser_ != nullptr) {
        av_parser_close(parser_);

        parser_ = nullptr;
    }
    if (pkt_ != nullptr) {
        av_packet_free(&pkt_);

        pkt_ = nullptr;
    }
    if (codec_ != nullptr) {
        codec_ = nullptr;
    }
}

void VDecFfmpegSample::InputFunc()
{
    uint32_t intervalUs = 1000 / DEFAULT_FRAME_RATE;
    AVCODEC_LOGI("[InputFunc]: Start input func");
    while (true) {
        if (!isRunning_.load()) {
            break;
        }

        int32_t index;
        unique_lock<mutex> lock(signal_->inMutex_);
        signal_->inCond_.wait(lock, [this]() { return signal_->inIdxQueue_.size() > 0; });
        if (!isRunning_.load()) {
            break;
        }
        index = signal_->inIdxQueue_.front();
        auto ret = inBufferMap_.find(index);
        if (ret == inBufferMap_.end()) {
            std::shared_ptr<AVSharedMemoryBase> buffer = vdec_->GetInputBuffer(index);
            if (buffer != nullptr) {
                inBufferMap_.insert(std::make_pair(index, buffer));

            } else {
                continue;
            }
        }
        std::shared_ptr<AVSharedMemoryBase> buffer = inBufferMap_[index];
        lock.unlock();

        
        if (ExtractFrame() != AVCS_ERR_OK && !file_end_) {
            //AVCODEC_LOGE("[InputFunc]: failed to ExtractFrame, index: %{public}d", index);
            continue;
        }
        

        AVCodecBufferInfo info;
        AVCodecBufferFlag flag;
        if (!file_end_) { // 正常帧数据
            uint8_t *ptr = buffer->GetBase();
            int32_t size = buffer->GetSize();
            CHECK_AND_RETURN_LOG((ptr != nullptr && size > pkt_->size && pkt_->size > 0),
                             "input buffer is invalid");
           
            if (memcpy_s(ptr, pkt_->size, pkt_->data, pkt_->size) != EOK) {
                AVCODEC_LOGE("Failed to memcpy");
            }

            //AVCODEC_LOGE("ptr:%{public}d, size:%{public}d, pkt size: %{public}d", ptr==nullptr, size, pkt_->size);

            info.presentationTimeUs = timeStamp_;
            info.size = pkt_->size;
            info.offset = 0;
            flag = AVCODEC_BUFFER_FLAG_NONE;
            timeStamp_ += SAMPLE_DURATION_US;
        } else { // 发送空帧数据
            info.presentationTimeUs = 0;
            info.size = 0;
            info.offset = 0;
            flag = AVCODEC_BUFFER_FLAG_EOS;
        }

        if (vdec_->QueueInputBuffer(index, info, flag) != AVCS_ERR_OK) {
            //AVCODEC_LOGE("[InputFunc]: failed to queue input buffer, index: " << index);
            continue;
        } 

        if (file_end_) {
            break; // 结束时候跳出循环
        }

        lock.lock();
        signal_->inIdxQueue_.pop();
        lock.unlock();
        file_num_read_++;

        usleep(intervalUs);
    }
}

void VDecFfmpegSample::OutputSurfaceFunc()
{
    AVCODEC_LOGI("[OutputSurfaceFunc]: Start output func");
    while (true) {
        if (!isRunning_.load()) {
            break;
        }
        AVCodecBufferInfo info;
        AVCodecBufferFlag flag;
        int32_t index;

        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.wait(lock, [this]() { return signal_->outIdxQueue_.size() > 0; });

        if (!isRunning_.load()) {
            break;
        }
        index = signal_->outIdxQueue_.front();
        info = signal_->infoQueue_.front();
        flag = signal_->flagQueue_.front();

        if (flag == AVCODEC_BUFFER_FLAG_EOS) {
            AVCODEC_LOGI("[OutputSurfaceFunc]: End of decoder, frame cnt: %{public}d", file_num_write_);
            break;
        }

        
        std::shared_ptr<AVSharedMemory> buffer = static_pointer_cast<AVSharedMemory>(vdec_->GetOutputBuffer(index));
        if (buffer == nullptr) {
            AVCODEC_LOGE("[OutputSurfaceFunc]: failed get output bufferEle, index: %{public}d", index);
            continue;
        }
        
        if(dumpFd_ != nullptr){
            uint8_t *ptr = buffer->GetBase();
            int32_t size = info.size;
            uint32_t flags = buffer->GetFlags();

            if(ptr != nullptr && size > 0){
                std::fwrite(ptr,size, 1, dumpFd_);
            }else{
                AVCODEC_LOGE("ptr:%{public}d, size:%{public}d,flags:%{public}d", ptr==nullptr, size, flags);
            }
        }
        
        if (vdec_->RenderOutputBuffer(index) != AVCS_ERR_OK) {
            AVCODEC_LOGE("[OutputSurfaceFunc]: RenderOutputBuffer fail");
            continue;
        }
        signal_->outIdxQueue_.pop();
        signal_->infoQueue_.pop();
        signal_->flagQueue_.pop();

        if (file_num_write_ % 50 == 0) {
            AVCODEC_LOGI("[OutputBufferFunc] write frame cnt: %{public}d", file_num_write_);
        }
        file_num_write_++;
    }
    
}

void VDecFfmpegSample::OutputBufferFunc()
{
    // uint8_t *scaleData = (uint8_t *)malloc(sizeof(uint8_t) * DEFAULT_WIDTH * DEFAULT_HEIGHT * 1.5);
    AVCODEC_LOGI("[OutputBufferFunc]: Start output func");
    while (true) {
        if (!isRunning_.load()) {
            break;
        }
        AVCodecBufferInfo info;
        AVCodecBufferFlag flag;
        int32_t index;

        unique_lock<mutex> lock(signal_->outMutex_);
        signal_->outCond_.wait(lock, [this]() { return signal_->outIdxQueue_.size() > 0; });

        if (!isRunning_.load()) {
            break;
        }
        index = signal_->outIdxQueue_.front();
        info = signal_->infoQueue_.front();
        flag = signal_->flagQueue_.front();
        lock.unlock();

        if (flag == AVCODEC_BUFFER_FLAG_EOS) {
            AVCODEC_LOGI("[OutputBufferFunc]: End of decoder, frame cnt: %{public}d", file_num_write_);
            break;
        }
        // int size = info.size;
        auto it = outBufferMap_.find(index);
        if (it == outBufferMap_.end()) {
            std::shared_ptr<AVSharedMemoryBase> buffer = vdec_->GetOutputBuffer(index);
            if (buffer != nullptr) {
                outBufferMap_.insert(std::make_pair(index, buffer));
                // AVCODEC_LOGI("[OutputBufferFunc]: success get output bufferEle, index: " << index);
            } else {
                // AVCODEC_LOGE("[OutputBufferFunc]: failed get output bufferEle, index: " << index);
                continue;
            }
        }
        std::shared_ptr<AVSharedMemoryBase> buffer = outBufferMap_[index];

        if (dumpFd_ != nullptr) {
            uint8_t *ptr = buffer->GetBase();
            int32_t size = info.size;
            uint32_t flags = buffer->GetFlags();

            if(ptr != nullptr && size > 0){
                std::fwrite(ptr,size, 1, dumpFd_);
            }else{
                AVCODEC_LOGE("ptr:%{public}d, size:%{public}d,flags:%{public}d", ptr==nullptr, size, flags);
            }
        }
        
        if (vdec_->ReleaseOutputBuffer(index) != AVCS_ERR_OK) {
            AVCODEC_LOGI("[OutputBufferFunc]: ReleaseOutputBuffer fail");
            continue;
        }
        // AVCODEC_LOGI("[OutputBufferFunc]: ReleaseOutputBuffer success");
        
        lock.lock();
        signal_->outIdxQueue_.pop();
        signal_->infoQueue_.pop();
        signal_->flagQueue_.pop();
        lock.unlock();

        if (file_num_write_ % 50 == 0) {
            AVCODEC_LOGI("[OutputBufferFunc] write frame cnt: %{public}d, size: %{public}d", file_num_write_, buffer->GetSize());
        }
        file_num_write_++;
    }
}

bool VDecFfmpegSample::IsRender()
{
    if (lastRenderedTimeUs_ == 0) {
        return true;
    }
    int64_t curTimeUs = GetSystemTimeUs();
    if (curTimeUs - lastRenderedTimeUs_ < 1000 / DEFAULT_FRAME_RATE / 2) {
        lastRenderedTimeUs_ = curTimeUs;
        return true;
    }

    return false;
}

int32_t VDecFfmpegSample::FlushStart()
{
    AVCODEC_LOGI("[FlushStart]: ============FlushStart===========");
    int ret = vdec_->Start();
    if (ret != AVCS_ERR_OK) {
        AVCODEC_LOGE("[FlushStart]: Failed to start codec");
        return ret;
    }
    AVCODEC_LOGI("[FlushStart]: Success to start codec");

    AVCODEC_LOGI("[FlushStart]: start to Create Extractor");
    if (CreateExtract() != AVCS_ERR_OK) {
        AVCODEC_LOGE("[FlushStart]: Create Extractor failed");
        isRunning_.store(false);
        (void)vdec_->Stop();
        ReleaseFile();
        CloseExtract();
        return AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("[FlushStart]: Success to Create Extractor");
    
    isRunning_.store(true);
    if (surface_) {
        outputLoop_ = make_unique<thread>(&VDecFfmpegSample::OutputSurfaceFunc, this);
    }else {
        outputLoop_ = make_unique<thread>(&VDecFfmpegSample::OutputBufferFunc, this);
    }
    if (outputLoop_ == nullptr) {
        AVCODEC_LOGE("[FlushStart]: Failed to create output loop");
        isRunning_.store(false);
        (void)vdec_->Stop();
        ReleaseFile();
        CloseExtract();
        StopInloop();
        return AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("[FlushStart]: Success to create output loop");

    inputLoop_ = make_unique<thread>(&VDecFfmpegSample::InputFunc, this);
    if (inputLoop_ == nullptr) {
        AVCODEC_LOGE("[FlushStart]: Failed to create input loop");
        isRunning_.store(false);
        (void)vdec_->Stop();
        ReleaseFile();
        CloseExtract();
        return AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("[FlushStart]: Success to create input loop");
    return AVCS_ERR_OK;
}

int32_t VDecFfmpegSample::Flush() // TODO 同步 & 异步： 异步需要调用start
{
    int ret;
    AVCODEC_LOGI("[Flush]: Flush starts");
    isRunning_.store(false);
    StopInloop();
    AVCODEC_LOGI("[Flush]: Stop Inloop success");
    StopOutloop();
    AVCODEC_LOGI("[Flush]: Stop outloop success");
    ret = vdec_->Flush();
    if (ret == AVCS_ERR_OK) {
        AVCODEC_LOGI("[Flush]: Flush codec success");
    } else {
        AVCODEC_LOGE("[Flush]: Flush codec failed");
    }
    ReleaseFile();
    AVCODEC_LOGI("[Flush]: Stop file success");
    CloseExtract();
    AVCODEC_LOGI("[Flush]: Stop extractor success");
    ResetBuffer();
    {
        unique_lock<mutex> lock(signal_->inMutex_);
        inBufferMap_.clear();
    }
    {
        unique_lock<mutex> lock(signal_->outMutex_);
        outBufferMap_.clear();
    }
    if (FlushStart() == AVCS_ERR_OK) {
        AVCODEC_LOGI("[Flush]: FlushStart success");
    }
    return ret;
}

int32_t VDecFfmpegSample::Reset() // 回到initialied状态
{
    isRunning_.store(false);
    StopInloop();
    AVCODEC_LOGI("[Reset]: Stop Inloop success");
    StopOutloop();
    AVCODEC_LOGI("[Reset]: Stop outloop success");
    auto ret = vdec_->Reset();
    if (ret == AVCS_ERR_OK) {
        AVCODEC_LOGI("[Reset]: Reset codec success");
    }
    CloseExtract();
    ReleaseFile();
    ResetBuffer();
    {
        unique_lock<mutex> lock(signal_->inMutex_);
        inBufferMap_.clear();
    }
    {
        unique_lock<mutex> lock(signal_->outMutex_);
        outBufferMap_.clear();
    }
    if (signal_) {
        signal_ = nullptr;
    }
    cb_.reset();
    return ret;
}

void VDecFfmpegSample::ResetBuffer()
{
    {
        unique_lock<mutex> lock(signal_->inMutex_);
        data_size_ = 0;
        if (data_ != nullptr) {
            // delete data_;
            data_ = nullptr;
        }
        file_end_ = 0;
        file_num_read_ = 0;
    }
    {
        unique_lock<mutex> lock(signal_->outMutex_);
        file_num_write_ = 0;
    }

    if (!surface_) {
        unique_lock<mutex> lock(signal_->inMutex_);
        std::queue<int32_t> empty;
        swap(empty, signal_->inIdxQueue_);
        signal_->inCond_.notify_all();
        lock.unlock();
    }
    unique_lock<mutex> lock(signal_->outMutex_);
    std::queue<int32_t> empty;
    swap(empty, signal_->outIdxQueue_);
    AVCODEC_LOGI("[ResetBuffer]: StopOutloop outIdxQueue_,outIdxQueue_.size(): %{public}d", signal_->outIdxQueue_.size());
    std::queue<AVCodecBufferInfo> empty1;
    swap(empty1, signal_->infoQueue_);
    AVCODEC_LOGI("[ResetBuffer]: StopOutloop infoQueue_");
    signal_->outCond_.notify_all();
    lock.unlock();
}

void VDecFfmpegSample::ReleaseFile()
{
    if (inFile_ != nullptr) {
        unique_lock<mutex> lock(signal_->inMutex_);
        fclose(inFile_);
        // delete inFile_;
        inFile_ = nullptr;
        AVCODEC_LOGI("[ReleaseFile]: Release InFile success");
    }
    if (dumpFd_ != nullptr) {
        unique_lock<mutex> lock(signal_->outMutex_);
        fclose(dumpFd_);
        // delete dumpFd_;
        dumpFd_ = nullptr;
        AVCODEC_LOGI("[ReleaseFile]: Release outFile success");
    }
}

void VDecFfmpegSample::Release()
{
    if (signal_ != nullptr) {
        // delete signal_;
        signal_ = nullptr;
    }
    AVCODEC_LOGI("[Release]: Release signal_ success");
    cb_.reset();
    AVCODEC_LOGI("[Release]: Release cb_ success");
    inBufferMap_.clear();
    outBufferMap_.clear();
    AVCODEC_LOGI("[Release]: Release BufferMap_ success");
    vdec_->Release();
    AVCODEC_LOGI("[Release]: Release codec success");
}

int32_t VDecFfmpegSample::Stop() // 回到config状态
{
    isRunning_.store(false);
    StopInloop();
    AVCODEC_LOGI("[Stop]: Stop Inloop success");
    StopOutloop();
    AVCODEC_LOGI("[Stop]: Stop outloop success");
    auto ret = vdec_->Stop();
    if (ret == AVCS_ERR_OK) {
        AVCODEC_LOGI("[Stop]: Stop codec success");
    } else {
        AVCODEC_LOGE("[Stop]: Stop codec failed");
    }
    ReleaseFile();
    AVCODEC_LOGI("[Stop]: Stop file success");
    CloseExtract();
    AVCODEC_LOGI("[Stop]: Stop extractor success");
    ResetBuffer();
    AVCODEC_LOGI("[Stop]: Reset buffer success");
    {
        unique_lock<mutex> lock(signal_->inMutex_);
        inBufferMap_.clear();
    }
    {
        unique_lock<mutex> lock(signal_->outMutex_);
        outBufferMap_.clear();
    }

    return ret;
}

void VDecFfmpegSample::StopInloop()
{
    if (inputLoop_ != nullptr && inputLoop_->joinable()) {
        inputLoop_->join();
        inputLoop_.reset();
    }
}

void VDecFfmpegSample::StopOutloop()
{
    if (outputLoop_ != nullptr && outputLoop_->joinable()) {
        outputLoop_->join();
        AVCODEC_LOGI("[StopOutloop]: outputLoop_ join");
        outputLoop_.reset();
        AVCODEC_LOGI("[StopOutloop]: outputLoop_ reset");
    }
}

int32_t VDecFfmpegSample::SetParameter(Format format)
{
    return vdec_->SetParameter(format);
}
