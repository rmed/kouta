#include <kouta/base/root.hpp>

namespace kouta::base
{
    Root::Root()
        // The root has no parent
        : Component{nullptr}
        , m_context()
    {
    }

    boost::asio::io_context& Root::context()
    {
        return m_context;
    }

    void Root::run()
    {
        // Have the event loop run forever
        auto work_guard{boost::asio::make_work_guard(m_context)};
        m_context.run();
    }

    void Root::stop()
    {
        m_context.stop();
    }
}  // namespace kouta::base
