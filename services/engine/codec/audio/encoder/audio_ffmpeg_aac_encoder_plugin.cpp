#include "audio_ffmpeg_aac_encoder_plugin.h"
#include "avcodec_errors.h"
#include "avcodec_dfx.h"
#include "avcodec_log.h"
#include "media_description.h"
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegAacEncoderPlugin"};
}

namespace OHOS {
namespace Media {

AudioFFMpegAacEncoderPlugin::AudioFFMpegAacEncoderPlugin() : basePlugin(std::make_unique<AudioFfmpegEncoderPlugin>()) {}

AudioFFMpegAacEncoderPlugin::~AudioFFMpegAacEncoderPlugin() {
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

static int32_t GetAdtsHeader(std::string &adtsHeader, uint32_t &headerSize, std::shared_ptr<AVCodecContext> ctx,
                             int aacLength)
{
    uint8_t freqIdx = 0;    //0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
    switch (ctx->sample_rate) {
    case 96000: freqIdx = 0; break;
    case 88200: freqIdx = 1; break;
    case 64000: freqIdx = 2; break;
    case 48000: freqIdx = 3; break;
    case 44100: freqIdx = 4; break;
    case 32000: freqIdx = 5; break;
    case 24000: freqIdx = 6; break;
    case 22050: freqIdx = 7; break;
    case 16000: freqIdx = 8; break;
    case 12000: freqIdx = 9; break;
    case 11025: freqIdx = 10; break;
    case 8000: freqIdx = 11; break;
    case 7350: freqIdx = 12; break;
    default: freqIdx = 4; break;
    }
    uint8_t chanCfg = ctx->channels;
    uint32_t frameLength = aacLength + 7;
    adtsHeader += 0xFF;
    adtsHeader += 0xF1;
    adtsHeader += ((ctx->profile) << 6) + (freqIdx << 2) + (chanCfg >> 2);
    adtsHeader += (((chanCfg & 3) << 6) + (frameLength >> 11));
    adtsHeader += ((frameLength & 0x7FF) >> 3);
    adtsHeader += (((frameLength & 7) << 5) + 0x1F);
    adtsHeader += 0xFC;
    headerSize = 7;

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

static bool CheckSampleFormat(const std::shared_ptr<AVCodec> &codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat* p = codec->sample_fmts;
    while (*p != AV_SAMPLE_FMT_NONE) { // 通过AV_SAMPLE_FMT_NONE作为结束符
        if (*p == sample_fmt)
            return true;
        p++;
    }
    return false;
}

static bool CheckSampleRate(const std::shared_ptr<AVCodec> &codec, const int sample_rate)
{
    const int* p = codec->supported_samplerates;
    while (*p != 0) {// 0作为退出条件，比如libfdk-aacenc.c的aac_sample_rates
        if (*p == sample_rate)
            return true;
        p++;
    }
    return false;
}

static bool CheckChannelLayout(const std::shared_ptr<AVCodec> &codec, const uint64_t channel_layout)
{
    // 不是每个codec都给出支持的channel_layout
    const uint64_t* p = codec->channel_layouts;
    if (!p) {
        AVCODEC_LOGI("The encoder %{public}s do not set channel_layouts", codec->name);
        return true;
    }
    while (*p != 0) { // 0作为退出条件，比如libfdk-aacenc.c的aac_channel_layout
        if (*p == channel_layout)
            return true;
        p++;
    }
    return false;
}

bool AudioFFMpegAacEncoderPlugin::CheckFormat(const Format &format) const
{
    if (!format.ContainKey(MediaDescriptionKey::MD_KEY_SAMPLE_FORMAT) ||
        !format.ContainKey(MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT) ||
        !format.ContainKey(MediaDescriptionKey::MD_KEY_SAMPLE_RATE)) {
        AVCODEC_LOGE("Format parameter missing");
        return false;
    }

    auto avCodec = basePlugin->GetAVCodec();
    int sampleFormat;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_FORMAT, sampleFormat);
    if (!CheckSampleFormat(avCodec, (AVSampleFormat)sampleFormat)) {
        AVCODEC_LOGE("Sample format not supported");
        return false;
    }

    int sampleRate;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, sampleRate);
    if (!CheckSampleRate(avCodec, sampleRate)) {
        AVCODEC_LOGE("Sample rate not supported");
        return false;
    }

    int64_t channelLayout;
    format.GetLongValue(MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT, channelLayout);
    if (!CheckChannelLayout(avCodec, channelLayout)) {
        AVCODEC_LOGE("Channel layout not supported");
        return false;
    }

    return true;
}

int32_t AudioFFMpegAacEncoderPlugin::init(const Format &format) {
    int32_t ret = basePlugin->AllocateContext("aac");
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("Allocat aac context failed, ret = %{publid}d", ret);
        return ret;
    }
    if (!CheckFormat(format)) {
        AVCODEC_LOGE("Format check failed.");
        return AVCodecServiceErrCode::AVCS_ERR_UNSUPPORT_AUD_PARAMS;
    }
    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("Init context failed, ret = %{publid}d", ret);
        return ret;
    }
    ret = basePlugin->OpenContext();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("Open context failed, ret = %{publid}d", ret);
        return ret;
    }

    ret = basePlugin->InitFrame();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("Init frame failed, ret = %{publid}d", ret);
        return ret;
    }

    basePlugin->RegisterHeaderFunc(GetAdtsHeader);

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegAacEncoderPlugin::processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer) {
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegAacEncoderPlugin::processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer) {
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegAacEncoderPlugin::reset() {
    return basePlugin->Reset();
}

int32_t AudioFFMpegAacEncoderPlugin::release() {
    return basePlugin->Release();
}

int32_t AudioFFMpegAacEncoderPlugin::flush() {
    return basePlugin->Flush();
}

uint32_t AudioFFMpegAacEncoderPlugin::getInputBufferSize() const {
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > 8192) {
        maxSize = 4 * 1024 * 8;
    }
    return maxSize;
}

uint32_t AudioFFMpegAacEncoderPlugin::getOutputBufferSize() const {
    return 8192;
}

Format AudioFFMpegAacEncoderPlugin::GetFormat() const noexcept {
    return basePlugin->GetFormat();
}

} // namespace Media
} // namespace OHOS