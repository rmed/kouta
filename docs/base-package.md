# Base package

The base package provides the basic building blocks for an asynchronous/event-based application and, more often than not, will be used by the rest of the packages.

The backbone is the event loop provided by `kouta::base::asio`, which aliases either `asio::` or `boost::asio` depending on whether the `KOUTA_STANDALONE_ASIO` option is set in CMake.. Kouta builds on top of that to provide a **tree-like** architecture with the following elements.

## Component

Implemented in `kouta::base::Component`.

This is the **base type for any asynchronous element that requires access to the event loop**. A `Component` **does not own the I/O context**, and instead is expected to have a **parent** from which it can reference said context (hence the aforementioned tree-like architecture).

Apart from providing access to the event loop, the **parent** also **keeps track of its children**. This allows for components to be allocated in the **heap** via the `new` keyword if desired so that, when a `Component` is deleted, it will automatically attempt to `delete` its children and remove itself from its parent.

In the case of objects allocated in the **stack**, each object will be deleted in reverse creation order, which should prevent any issues with calling `delete` on non-heap objects due to objects removing themselves from the parent's children list.

**Note**: take extra care when mixing stack and heap allocations, as this may cause invalid deletions.

These components can be chained as many times as needed, as long as there is one parent at the root of the tree which provides the I/O context (see `kouta::base::Component::context()`).

Note that all components deriving from a single parent will be executed **in the same thread**.

Method calls can be **posted to the event loop** via the `kouta::base::Component::post()` method. This is the core of the event-based architecture, as it allows to defer method execution in a single thread in an efficient manner. In addition, it is also possible to **post calls to lambdas and free functions**.

A simple component could be:

```cpp
#include <iostream>
#include <kouta/base/component.hpp>

class MyComponent : public kouta::base::Component
{
public:
    explicit MyComponent(kouta::base::Component* parent)
        : kouta::base::Component{parent}
    {}

    void print_message(int value)
    {
        std::cout << "Received message " << value << std::endl;
    }
};

// ...
// MyComponent comp{...};
// ...

// Deferred method call
comp.post(&MyComponent::print_message, 42);
comp.post(&MyComponent::print_message, 546);
comp.post([]() {
    std::cout << "This is deferred" << std::endl;
});
```

## Root

Implemented in `kouta::base::Root`.

As opposed to the base `Component` explained above, the `Root` **owns the I/O context** and is supposed to serve as the **parent** for other components that want to share the same event loop.

While a **parent** can be provided to the root, it is only used when dealing with heap-allocated objects in order to guarantee that the `Root` is deleted with its parent.

Note that starting the event loop of a `Root` (`kouta::base::Root::run()`) **blocks the current thread** and will only be unblocked after the event loop is terminated (e.g. via the `kouta::base::Root::stop()` method).

```cpp
#include <iostream>
#include <kouta/base/root.hpp>

class MyComponent : public kouta::base::Component
{
public:
    explicit MyComponent(kouta::base::Component* parent)
        : kouta::base::Component{parent}
    {}

    void print_message(int value)
    {
        std::cout << "Received message " << value << std::endl;
    }
};

class MyRoot : public kouta::base::Root
{
public:
    MyRoot()
        : kouta::base::Root{}
        // This is the parent for the other components
        , m_comp_1{this}
        , m_comp_2{this}
    {}

    void run() override
    {
        // Enqueue some events before running
        m_comp_1.post(&MyComponent::print_message, 42);
        m_comp_2.post(&MyComponent::print_message, 253);

        kouta::base::Root::run();
    }

private:
    MyComponent m_comp_1;
    MyComponent m_comp_2;
};

// ...

MyRoot root{};
// Will block until explicitly stopped!
root.run();
```

## Branch

Implemented in `kouta::base::Branch`.

The *problem* with the `Root` type is that there is a one-to-one relation between a `Root` and a thread, considering that the event loop must block in order to handle events. Even though this is probably fine in most cases, there might be situations where another "asynchronous tree" needs to be introduced in the application, for instance to handle communications with an I/O device with high throughput.

The `Branch` type was introduced for such cases. It **is a `Root`** (owns the I/O context), but manages an `std::thread` internally. As opposed to the `Root`, starting the event loop via `kouta::base::Branch::run()` **will not block**, and instead start the event loop in the child thread.

It is supposed to wrap a `Component`, allowing external components to post events to the wrapped `Component`, as well as to the `Branch` itself. In addition, the `Branch` assumes that the wrapped `Component` expects a pointer to a parent as its first argument (which will be set to the `Branch` itself).

