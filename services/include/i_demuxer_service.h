#ifndef I_DEMUXER_SERVICE_H
#define I_DEMUXER_SERVICE_H

#include <string>
// #include <vector>
#include <memory>
#include "avcodec_common.h"
// #include "avsharedmemory.h"
// #include "avcontainer_common.h"
// #include "media_description.h"

namespace OHOS {
namespace Media {
class IDemuxerService {
public:
    virtual ~IDemuxerService() = default;

    // 业务
    virtual int32_t Init(uintptr_t sourceAddr) = 0;
    virtual int32_t SelectSourceTrackByID(uint32_t index) = 0;
    virtual int32_t UnselectSourceTrackByID(uint32_t index) = 0;
    virtual int32_t CopyNextSample(uint32_t &trackIndex, uint8_t *buffer, AVCodecBufferInfo &bufferInfo,AVCodecBufferFlag &flag) = 0;
    virtual int32_t SeekToTime(int64_t mSeconds, const AVSeekMode mode) = 0;
};
} // namespace Media
} // namespace OHOS
#endif // I_DEMUXER_SERVICE_H