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

#include "type_converter.h"
#include "hcodec_log.h"

namespace OHOS::MediaAVCodec {
using namespace std;
using namespace OHOS::HDI::Codec::V1_0;

OMX_VIDEO_CODINGTYPE TypeConverter::HdiRoleToOmxCodingType(AvCodecRole role)
{
    static const map<AvCodecRole, OMX_VIDEO_CODINGTYPE> table = {
        { MEDIA_ROLETYPE_VIDEO_AVC, OMX_VIDEO_CodingAVC },
        { MEDIA_ROLETYPE_VIDEO_HEVC, static_cast<OMX_VIDEO_CODINGTYPE>(CODEC_OMX_VIDEO_CodingHEVC) },
    };
    auto it = table.find(role);
    if (it == table.end()) {
        LOGW("unknown AvCodecRole %{public}d", role);
        return OMX_VIDEO_CodingMax;
    }
    return it->second;
}

std::optional<OmxCodingType> TypeConverter::MimeToOmxCodingType(const std::string &mime)
{
    static const map<std::string, OmxCodingType> table {
        {
            string(CodecMimeType::VIDEO_AVC),
            {
                OMX_VIDEO_CodingAVC,
                MEDIA_ROLETYPE_VIDEO_AVC
            },
        },
        {
            string(CodecMimeType::VIDEO_HEVC),
            {
                static_cast<OMX_VIDEO_CODINGTYPE>(CODEC_OMX_VIDEO_CodingHEVC),
                MEDIA_ROLETYPE_VIDEO_HEVC
            },
        },
    };
    auto it = table.find(mime);
    if (it == table.end()) {
        LOGW("unknown mime %{public}s", mime.c_str());
        return std::nullopt;
    }
    return it->second;
}

std::optional<GraphicPixelFormat> TypeConverter::InnerFmtToDisplayFmt(VideoPixelFormat format)
{
    static const map<VideoPixelFormat, GraphicPixelFormat> table = {
        { YUVI420, GRAPHIC_PIXEL_FMT_YCBCR_420_P },
        { NV12, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
        { NV21, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
        { RGBA, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    };
    auto it = table.find(format);
    if (it == table.end()) {
        LOGW("unknown VideoPixelFormat %{public}d", format);
        return std::nullopt;
    }
    return it->second;
}

std::optional<GraphicPixelFormat> TypeConverter::OmxFmtToDisplayFmt(OMX_COLOR_FORMATTYPE format)
{
    static const std::map<OMX_COLOR_FORMATTYPE, GraphicPixelFormat> table {
        {OMX_COLOR_FormatYUV420Planar,     GRAPHIC_PIXEL_FMT_YCBCR_420_P},
        {OMX_COLOR_FormatYUV420SemiPlanar, GRAPHIC_PIXEL_FMT_YCBCR_420_SP},
    };
    auto it = table.find(format);
    if (it == table.end()) {
        LOGW("unknown OMX_COLOR_FORMATTYPE %{public}d", format);
        return std::nullopt;
    }
    return it->second;
}

std::optional<VideoPixelFormat> TypeConverter::OmxFmtToInnerFmt(OMX_COLOR_FORMATTYPE format)
{
    static const map<OMX_COLOR_FORMATTYPE, VideoPixelFormat> table = {
        { OMX_COLOR_FormatYUV420Planar,     YUVI420 },
        { OMX_COLOR_FormatYUV420SemiPlanar, NV12 },
    };
    auto it = table.find(format);
    if (it == table.end()) {
        LOGW("unknown OMX_COLOR_FORMATTYPE %{public}d", format);
        return std::nullopt;
    }
    return it->second;
}

std::optional<GraphicTransformType> TypeConverter::InnerRotateToDisplayRotate(VideoRotation rotate)
{
    static const map<VideoRotation, GraphicTransformType> table = {
        { VIDEO_ROTATION_0, GRAPHIC_ROTATE_NONE },
        { VIDEO_ROTATION_90, GRAPHIC_ROTATE_90 },
        { VIDEO_ROTATION_180, GRAPHIC_ROTATE_180 },
        { VIDEO_ROTATION_270, GRAPHIC_ROTATE_270 },
    };
    auto it = table.find(rotate);
    if (it == table.end()) {
        LOGW("unknown VideoRotation %{public}u", rotate);
        return std::nullopt;
    }
    return it->second;
}

Primaries TypeConverter::InnerPrimaryToOmxPrimary(ColorPrimary primary)
{
    static const map<ColorPrimary, Primaries> table = {
        { COLOR_PRIMARY_BT709,        PRIMARIES_BT709 },
        { COLOR_PRIMARY_UNSPECIFIED,  PRIMARIES_UNSPECIFIED },
        { COLOR_PRIMARY_BT470_M,      PRIMARIES_BT470_6M },
        { COLOR_PRIMARY_BT601_625,    PRIMARIES_BT601_625 },
        { COLOR_PRIMARY_BT601_525,    PRIMARIES_BT601_525 },
        { COLOR_PRIMARY_SMPTE_ST240,  PRIMARIES_BT601_525 },
        { COLOR_PRIMARY_GENERIC_FILM, PRIMARIES_GENERICFILM },
        { COLOR_PRIMARY_BT2020,       PRIMARIES_BT2020 },
        { COLOR_PRIMARY_SMPTE_ST428,  PRIMARIES_MAX },
        { COLOR_PRIMARY_P3DCI,        PRIMARIES_MAX },
        { COLOR_PRIMARY_P3D65,        PRIMARIES_MAX },
    };
    auto it = table.find(primary);
    if (it == table.end()) {
        LOGW("unknown ColorPrimary %{public}d, use unspecified instead", primary);
        return PRIMARIES_UNSPECIFIED;
    }
    return it->second;
}

Transfer TypeConverter::InnerTransferToOmxTransfer(TransferCharacteristic transfer)
{
    static const map<TransferCharacteristic, Transfer> table = {
        { TRANSFER_CHARACTERISTIC_BT709,           TRANSFER_SMPTE170 },
        { TRANSFER_CHARACTERISTIC_UNSPECIFIED,     TRANSFER_UNSPECIFIED },
        { TRANSFER_CHARACTERISTIC_GAMMA_2_2,       TRANSFER_GAMMA22 },
        { TRANSFER_CHARACTERISTIC_GAMMA_2_8,       TRANSFER_GAMMA28 },
        { TRANSFER_CHARACTERISTIC_BT601,           TRANSFER_SMPTE170 },
        { TRANSFER_CHARACTERISTIC_SMPTE_ST240,     TRANSFER_SMPTE240 },
        { TRANSFER_CHARACTERISTIC_LINEAR,          TRANSFER_LINEAR },
        { TRANSFER_CHARACTERISTIC_LOG,             TRANSFER_MAX },
        { TRANSFER_CHARACTERISTIC_LOG_SQRT,        TRANSFER_MAX },
        { TRANSFER_CHARACTERISTIC_IEC_61966_2_4,   TRANSFER_XVYCC },
        { TRANSFER_CHARACTERISTIC_BT1361,          TRANSFER_BT1361 },
        { TRANSFER_CHARACTERISTIC_IEC_61966_2_1,   TRANSFER_MAX },
        { TRANSFER_CHARACTERISTIC_BT2020_10BIT,    TRANSFER_SMPTE170 },
        { TRANSFER_CHARACTERISTIC_BT2020_12BIT,    TRANSFER_SMPTE170 },
        { TRANSFER_CHARACTERISTIC_PQ,              TRANSFER_PQ },
        { TRANSFER_CHARACTERISTIC_SMPTE_ST428,     TRANSFER_ST428 },
        { TRANSFER_CHARACTERISTIC_HLG,             TRANSFER_HLG },
    };
    auto it = table.find(transfer);
    if (it == table.end()) {
        LOGW("unknown TransferCharacteristic %{public}d, use unspecified instead", transfer);
        return TRANSFER_UNSPECIFIED;
    }
    return it->second;
}

MatrixCoeffs TypeConverter::InnerMatrixToOmxMatrix(MatrixCoefficient matrix)
{
    static const map<MatrixCoefficient, MatrixCoeffs> table = {
        { MATRIX_COEFFICIENT_IDENTITY,          MATRIX_MAX },
        { MATRIX_COEFFICIENT_BT709,             MATRIX_BT709 },
        { MATRIX_COEFFICIENT_UNSPECIFIED,       MATRIX_UNSPECIFED },
        { MATRIX_COEFFICIENT_FCC,               MATRIX_FCC },
        { MATRIX_COEFFICIENT_BT601_625,         MATRIX_BT601 },
        { MATRIX_COEFFICIENT_BT601_525,         MATRIX_BT601 },
        { MATRIX_COEFFICIENT_SMPTE_ST240,       MATRIX_SMPTE240 },
        { MATRIX_COEFFICIENT_YCGCO,             MATRIX_MAX },
        { MATRIX_COEFFICIENT_BT2020_NCL,        MATRIX_BT2020 },
        { MATRIX_COEFFICIENT_BT2020_CL,         MATRIX_BT2020CONSTANT },
        { MATRIX_COEFFICIENT_SMPTE_ST2085,      MATRIX_MAX },
        { MATRIX_COEFFICIENT_CHROMATICITY_NCL,  MATRIX_BT2020 },
        { MATRIX_COEFFICIENT_CHROMATICITY_CL,   MATRIX_BT2020CONSTANT },
        { MATRIX_COEFFICIENT_ICTCP,             MATRIX_MAX },
    };
    auto it = table.find(matrix);
    if (it == table.end()) {
        LOGW("unknown MatrixCoefficient %{public}d, use unspecified instead", matrix);
        return MATRIX_UNSPECIFED;
    }
    return it->second;
}

std::optional<OMX_VIDEO_AVCPROFILETYPE> TypeConverter::AvcProfileToOmxAvcProfile(int32_t avcProfile)
{
    static const map<int32_t, OMX_VIDEO_AVCPROFILETYPE> avcProfileConverterList = {
        { AVC_PROFILE_BASELINE,             OMX_VIDEO_AVCProfileBaseline },
        { AVC_PROFILE_EXTENDED,             OMX_VIDEO_AVCProfileExtended },
        { AVC_PROFILE_HIGH,                 OMX_VIDEO_AVCProfileHigh },
        { AVC_PROFILE_HIGH_10,              OMX_VIDEO_AVCProfileHigh10 },
        { AVC_PROFILE_HIGH_422,             OMX_VIDEO_AVCProfileHigh422 },
        { AVC_PROFILE_HIGH_444,             OMX_VIDEO_AVCProfileHigh444 },
        { AVC_PROFILE_MAIN,                 OMX_VIDEO_AVCProfileMain }
    };
    auto it = avcProfileConverterList.find(avcProfile);
    if (it == avcProfileConverterList.end()) {
        LOGW("unknown AVC Profile(0x%{public}x)", avcProfile);
        return std::nullopt;
    }

    return it->second;
}

std::optional<int32_t> TypeConverter::HdiAvcProfileToAvcProfile(Profile hdiAvcProfile)
{
    static const map<Profile, int32_t> avcProfileConverterList = {
        { AVC_BASELINE_PROFILE, AVC_PROFILE_BASELINE },
        { AVC_HIGH_PROFILE,     AVC_PROFILE_HIGH },
        { AVC_MAIN_PROFILE,     AVC_PROFILE_MAIN }
    };
    auto it = avcProfileConverterList.find(hdiAvcProfile);
    if (it == avcProfileConverterList.end()) {
        LOGW("unknown hdi AVC Profile(0x%{public}x)", hdiAvcProfile);
        return std::nullopt;
    }

    return it->second;
}

std::optional<CodecHevcProfile> TypeConverter::HevcProfileToOmxHevcProfile(int32_t hevcProfile)
{
    static const map<int32_t, CodecHevcProfile> hevcProfileConverterList = {
        { HEVC_PROFILE_MAIN,           CODEC_HEVC_PROFILE_MAIN },
        { HEVC_PROFILE_MAIN_10,        CODEC_HEVC_PROFILE_MAIN10 },
        { HEVC_PROFILE_MAIN_STILL,     CODEC_HEVC_PROFILE_MAIN_STILL },
        { HEVC_PROFILE_MAIN_10_HDR10,  CODEC_HEVC_PROFILE_MAIN10_HDR10 },
    };
    auto it = hevcProfileConverterList.find(hevcProfile);
    if (it == hevcProfileConverterList.end()) {
        LOGW("unknown HEVC Profile(0x%{public}x)", hevcProfile);
        return std::nullopt;
    }

    return it->second;
}

std::optional<int32_t> TypeConverter::HdiHevcProfileToHevcProfile(Profile hdiHevcProfile)
{
    static const map<Profile, int32_t> hevcProfileConverterList = {
        { HEVC_MAIN_PROFILE,    HEVC_PROFILE_MAIN },
        { HEVC_MAIN_10_PROFILE, HEVC_PROFILE_MAIN_10 },
    };
    auto it = hevcProfileConverterList.find(hdiHevcProfile);
    if (it == hevcProfileConverterList.end()) {
        LOGW("unknown hdi HEVC Profile(0x%{public}x)", hdiHevcProfile);
        return std::nullopt;
    }

    return it->second;
}

}