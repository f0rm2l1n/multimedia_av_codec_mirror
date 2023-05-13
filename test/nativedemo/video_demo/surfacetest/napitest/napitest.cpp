/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "napitest.h"
#include "surface_utils.h"
#include "surface.h"
#include "videodec_ffmpeg_demo.h"
#include <thread>

using namespace OHOS;
using namespace OHOS::Media;

namespace {
    const std::string MIME_TYPE = "video/avc";
}

namespace napitest {
std::string g_surfaceId;
sptr<Surface> surface = nullptr;
FILE *inFp = nullptr;
FILE *outFp = nullptr;

bool setSurfaceID(std::string& surfaceID, AVFileDescriptor& inJsFp, int32_t& outFd, int32_t& width, int32_t& height, uint32_t& outErrCode, std::string& out)
{
    int32_t inFd = inJsFp.fd;
    int64_t inOffset = inJsFp.offset;
    int64_t inLen = inJsFp.length;

    AVCODEC_LOGI("[setSurfaceId], surfaceId: %{public}s, inFd: %{public}d, inOffset: %{public}lld, inSize: %{public}lld, outFd: %{public}d", surfaceID.c_str(), inFd, inOffset,inLen, outFd);
    
    if (surfaceID.empty() || surfaceID[0] < '0' || surfaceID[0] > '9') {
        AVCODEC_LOGE("[setSurfaceId], surfaceID is invalid");
        out = "surfaceID is invalid";
        return false;
    }

    
    if(inFd < 0 || inOffset < 0 || inLen < 0) {
        AVCODEC_LOGE("[setSurfaceId], inJsFp is invalid");
        out = "inJsFp is invalid";
        return false;
    }

    if(outFd < 0) {
        AVCODEC_LOGW("[setSurfaceId], outFd is invalid");
        out = "outFd is invalid";
        return false;
    }

    inFp=fdopen(inFd,"rb");
    if(inFp==nullptr){
        AVCODEC_LOGW("[setSurfaceId], inFp is invalid");
        out = "inFp is invalid";
        return false;
    }
    fseek(inFp, inOffset, SEEK_SET);

    outFp=fdopen(outFd,"wb");
    if(outFp==nullptr){
        AVCODEC_LOGW("[setSurfaceId], outFp is invalid");
        out = "outFp is invalid";
        return false;
    }

    uint64_t sId = std::stoull(surfaceID);
    AVCODEC_LOGI("[setSurfaceId], surfaceId =  %{public}lld", sId);

    surface = SurfaceUtils::GetInstance()->GetSurface(sId);
    if (surface == nullptr) {
        AVCODEC_LOGW("[setSurfaceId], surface is null");
        out = "surface is null";
        return false;
    }
    
    std::unique_ptr<Codec::VDecFfmpegSample> vdec = std::make_unique<Codec::VDecFfmpegSample>();
    if (vdec == nullptr) {
        AVCODEC_LOGW("[setSurfaceId], vdec is null");
        out = "vdec is null";
        return false;
    }
    vdec->RunVideoDec(inFp, outFp, width, height, surface);

    g_surfaceId = surfaceID;
    out = g_surfaceId + "," + std::to_string(inFd) + "," + std::to_string(outFd);

    return true;
}

bool getSurfaceID(uint32_t& outErrCode, std::string& out)
{
	out = g_surfaceId;
    AVCODEC_LOGI("[getSurfaceId], surfaceId: %{public}s", out.c_str());
    return true;
}
}
