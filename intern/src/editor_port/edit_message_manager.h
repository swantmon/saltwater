
#pragma once

#include "editor_port/edit_message_type.h"

#include <functional>

namespace Edit
{
    class CMessage;
} // namespace Edit

namespace Edit
{
    typedef std::function<void(CMessage& _rMessage)> CMessageDelegate;
} // namespace Edit

#define EDIT_RECEIVE_MESSAGE(_Method) std::bind(_Method, this, std::placeholders::_1)

namespace Edit
{
namespace MessageManager
{
    void Register(SGUIMessageType::Enum _Type, const CMessageDelegate& _rDelegate);
    void Register(SApplicationMessageType::Enum _Type, const CMessageDelegate& _rDelegate);

    int SendMessage(SGUIMessageType::Enum _Type, const CMessage& _rMessage);
    int SendMessage(SApplicationMessageType::Enum _Type, const CMessage& _rMessage);
} // namespace MessageManager
} // namespace Edit
