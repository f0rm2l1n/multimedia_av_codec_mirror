#include "codeclist_demo.h"
#include "native_avmagic.h"
#include <iostream>
extern "C" {
#include <string.h>
}
namespace OHOS { namespace Media{
// const std::string MIME_TYPE = "video/avc";
// 基于能力需求查找编码器(高级使用场景,可选)
// 视频编码能力查询示例
void CodecListDemo::RunCase() {
    std::cout  << "===== ============== ======" << std::endl;
    const char *mime = "video/mpeg2";
    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetStringValue(format, "codec_mime", mime);
    const char *codecName = OH_AVCodec_FindDecoder(format);
    std::cout  << codecName << std::endl;
    std::cout  << "start getting caps" << std::endl;
    // const char *codecName_ = "avdec_h264"; avdec_mpeg4
    OH_AVCapability *cap = OH_AVCodec_CreateCapability(codecName);
    const char *mimetype = OH_AVCapability_GetMimeType(cap);
    CapabilityData capsData = cap->capabilityData_;
    std::cout  << mimetype << std::endl;
    std::cout  << capsData.maxInstance << std::endl;
    // std::cout  << capsData.pixFormat[0] << std::endl;
    std::cout  << "get caps successful" << std::endl;
    
    std::cout  << "===== ============== ======" << std::endl;
    const char *mime2 = "audio/opus";
    OH_AVFormat *format2 = OH_AVFormat_Create();
    OH_AVFormat_SetStringValue(format2, "codec_mime", mime2);
    const char *codecName2 = OH_AVCodec_FindDecoder(format2);
    std::cout  << codecName2 << std::endl;
    std::cout  << "start getting caps" << std::endl;
    // const char *codecName_ = "avdec_h264"; avdec_mpeg4
    OH_AVCapability *cap2 = OH_AVCodec_CreateCapability(codecName2);
    const char *mimetype2 = OH_AVCapability_GetMimeType(cap2);
    CapabilityData capsData2 = cap2->capabilityData_;
    std::cout  << mimetype2 << std::endl;
    std::cout  << capsData2.maxInstance << std::endl;
    // std::cout  << capsData.pixFormat[0] << std::endl;
    // std::cout  << "get caps successful" << std::endl;
    // std::cout  << "===== ============== ======" << std::endl;
    // const char *mime3 = "video/mp4v-es";
    // OH_AVFormat *format3 = OH_AVFormat_Create();
    // OH_AVFormat_SetStringValue(format3, "codec_mime", mime3);
    // const char *codecName3 = OH_AVCodec_FindDecoder(format3);
    // std::cout  << codecName3 << std::endl;
    // std::cout  << "start getting caps" << std::endl;
    // // const char *codecName_ = "avdec_h264"; avdec_mpeg4
    // OH_AVCapability *cap3 = OH_AVCodec_CreateCapability(codecName3);
    // const char *mimetype3 = OH_AVCapability_GetMimeType(cap3);
    // CapabilityData capsData3 = cap3->capabilityData_;
    // std::cout  << mimetype3 << std::endl;
    // std::cout  << capsData3.maxInstance << std::endl;
    // std::cout  << capsData.pixFormat[0] << std::endl;
    // std::cout  << "get caps successful" << std::endl;

    // std::cout  << "===== ============== ======" << std::endl;
    // const char *mime4 = "video/mpegfgsd3";
    // OH_AVFormat *format4 = OH_AVFormat_Create();
    // OH_AVFormat_SetStringValue(format4, "codec_mime", mime4);
    // const char *codecName4 = OH_AVCodec_FindEncoder(format4);
    // std::cout  << codecName4 << std::endl;
    // std::cout  << "start getting caps" << std::endl;
    // // const char *codecName_ = "avdec_h264"; avdec_mpeg4
    // OH_AVCapability *cap4 = OH_AVCodec_CreateCapability(codecName4);
    // const char *mimetype4 = OH_AVCapability_GetMimeType(cap4);
    // CapabilityData capsData4 = cap4->capabilityData_;
    // std::cout  << mimetype4 << std::endl;
    // std::cout  << capsData4.maxInstance << std::endl;
    // // std::cout  << capsData.pixFormat[0] << std::endl;
    // std::cout  << "get caps successful" << std::endl;

}
}}
