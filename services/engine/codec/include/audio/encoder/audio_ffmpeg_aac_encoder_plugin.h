#ifndef AUDIO_FFMPEG_AAC_ENCODER_PLUGIN_H
#define AUDIO_FFMPEG_AAC_ENCODER_PLUGIN_H

#include "audio_ffmpeg_base_codec.h"
#include "audio_ffmpeg_encoder_plugin.h"
#include "avcodec_audio_codec_key.h"

namespace OHOS {
namespace Media {

class AudioFFMpegAacEncoderPlugin : public IAudioFFMpegBaseCodec::CodecRegister<AudioFFMpegAacEncoderPlugin> {
public:
    AudioFFMpegAacEncoderPlugin();
    ~AudioFFMpegAacEncoderPlugin() override;

    int32_t init(const Format &format) override;
    int32_t processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer) override;
    int32_t processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer) override;
    int32_t reset() override;
    int32_t release() override;
    int32_t flush() override;
    uint32_t getInputBufferSize() const override;
    uint32_t getOutputBufferSize() const override;
    Format GetFormat() const noexcept override;

    const static std::string identify()
    {
        return std::string(AVCodecAudioCodecKey::AUDIO_ENCODER_AAC_NAME_KEY);
    }

private:
    bool CheckFormat(const Format &format) const;
    std::unique_ptr<AudioFfmpegEncoderPlugin> basePlugin;
};

} // namespace Media
} // namespace OHOS
#endif