The **parent** provided to the `Branch` is only used when dealing with heap-allocated objects in order to guarantee that the `Root` is deleted with its parent.


```cpp
#include <iostream>
#include <kouta/base/branch.hpp>

class MyComponent : public kouta::base::Component
{
public:
    explicit MyComponent(kouta::base::Component* parent)
        : kouta::base::Component{parent}
    {}

    void print_message(int value)
    {
        std::cout << "Received message " << value << std::endl;
    }
};

class MyRoot : public kouta::base::Root
{
public:
    MyRoot()
        : kouta::base::Root{}
        // This is the parent for the other components
        , m_comp_1{this}
        // The constructor of the branch passes arguments to the wrapped component-
        //
        // The parent must be the branch itself.
        , m_comp_2{&m_comp_2}
    {}

    void run() override
    {
        // Enqueue some events before running
        m_comp_1.post(&MyComponent::print_message, 42);
        m_comp_2.post(&MyComponent::print_message, 253);

        // Start the branch
        m_comp_2.run();

        kouta::base::Root::run();
    }

private:
    MyComponent m_comp_1;
    // Runs in a separate thread
    kouta::base::Branch<MyComponent> m_comp_2;
};

// ...

MyRoot root{};
// Will block until explicitly stopped!
root.run();
```

## Callbacks

Implemented in `kouta::base::callback`.

Callbacks are wrappers around `std::function` that allow safely registering a method to call.

There are different types of callbacks:

- **Base callback** (`kouta::base::callback::BaseCallback`, aliased as `kouta::base::Callback`): Empty callback type which is supposed to be used as an implementation-agnostic callback. If called as-is, whill throw an exception, helping identify non-defined callbacks.
- **Direct callback** (`kouta::base::callback::DirectCallback`): Can be seen as a direct method call **within the same thread**. Can be used with any type of object.
- **Deferred callback** (`kouta::base::callback::DeferredCallback`): Posts a method/functor call to the event loop of a `Component`, effectively running it in that component's thread.
- **Callback list** (`kouta::base::callback::CallbackList`): Container for any of the aforementioned callbacks. When invoked, will in turn invoke the contained callbacks, regardless of their type, allowing for a one-to-many model.

**Callbacks cannot return anything**.

```cpp
#include <iostream>
#include <kouta/base/component.hpp>
#include <kouta/base/callback.hpp>

class MyComponent : public kouta::base::Component
{
public:
    explicit MyComponent(kouta::base::Component* parent)
        : kouta::base::Component{parent}
    {}

    void print_message(int value)
    {
        std::cout << "Received message " << value << std::endl;
    }
};

// ...
// MyComponent comp{...};
// ...

// Serves as placeholder, will throw when called, but can be assigned another callback type
kouta::base::Callback<int> base{};

// Direct invocation within same thread
kouta::base::callback::DirectCallback<int> direct{&comp, &MyComponent::print_message};
direct(42);

// Deferred invocation in the event loop of the component
kouta::base::callback::DeferredCallback<int> deferred{&comp, &MyComponent::print_message};
deferred(43);
kouta::base::callback::DeferredCallback<int> deferred2{&comp, [](int value) { std::cout << "Lambda message " << value << std::endl; }}

// List of different callback types
kouta::base::callback::CallbackList<int> cb_list{
    kouta::base::callback::DirectCallback{&comp, &Mycomponent::print_message},
    kouta::base::callback::DeferredCallback{&comp, &Mycomponent::print_message}
};
cb_list(44);
```

## Timer

Implemented in `kouta::base::Timer`.

The `Timer` allows performing actions after a certain amount of time. Its duration can be modified at any point, and can also be stopped before it elapses (for instance to cancel a timeout in a request).

Once the `Timer` elapses, it **is within the context of its parent component**, so it is up to the developer to make sure that proper synchronization occurs if calling something outside of that context.

```cpp
#include <iostream>
#include <kouta/base/timer.hpp>

class MyComponent : public kouta::base::Component
{
public:
    explicit MyComponent(kouta::base::Component* parent)
        : kouta::base::Component{parent}
        , m_timer{this, std::chrono::milliseconds{200}, std::bind_front(this, &MyComponent::handle_timeout)}
    {
        m_timer.start();
    }

    void handle_timeout(Timer& timer)
    {
        std::cout << "Timer expired " << std::endl;

        // Modify duration and restart
        timer.set_duration(std::chrono::milliseconds{500});
        timer.start();
    }

private:
    kouta::base::Timer m_timer;
};
```
