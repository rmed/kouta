#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <kouta/base/callback.hpp>
#include <kouta/base/component.hpp>

namespace kouta::tests::base
{
    using namespace kouta::base;

    /// @brief Dummy component for tests.
    class DummyComponent : public Component
    {
    public:
        DummyComponent() = delete;

        explicit DummyComponent(Component* parent);

        DummyComponent(
            Component* parent,
            const Callback<std::uint16_t>& callback_a,
            const Callback<std::int32_t, const std::string&>& callback_b,
            const Callback<const std::vector<std::uint8_t>&>& callback_c);

        DummyComponent(
            Component* parent,
            const Callback<std::uint16_t>& callback_a,
            const Callback<std::int32_t, const std::string&>& callback_b,
            const Callback<const std::vector<std::uint8_t>&>& callback_c,
            const Callback<std::thread::id>& callback_d);

        DummyComponent(Component* parent, const Callback<Component*> callback_on_delete);

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
        Callback<std::uint16_t> m_callback_a;
        Callback<std::int32_t, const std::string&> m_callback_b;
        Callback<const std::vector<std::uint8_t>&> m_callback_c;
        Callback<std::thread::id> m_callback_d;
        std::optional<Callback<Component*>> m_callback_on_delete;
    };
}  // namespace kouta::tests::base
