#ifndef I_SOURCE_SERVICE_H
#define I_SOURCE_SERVICE_H

#include <string>
#include <memory>
#include "format.h"

namespace OHOS {
namespace Media {
class ISourceService {
public:
    virtual ~ISourceService() = default;

    // 业务
    virtual int32_t Init(const std::string &uri) = 0;
    virtual int32_t GetTrackCount(uint32_t &trackCount) = 0;
    virtual int32_t SetTrackFormat(const Format &format, uint32_t trackIndex) = 0;
    virtual int32_t GetTrackFormat(Format &format, uint32_t trackIndex) = 0;
    virtual int32_t GetSourceFormat(Format &format) = 0;
    virtual uint64_t GetSourceAddr() = 0;
};
} // namespace Media
} // namespace OHOS
#endif // I_SOURCE_SERVICE_H
