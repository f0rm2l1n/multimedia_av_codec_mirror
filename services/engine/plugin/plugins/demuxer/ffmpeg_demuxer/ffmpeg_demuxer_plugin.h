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

#ifndef FFMPEG_DEMUXER_PLUGIN_H
#define FFMPEG_DEMUXER_PLUGIN_H

#include <iostream>
#include <cstddef>
#include <memory>
#include <vector>
#include <map>

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/dict.h"

#ifdef __cplusplus
}
#endif

#include "demuxer_plugin.h"
#include "avcodec_common.h"
#include "avsharedmemory.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace FFmpeg {
class FFmpegDemuxerPlugin : public DemuxerPlugin {
public:
    FFmpegDemuxerPlugin();
    ~FFmpegDemuxerPlugin();
    int32_t Create(uintptr_t sourceAddr) override;
    int32_t CopyNextSample(uint32_t &trackIndex, uint8_t* buffer,
                            AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag) override;
    int32_t SelectSourceTrackByID(uint32_t trackIndex) override;
    int32_t UnselectSourceTrackByID(uint32_t trackIndex) override;
    int32_t SeekToTime(int64_t mSeconds, AVSeekMode mode) override;
    std::vector<uint32_t> GetSelectedTrackIds();
private:
    bool IsInSelectedTrack(uint32_t trackIndex);
    AVCodecBufferFlag ConvertFlagsFromFFmpeg(AVPacket* pkt,  AVStream* avStream);
    int64_t GetTotalStreamFrames(int streamIndex);
    int32_t SetBitStreamFormat();
    int32_t ConvertAVPacketToSample(AVStream* avStream, uint8_t* buffer, AVCodecBufferInfo &bufferInfo,
                                    AVCodecBufferFlag &flag, AVPacket* pkt);
    void ConvertAvcOrHevcToAnnexb(AVPacket& pkt);
    void InitBitStreamContext(const AVStream& avStream);
    int64_t CalculateTimeByFrameIndex(AVStream* avStream, int keyFrameIdx);
    std::vector<uint32_t> selectedTrackIds_;
    std::shared_ptr<AVFormatContext> formatContext_;
    std::shared_ptr<AVBSFContext> avbsfContext_ {nullptr};
    std::map<uint32_t, VideoBitStreamFormat> videoBitStreamFormat_;
    std::map<uint32_t, uint64_t> sampleIndex_;
};
} // namespace FFmpeg
} // namespace Plugin
} // namespace AVCodec
} // namespace OH
#endif // FFMPEG_DEMUXER_PLUGIN_H