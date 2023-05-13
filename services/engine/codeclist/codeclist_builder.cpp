#include "codeclist_builder.h"
#include "fcodec.h"
// #include "audio_ffmpeg_adapter.h"

namespace OHOS {
namespace Media {
int32_t VideoCodecList::GetCapabilityList(std::vector<CapabilityData>& caps) {
    std::shared_ptr<Codec::FCodec> codec = std::make_shared<Codec::FCodec>();
    auto ret = codec->getCodecCapability(caps);
    return ret;
}

int32_t AudioCodecList::GetCapabilityList(std::vector<CapabilityData>& caps) {
    (void)caps;
    return 0;
}
} // namespace Media
} // namespace OHOS