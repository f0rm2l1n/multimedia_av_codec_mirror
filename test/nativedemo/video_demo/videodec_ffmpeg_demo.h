#ifndef VIDEODEC_FFMPEG_DEMO_H
#define VIDEODEC_FFMPEG_DEMO_H
#include <iostream>
#include <stdio.h>
#include <mutex>
#include <thread>
#include <queue>
#include <unistd.h>
#include <atomic>
#include <fstream>
#include <string>
#include "fcodec.h"
#include "avcodec_log.h"

extern "C" {
#include <string.h>
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavcodec/avcodec.h"
}

namespace OHOS { namespace Media {namespace Codec {

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

class VDecFfmpegSample : public NoCopyable {
public:
    VDecFfmpegSample() = default;
    ~VDecFfmpegSample();
    void RunVideoDec(FILE *inFp, FILE *outFp, int32_t width, int32_t height, sptr<Surface> surface=nullptr, std::string codeName="");

private:
    int64_t GetSystemTimeUs();
    int32_t CreateVideoDecoder(std::string codeName);
    int32_t SetVideoDecoderCallback();
    int32_t ConfigureVideoDecoder();
    int32_t SetSurface(Surface *surface);
    int32_t Start();
    int32_t Stop();
    int32_t Flush();
    int32_t Reset();
    void Release();
    int32_t SetParameter(Format format);
    void InputFunc();
    void OutputSurfaceFunc();
    void OutputBufferFunc();
    void ReleaseSignal();
    bool IsRender();
    int32_t ExtractFrame();
    int32_t CreateExtract();
    void CloseExtract();
    void StopInloop();
    void StopOutloop();
    void ReleaseFile();
    void ResetBuffer();
    int32_t FlushStart();
    int32_t GetoutputformatVideoDecoder();
    int32_t SetparameterVideoDecoder();
    void BasicTest1();
    void BasicTest2();
    void BasicTest3();
    void BasicTest4();

    std::atomic<bool> isRunning_{false};
    FILE *inFile_{nullptr};
    FILE *dumpFd_{nullptr};
    std::unique_ptr<std::thread> inputLoop_{nullptr};
    std::unique_ptr<std::thread> outputLoop_{nullptr};
    std::unordered_map<uint32_t, std::shared_ptr<AVSharedMemoryBase>> inBufferMap_;
    std::unordered_map<uint32_t, std::shared_ptr<AVSharedMemoryBase>> outBufferMap_;
    std::shared_ptr<CodecBase> vdec_;
    VDecSignal *signal_;
    std::shared_ptr<AVCodecCallback> cb_;
    int64_t timeStamp_{0};
    uint32_t frameCount_{0};
    int64_t lastRenderedTimeUs_{0};
    // Extract Frame：
    const AVCodec *codec_{nullptr};
    AVCodecParserContext *parser_{nullptr};
    AVCodecContext *codec_ctx_{nullptr};
    AVPacket *pkt_{nullptr};
    size_t data_size_{0};
    uint8_t *data_{nullptr};
    uint8_t inbuf_[10240 + AV_INPUT_BUFFER_PADDING_SIZE];
    int32_t file_end_{0};
    int32_t file_num_read_{0};
    int32_t file_num_write_{0};
    bool restart_{false};
    int32_t width_;
    int32_t height_;
    sptr<Surface> surface_{nullptr};
};
}} }    // namespace OHOS::Media
#endif // VIDEODEC_NDK_SAMPLE_H
