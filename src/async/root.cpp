#include "kouta/async/root.hpp"

namespace kouta::async
{
    Root::Root()
        : Root{nullptr}
    {
    }

    Root::Root(Component* parent)
        : Component{parent}
        , m_context{}
    {
    }

    Context& Root::context()
    {
        return m_context;
    }

    void Root::run()
    {
        // This is blocking!
        m_context.run();
    }

    void Root::stop()
    {
        m_context.stop();
    }
}  // namespace kouta::async
