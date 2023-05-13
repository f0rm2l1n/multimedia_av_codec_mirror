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

#ifndef CODEC_UTILS_H
#define CODEC_UTILS_H

#include "surface/surface.h"
#include "avcodec_errors.h"
#include "surface_memory.h"
#include "avsharedmemorybase.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
};

namespace OHOS { namespace Media { namespace Codec {
enum struct VideoFormat : uint8_t {
    UNKNOWN = 0,
    H264 = 1,
    MPEG4 = 2,
};

enum struct VideoPixelFormat : uint32_t {
    UNKNOWN,
    YUV420P,   ///< planar YUV 4:2:0, 1 Cr & Cb sample per 2x2 Y samples
    NV12,      ///< semi-planar YUV 4:2:0, UVUV...
    NV21,      ///< semi-planar YUV 4:2:0, VUVU...
    RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
  
};

struct ScalePara {
    int32_t srcWidth {0};
    int32_t srcHeight {0};
    AVPixelFormat srcFfFmt {AVPixelFormat::AV_PIX_FMT_NONE};
    int32_t dstWidth {0};
    int32_t dstHeight {0};
    AVPixelFormat dstFfFmt {AVPixelFormat::AV_PIX_FMT_RGBA};
    int32_t align {16};
};

struct Scale {
public:
    int32_t Init(const ScalePara& scalePara, uint8_t** dstData, int32_t* dstLineSize);
    int32_t Convert(uint8_t** srcData, const int32_t* srcLineSize, uint8_t** dstData, int32_t* dstLineSize);
private:
    ScalePara scalePara_ {};
    std::shared_ptr<SwsContext> swsCtx_ {nullptr};
};

PixelFormat TranslateSurfaceFormat(const VideoPixelFormat& surfaceFormat);
VideoPixelFormat ConvertPixelFormatFromFFmpeg(int32_t ffmpegPixelFormat);
AVPixelFormat ConvertPixelFormatToFFmpeg(VideoPixelFormat pixelFormat);
int32_t ConvertVideoFrame(std::shared_ptr<Scale> scale, std::shared_ptr<AVFrame> frame, uint8_t **dstData,
                       int32_t *dstLineSize, AVPixelFormat dstPixFmt);
int32_t WriteRgbDataStride(const std::shared_ptr<SurfaceMemory> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                          VideoPixelFormat pixFmt, int32_t stride, int32_t height);
int32_t WriteYuvData(const std::shared_ptr<AVSharedMemoryBase> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                    VideoPixelFormat pixFmt, int32_t height, int32_t width);
int32_t WriteRgbData(const std::shared_ptr<SurfaceMemory> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                    VideoPixelFormat pixFmt, int32_t height, int32_t width);


std::string AVStrError(int errnum);
bool IsYuvFormat(AVPixelFormat format);
bool IsRgbFormat(AVPixelFormat format);

}}} // namespace OHOS::Media::Codec
#endif