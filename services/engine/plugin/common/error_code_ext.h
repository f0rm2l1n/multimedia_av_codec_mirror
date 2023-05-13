#include <map>
#include "plugin_definition.h"
#include "avcodec_errors.h"

namespace OHOS {
namespace Media {
int32_t TranslatePluginStatus(Plugin::Status error)
{
    const static std::map<Plugin::Status, int32_t> g_transTable = {
        {Plugin::Status::END_OF_STREAM, AVCodecServiceErrCode::AVCS_ERR_OK},
        {Plugin::Status::OK, AVCodecServiceErrCode::AVCS_ERR_OK},
        {Plugin::Status::NO_ERROR, AVCodecServiceErrCode::AVCS_ERR_OK},
        {Plugin::Status::ERROR_UNKNOWN, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_PLUGIN_ALREADY_EXISTS, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_INCOMPATIBLE_VERSION, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_NO_MEMORY, AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY},
        {Plugin::Status::ERROR_WRONG_STATE, AVCodecServiceErrCode::AVCS_ERR_INVALID_OPERATION},
        {Plugin::Status::ERROR_UNIMPLEMENTED, AVCodecServiceErrCode::AVCS_ERR_UNSUPPORT},
        {Plugin::Status::ERROR_INVALID_PARAMETER, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL},
        {Plugin::Status::ERROR_INVALID_DATA, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL},
        {Plugin::Status::ERROR_MISMATCHED_TYPE, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL},
        {Plugin::Status::ERROR_TIMED_OUT, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_UNSUPPORTED_FORMAT, AVCodecServiceErrCode::AVCS_ERR_UNSUPPORT_CONTAINER_TYPE},
        {Plugin::Status::ERROR_NOT_ENOUGH_DATA, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_NOT_EXISTED, AVCodecServiceErrCode::AVCS_ERR_OPEN_FILE_FAILED},
        {Plugin::Status::ERROR_AGAIN, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_PERMISSION_DENIED, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_NULL_POINTER, AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL},
        {Plugin::Status::ERROR_INVALID_OPERATION, AVCodecServiceErrCode::AVCS_ERR_INVALID_OPERATION},
        {Plugin::Status::ERROR_CLIENT, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_SERVER, AVCodecServiceErrCode::AVCS_ERR_UNKNOWN},
        {Plugin::Status::ERROR_DELAY_READY, AVCodecServiceErrCode::AVCS_ERR_OK},
    };
    auto ite = g_transTable.find(error);
    if (ite == g_transTable.end()) {
        return AVCS_ERR_UNKNOWN;
    }
    return ite->second;
}
} // namespace Media
} // namespace OHOS