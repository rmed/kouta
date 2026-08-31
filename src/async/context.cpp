#include "detail/context_p.hpp"

#include <asio/post.hpp>

namespace kouta::async
{
    Context::Context()
        : m_impl{std::make_unique<Context::Impl>()}
    {
    }

    // Destructor must be specified here due to the incomplete type of the Impl struct.
    Context::~Context() = default;

    void Context::run()
    {
        // The event loop runs forever
        auto work_guard{asio::make_work_guard(m_impl->asio_context)};
        m_impl->asio_context.run();
    }

    void Context::stop()
    {
        m_impl->asio_context.stop();
    }

    void Context::post(const PostableHandler& handler)
    {
        asio::post(m_impl->asio_context.get_executor(), handler);
    }

    void Context::post(PostableHandler&& handler)
    {
        asio::post(m_impl->asio_context.get_executor(), handler);
    }
}  // namespace kouta::async
