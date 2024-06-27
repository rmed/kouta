#include "dummy-component.hpp"

namespace kouta::tests::base
{
    DummyComponent::DummyComponent(
        Component* parent,
        const Callback<std::uint16_t>& callback_a,
        const Callback<std::int32_t, const std::string&>& callback_b,
        const Callback<const std::vector<std::uint8_t>&>& callback_c)
        : Component{parent}
        , m_callback_a{callback_a}
        , m_callback_b{callback_b}
        , m_callback_c{callback_c}
    {
    }

    DummyComponent::DummyComponent(
        Component* parent,
        const Callback<std::uint16_t>& callback_a,
        const Callback<std::int32_t, const std::string&>& callback_b,
        const Callback<const std::vector<std::uint8_t>&>& callback_c,
        const Callback<std::thread::id>& callback_d)
        : Component{parent}
        , m_callback_a{callback_a}
        , m_callback_b{callback_b}
        , m_callback_c{callback_c}
        , m_callback_d{callback_d}
    {
    }

    void DummyComponent::call_a(std::uint16_t value)
    {
        m_callback_a(value);
    }

    void DummyComponent::call_b(std::int32_t value_a, const std::string& value_b)
    {
        m_callback_b(value_a, value_b);
    }

    void DummyComponent::call_c(const std::vector<std::uint8_t>& value)
    {
        m_callback_c(value);
    }

    void DummyComponent::call_d()
    {
        // Provide the ID of the thread this component is running in
        m_callback_d(std::this_thread::get_id());
    }
}  // namespace kouta::tests::base
