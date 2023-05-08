/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FFMPEG_MUXER_PLUGIN_H
#define FFMPEG_MUXER_PLUGIN_H

#include "muxer_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#include "libavutil/opt.h"
#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace Media {
namespace Plugin {
namespace Ffmpeg {
class FFmpegMuxerPlugin : public MuxerPlugin {
public:
    explicit FFmpegMuxerPlugin(std::string name, int32_t fd);
    ~FFmpegMuxerPlugin() override;

    Status SetLocation(float latitude, float longitude) override;
    Status SetRotation(int32_t rotation) override;
    Status AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc) override;
    Status Start() override;
    Status WriteSampleBuffer(uint8_t *sampleBuffer, const TrackSampleInfo &info) override;
    Status Stop() override;

private:
    Status SetCodecParameterOfTrack(AVStream *stream, const MediaDescription &trackDesc);
    static int32_t IoRead(void *opaque, uint8_t *buf, int bufSize);
    static int32_t IoWrite(void *opaque, uint8_t *buf, int bufSize);
    static int64_t IoSeek(void *opaque, int64_t offset, int whence);
    static AVIOContext *InitAvIoCtx(int32_t fd, int writeFlags);
    static void DeInitAvIoCtx(AVIOContext *ptr);
    static int32_t IoOpen(AVFormatContext *s, AVIOContext **pb, const char *url, int flags, AVDictionary **options);
    static void IoClose(AVFormatContext *s, AVIOContext *pb);
    void CloseFd();

private:
    struct IOContext {
        int32_t fd_ {-1};
        int64_t pos_ {0};
        int64_t end_ {0};
    };
    int32_t fd_ {-1};
    std::shared_ptr<AVPacket> cachePacket_ {};
    std::shared_ptr<AVOutputFormat> outputFormat_ {};
    std::shared_ptr<AVFormatContext> formatContext_ {};
    int32_t rotation_ { 0 };
};
} // Ffmpeg
} // Plugin
} // Media
} // OHOS
#endif // FFMPEG_MUXER_PLUGIN_H
