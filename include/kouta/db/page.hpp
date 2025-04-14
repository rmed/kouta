#pragma once

#include <cstdint>
#include <vector>

namespace kouta::db
{
    /// @brief Database pagination result POD.
    ///
    /// @details
    /// This structure acts as a container for paginated results of queries by providing information about the current
    /// page, total number of pages, and number of elements per page that may be useful for the caller to request
    /// subsequent pages from the database.
    ///
    /// @tparam TData               Type of the data returned by the query.
    template<class TData>
    class Page
    {
    public:
        /// @brief Data type contained in the `items` attribute.
        using DataType = TData;

        /// @brief Container for items.
        using ContainerType = std::vector<TData>;

        /// @brief Default constructor
        Page() = default;

        /// @brief Constructor.
        ///
        /// @param[in] page         Current page index.
        /// @param[in] pages        Total number of pages according to the query.
        /// @param[in] total_items  Total number of items according to the query.
        /// @param[in] items        Items corresponding to the current page.
        Page(std::size_t page, std::size_t pages, std::size_t total_items, ContainerType&& items)
            : m_page{page}
            , m_pages{pages}
            , m_total_items{total_items}
            , m_items{items}
        {
        }

        // Copyable
        Page(const Page&) = default;
        Page& operator=(const Page&) = default;

        // Movable
        Page(Page&&) = default;
        Page& operator=(Page&&) = default;

        virtual ~Page() = default;

        /// @brief Obtain the index of the page represented by this class.
        ///
        /// @note This is zero-indexed.
        std::size_t page() const
        {
            return m_page;
        }

        /// @brief Obtain the total number of pages for the related query.
        std::size_t pages() const
        {
            return m_pages;
        }

        /// @brief Obtain the total number of items returned by the related query.
        std::size_t total_items() const
        {
            return m_total_items;
        }

        /// @brief Obtain a reference to the contained items.
        /// @{
        const ContainerType& items() const
        {
            return m_items;
        }

        ContainerType& items()
        {
            return m_items;
        }
        /// @}

        /// @brief Check whether there is a page before this one.
        bool has_prev_page() const
        {
            return (m_page > 0 && m_pages > 0);
        }

        /// @brief Check whether there is a page after this one.
        bool has_next_page() const
        {
            return (m_page > 0 && (m_page + 1) < m_pages);
        }

        /// @brief Obtain the previous page (if any).
        ///
        /// @returns Previous page index if available, otherwise returns current page.
        std::size_t prev_page() const
        {
            return has_prev_page() ? (m_page - 1) : m_page;
        }

        /// @brief Obtain the next page (if any).
        ///
        /// @returns Next page index if available, otherwise returns current page.
        std::size_t next_page() const
        {
            return has_next_page() ? (m_page + 1) : m_page;
        }

        /// @brief Set the page index.
        ///
        /// @param[in] page         Page index (zero-indexed).
        void set_page(std::size_t page)
        {
            m_page = page;
        }

        /// @brief Set the total number of pages for the related query.
        ///
        /// @param[in] pages        Total number of pages.
        void set_pages(std::size_t pages)
        {
            m_pages = pages;
        }

        /// @brief Set the total number of items for the related query.
        ///
        /// @param[in] total        Total number of items, as returned by a COUNT query.
        void set_total_items(std::size_t total)
        {
            m_total_items = total;
        }

    private:
        /// @brief Current page.
        std::size_t m_page;

        /// @brief Total number of pages.
        std::size_t m_pages;

        /// @brief Total number of items (in all pages).
        std::size_t m_total_items;

        /// @brief Items in the current page.
        std::vector<TData> m_items;
    };
}  // namespace kouta::db
