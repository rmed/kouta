#include <kouta/base/component.hpp>

namespace kouta::base
{
    Component::Component(Component* parent)
        : m_parent{parent}
    {
    }

    boost::asio::io_context& Component::context()
    {
        return m_parent->context();
    }
}  // namespace kouta::base
