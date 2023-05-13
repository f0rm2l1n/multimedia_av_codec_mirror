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

#include "codec_utils.h"
#include "avcodec_log.h"

namespace OHOS { namespace Media { namespace Codec {

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "FCodec"};
std::map<VideoPixelFormat, AVPixelFormat> g_pixelFormatMap = {
    {VideoPixelFormat::YUV420P, AV_PIX_FMT_YUV420P},
    {VideoPixelFormat::NV12, AV_PIX_FMT_NV12},
    {VideoPixelFormat::NV21, AV_PIX_FMT_NV21},
    {VideoPixelFormat::RGBA, AV_PIX_FMT_RGBA},
    {VideoPixelFormat::BGRA, AV_PIX_FMT_BGRA},
};
}

int32_t ConvertVideoFrame(std::shared_ptr<Scale> scale, std::shared_ptr<AVFrame> frame, uint8_t **dstData,
                                  int32_t *dstLineSize, AVPixelFormat dstPixFmt)
{
    if (scale == nullptr) {
        scale = std::make_shared<Scale>();
        ScalePara scalePara{static_cast<int32_t>(frame->width),
                            static_cast<int32_t>(frame->height),
                            static_cast<AVPixelFormat>(frame->format),
                            static_cast<int32_t>(frame->width),
                            static_cast<int32_t>(frame->height),
                            dstPixFmt};
        CHECK_AND_RETURN_RET_LOG(scale->Init(scalePara, dstData, dstLineSize) == AVCS_ERR_OK, AVCS_ERR_UNKNOWN,
                                 "Scale init error");
    }
    return scale->Convert(frame->data, frame->linesize, dstData, dstLineSize);
}


int32_t WriteRgbDataStride(const std::shared_ptr<SurfaceMemory> &frameBuffer, uint8_t **scaleData,
                                     int32_t *scaleLineSize, VideoPixelFormat pixFmt, int32_t stride, int32_t height)
{
    if (pixFmt == VideoPixelFormat::RGBA || pixFmt == VideoPixelFormat::BGRA) {
        size_t srcPos = 0;
        size_t dstPos = 0;
        int32_t writeSize = scaleLineSize[0];
        for (uint32_t colNum = 0; colNum < height; colNum++) {
            frameBuffer->Write(scaleData[0] + srcPos, writeSize, dstPos);
            dstPos += stride;
        }
    } else {
        return AVCS_ERR_UNSUPPORT;
    }
    AVCODEC_LOGD("WriteRgbDataStride success");
    return AVCS_ERR_OK;
}

int32_t WriteYuvData(const std::shared_ptr<AVSharedMemoryBase> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                               VideoPixelFormat pixFmt, int32_t height, int32_t width)
{
    size_t ySize = static_cast<size_t>(scaleLineSize[0] * height);      // yuv420: 411 nv21
    size_t uvSize = static_cast<size_t>(scaleLineSize[1] * height / 2); // 2
    size_t frameSize = 0;
    if (pixFmt == VideoPixelFormat::YUV420P) {
        frameSize = ySize + (uvSize * 2); // 2
    } else if (pixFmt == VideoPixelFormat::NV21 || pixFmt == VideoPixelFormat::NV12) {
        frameSize = ySize + uvSize;
    }
    CHECK_AND_RETURN_RET_LOG(frameBuffer->GetSize() >= frameSize, AVCS_ERR_NO_MEMORY,
                             "output buffer size is not enough: real[%{public}zu], need[%{public}zu]",
                             frameBuffer->GetSize(), frameSize);
    if (pixFmt == VideoPixelFormat::YUV420P) {
        frameBuffer->Write(scaleData[0], ySize);
        frameBuffer->Write(scaleData[1], uvSize);
        frameBuffer->Write(scaleData[2], uvSize); // 2
    } else if ((pixFmt == VideoPixelFormat::NV12) || (pixFmt == VideoPixelFormat::NV21)) {
        frameBuffer->Write(scaleData[0], ySize);
        frameBuffer->Write(scaleData[1], uvSize);
    } else {
        return AVCS_ERR_UNSUPPORT;
    }
    AVCODEC_LOGD("WriteYuvData success");
    return AVCS_ERR_OK;
}

int32_t WriteRgbData(const std::shared_ptr<SurfaceMemory> &frameBuffer, uint8_t **scaleData, int32_t *scaleLineSize,
                               VideoPixelFormat pixFmt, int32_t height, int32_t width)
{
    uint32_t stride = frameBuffer->GetSurfaceBufferStride();
    if (stride % width) {
        return WriteRgbDataStride(frameBuffer, scaleData, scaleLineSize, pixFmt, stride, height);
    }
    

    size_t frameSize = static_cast<size_t>(scaleLineSize[0] * height);
    CHECK_AND_RETURN_RET_LOG(frameBuffer->GetSize() >= frameSize, AVCS_ERR_NO_MEMORY,
                             "output buffer size is not enough: real[%{public}zu], need[%{public}zu]",
                             frameBuffer->GetSize(), frameSize);
    if (pixFmt == VideoPixelFormat::RGBA || pixFmt == VideoPixelFormat::BGRA) {
        frameBuffer->Write(scaleData[0], frameSize);
    } else {
        return AVCS_ERR_UNSUPPORT;
    }
    AVCODEC_LOGD("WriteRgbData success");
    return AVCS_ERR_OK;
}

std::string AVStrError(int errnum)
{
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(errnum, errbuf, AV_ERROR_MAX_STRING_SIZE);
    return std::string(errbuf);
}

PixelFormat TranslateSurfaceFormat(const VideoPixelFormat& surfaceFormat)
{
    switch (surfaceFormat) {
        case VideoPixelFormat::YUV420P: {
            return PixelFormat::PIXEL_FMT_YCBCR_420_P;
        }
        case VideoPixelFormat::RGBA: {
            return PixelFormat::PIXEL_FMT_RGBA_8888;
        }
        case VideoPixelFormat::BGRA: {
            return PixelFormat::PIXEL_FMT_BGRA_8888;
        }
        case VideoPixelFormat::NV12: {
            return PixelFormat::PIXEL_FMT_YCBCR_420_SP;
        }
        case VideoPixelFormat::NV21: {
            return PixelFormat::PIXEL_FMT_YCRCB_420_SP;
        }
        default:
            return PixelFormat::PIXEL_FMT_BUTT;
    }
}

VideoPixelFormat ConvertPixelFormatFromFFmpeg(int32_t ffmpegPixelFormat)
{
    auto iter = std::find_if(g_pixelFormatMap.begin(), g_pixelFormatMap.end(),
        [&] (const std::pair<VideoPixelFormat, AVPixelFormat>& tmp) -> bool {
        return tmp.second == ffmpegPixelFormat;
    });
    return iter == g_pixelFormatMap.end() ? VideoPixelFormat::UNKNOWN : iter->first;
}

AVPixelFormat ConvertPixelFormatToFFmpeg(VideoPixelFormat pixelFormat)
{
    auto iter = std::find_if(g_pixelFormatMap.begin(), g_pixelFormatMap.end(),
        [&] (const std::pair<VideoPixelFormat, AVPixelFormat>& tmp) -> bool {
        return tmp.first == pixelFormat;
    });
    return iter == g_pixelFormatMap.end() ? AV_PIX_FMT_NONE : iter->second;
}

bool IsYuvFormat(AVPixelFormat format)
{
    return (format == AV_PIX_FMT_YUV420P || format == AV_PIX_FMT_NV12 || format == AV_PIX_FMT_NV21);
}

bool IsRgbFormat(AVPixelFormat format)
{
    return (format == AV_PIX_FMT_RGBA || format == AV_PIX_FMT_BGRA);
}

// #if defined(VIDEO_SUPPORT)
int32_t Scale::Init(const ScalePara& scalePara, uint8_t** dstData, int32_t* dstLineSize)
{
    scalePara_ = scalePara;
    if (swsCtx_ != nullptr) {
        return AVCS_ERR_OK;
    }
    auto swsContext = sws_getContext(scalePara_.srcWidth, scalePara_.srcHeight, scalePara_.srcFfFmt,
        scalePara_.dstWidth, scalePara_.dstHeight, scalePara_.dstFfFmt,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    if(swsContext == nullptr){
        return AVCS_ERR_UNKNOWN;
    }
    swsCtx_ = std::shared_ptr<SwsContext>(swsContext, [](struct SwsContext *ptr) {
        if (ptr != nullptr) {
            sws_freeContext(ptr);
        }
    });
    auto ret = av_image_alloc(dstData, dstLineSize, scalePara_.dstWidth, scalePara_.dstHeight,
        scalePara_.dstFfFmt, scalePara_.align);
    if(ret < 0){
        return AVCS_ERR_UNKNOWN;
    }
    for (int32_t i = 0; dstLineSize[i] > 0; i++) {
        if (dstData[i] && !dstLineSize[i]) {
            return AVCS_ERR_UNKNOWN;
        }
    }
    return AVCS_ERR_OK;
}

int32_t Scale::Convert(uint8_t** srcData, const int32_t* srcLineSize, uint8_t** dstData, int32_t* dstLineSize)
{
    auto res = sws_scale(swsCtx_.get(), srcData, srcLineSize, 0, scalePara_.srcHeight, dstData, dstLineSize);
    if(res < 0){
        return AVCS_ERR_UNKNOWN;
    }
    return AVCS_ERR_OK;
}

}}} // namespace OHOS::Media::Codec