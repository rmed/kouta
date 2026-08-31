#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "kouta/callback/abstract-callback.hpp"
#include "kouta/async/component.hpp"

namespace kouta::tests::async
{
    using namespace kouta::async;

    /// @brief Dummy component for tests.
    class DummyComponent : public Component
    {
    public:
        DummyComponent() = delete;

        explicit DummyComponent(Component* parent);

        DummyComponent(
            Component* parent,
            const callback::AbstractCallback<std::uint16_t>& callback_a,
            const callback::AbstractCallback<std::int32_t, const std::string&>& callback_b,
            const callback::AbstractCallback<const std::vector<std::uint8_t>&>& callback_c);

        DummyComponent(
            Component* parent,
            const callback::AbstractCallback<std::uint16_t>& callback_a,
            const callback::AbstractCallback<std::int32_t, const std::string&>& callback_b,
            const callback::AbstractCallback<const std::vector<std::uint8_t>&>& callback_c,
            const callback::AbstractCallback<std::thread::id>& callback_d);

        DummyComponent(Component* parent, const callback::AbstractCallback<Component*> callback_on_delete);

        // Not copyable
        DummyComponent(const DummyComponent&) = delete;
        DummyComponent& operator=(const DummyComponent&) = delete;

        // Not movable
        DummyComponent(DummyComponent&&) = delete;
        DummyComponent& operator=(DummyComponent&&) = delete;

        ~DummyComponent() override
        {
            // Notify that the object was deleted
            if (m_callback_on_delete)
            {
                m_callback_on_delete.value()(this);
            }
        }

        /// @brief Callback invokers.
        /// @{
        void call_a(std::uint16_t value);
        void call_b(std::int32_t value_a, const std::string& value_b);
        void call_c(const std::vector<std::uint8_t>& value);
        void call_d();
        /// @}

    private:
        callback::AbstractCallback<std::uint16_t> m_callback_a;
        callback::AbstractCallback<std::int32_t, const std::string&> m_callback_b;
        callback::AbstractCallback<const std::vector<std::uint8_t>&> m_callback_c;
        callback::AbstractCallback<std::thread::id> m_callback_d;
        std::optional<callback::AbstractCallback<Component*>> m_callback_on_delete;
    };
}  // namespace kouta::tests::async
