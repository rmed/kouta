#pragma once

#include <bitset>
#include <initializer_list>
#include <type_traits>

namespace kouta::utils
{
    /// @brief Custom bitset implementation that allows using enumeration values as indices.
    ///
    /// @details
    /// The used enumeration type used must:
    ///
    /// - Derive from std::size_t
    /// - Not set any value for the labels (optionally, value 0 can be set for the first one)
    /// - Contain a `_Total` label at the end
    ///
    /// The `_Total` label is used to determine the number of values in the enumeration (by explicitly casting it to an
    /// std::size_t value).
    ///
    /// @example
    /// ```c++
    /// enum class MyEnum : std::size_t
    /// {
    ///     A,
    ///     B,
    ///     C,
    ///
    ///     _Total
    /// };
    ///
    /// EnumSet<MyEnum> set{MyEnum::A, MyEnum::C};
    ///
    /// // Test and set using enumeration values
    /// set.test(MyEnum::A);
    /// set.set{MyEnum::B};
    ///
    /// // Or raw indices
    /// set.test(0);
    /// set.set(1);
    /// ```
    ///
    /// @tparam TEnum               Enumeration type to use.
    template<class TEnum>
        requires std::is_enum_v<TEnum>
    class EnumSet : public std::bitset<static_cast<std::size_t>(TEnum::_Total)>
    {
    public:
        /// Enumeration type used.
        using EnumType = TEnum;

        /// Base biset type.
        using BaseType = std::bitset<static_cast<std::size_t>(TEnum::_Total)>;

        using reference = typename BaseType::reference;

        // Re-export some methods
        using BaseType::BaseType;
        using BaseType::operator[];
        using BaseType::set;
        using BaseType::test;

        /// @brief Constructor from a set of values.
        ///
        /// @param[in] values           Values to set.
        EnumSet(std::initializer_list<TEnum> values)
            : BaseType{}
        {
            for (auto v : values)
            {
                set(v);
            }
        }

        /// @brief Access a specific bit.
        ///
        /// @note Does not perform any bound checking.
        ///
        /// @param[in] pos          Position to check.
        ///
        /// @return Value of the bit.
        bool operator[](EnumType pos) const
        {
            return BaseType::operator[](static_cast<std::size_t>(pos));
        }

        /// @brief Access a specific bit.
        ///
        /// @note Does not perform any bound checking.
        ///
        /// @param[in] pos          Position to check.
        ///
        /// @return Reference to the bit.
        reference operator[](EnumType pos)
        {
            return BaseType::operator[](static_cast<std::size_t>(pos));
        }

        /// @brief Access a specific bit.
        ///
        /// @param[in] pos          Position to check.
        ///
        /// @return Value of the bit.
        ///
        /// @throws std::out_of_range if pos does not correspond to a valid position within the bitset.
        bool test(EnumType pos) const
        {
            return test(static_cast<std::size_t>(pos));
        }

        /// @brief Set the value of a specific bit.
        ///
        /// @param[in] pos          Position to set.
        /// @param[in] value        Value to set.
        ///
        /// @return Reference to this object for chaining
        ///
        /// @throws std::out_of_range if pos does not correspond to a valid position within the bitset.
        EnumSet& set(EnumType pos, bool value = true)
        {
            set(static_cast<std::size_t>(pos), value);

            return *this;
        }
    };
}  // namespace kouta::utils
