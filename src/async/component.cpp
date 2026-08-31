#include "kouta/async/component.hpp"

namespace kouta::async
{
    Component::Component(Component* parent)
        : m_parent{parent}
    {
        if (m_parent)
        {
            m_parent->add_child(this);
        }
    }
    Component::~Component()
    {
        // Delete children
        while (!m_children.empty())
        {
            // Remove child from the set directly
            auto* component = m_children.extract(m_children.cbegin()).value();
            delete component;
        }

        // Delete from parent
        if (m_parent)
        {
            m_parent->remove_child(this);
        }
    }

    Context& Component::context()
    {
        return m_parent->context();
    }

    void Component::post(Context::PostableHandler&& callable)
    {
        context().post(std::move(callable));
    }

    void Component::add_child(Component* component)
    {
        m_children.insert(component);
    }

    void Component::remove_child(Component* component)
    {
        m_children.extract(component);
    }
}  // namespace kouta::async
