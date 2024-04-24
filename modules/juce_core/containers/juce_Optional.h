/*
  ==============================================================================

   This file is part of the YUP library.
   Copyright (c) 2024 - kunitoki@gmail.com

   YUP is an open source library subject to open-source licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   YUP IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

using Nullopt = std::nullopt_t;
constexpr auto nullopt = std::nullopt;

// Without this, our tests can emit "unreachable code" warnings during
// link time code generation.
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4702)

#ifndef DOXYGEN
#define JUCE_OPTIONAL_OPERATORS X(==) X(!=) X(<) X(<=) X(>) X(>=)
#endif

/**
    A simple optional type.

    In new code, you should probably prefer using std::optional directly.

    This is intended to stand-in for std::optional while JUCE's minimum
    supported language standard is lower than C++17. When the minimum language
    standard moves to C++17, this class will probably be deprecated, in much
    the same way that juce::ScopedPointer was deprecated in favour of
    std::unique_ptr after C++11.

    This isn't really intended to be used by JUCE clients. Instead, it's to be
    used internally in JUCE code, with an API close-enough to std::optional
    that the types can be swapped with fairly minor disruption at some point in
    the future, but *without breaking any public APIs*.

    @tags{Core}
*/
template <typename Value>
class Optional
{
    template <typename>   struct IsOptional              : std::false_type {};
    template <typename T> struct IsOptional<Optional<T>> : std::true_type  {};

public:
    Optional() = default;
    Optional (const Optional&) = default;
    Optional (Optional&&) = default;
    Optional& operator= (const Optional&) = default;
    Optional& operator= (Optional&&) = default;

    Optional (Nullopt) noexcept {}

    template <typename Head, typename... Tail, std::enable_if_t<! IsOptional<std::decay_t<Head>>::value, int> = 0>
    Optional (Head&& head, Tail&&... tail)
        noexcept (std::is_nothrow_constructible_v<std::optional<Value>, Head, Tail...>)
        : opt (std::forward<Head> (head), std::forward<Tail> (tail)...) {}

    template <typename Other>
    Optional (const Optional<Other>& other)
        noexcept (std::is_nothrow_constructible_v<std::optional<Value>, const std::optional<Other>&>)
        : opt (other.opt) {}

    template <typename Other>
    Optional (Optional<Other>&& other)
        noexcept (std::is_nothrow_constructible_v<std::optional<Value>, std::optional<Other>&&>)
        : opt (std::move (other.opt)) {}

    template <typename Other, std::enable_if_t<! IsOptional<std::decay_t<Other>>::value, int> = 0>
    Optional& operator= (Other&& other)
        noexcept (std::is_nothrow_assignable_v<std::optional<Value>, Other>)
    {
        opt = std::forward<Other> (other);
        return *this;
    }

    template <typename Other>
    Optional& operator= (const Optional<Other>& other)
        noexcept (std::is_nothrow_assignable_v<std::optional<Value>, const std::optional<Other>&>)
    {
        opt = other.opt;
        return *this;
    }

    template <typename Other>
    Optional& operator= (Optional<Other>&& other)
        noexcept (std::is_nothrow_assignable_v<std::optional<Value>, std::optional<Other>&&>)
    {
        opt = std::move (other.opt);
        return *this;
    }

    template <typename... Other>
    auto& emplace (Other&&... other)
    {
        return opt.emplace (std::forward<Other> (other)...);
    }

    void reset() noexcept
    {
        opt.reset();
    }

    void swap (Optional& other)
        noexcept (std::is_nothrow_swappable_v<std::optional<Value>>)
    {
        opt.swap (other.opt);
    }

    decltype (auto) operator->()       { return opt.operator->(); }
    decltype (auto) operator->() const { return opt.operator->(); }
    decltype (auto) operator* ()       { return opt.operator* (); }
    decltype (auto) operator* () const { return opt.operator* (); }

    explicit operator bool() const noexcept { return opt.has_value(); }
    bool hasValue() const noexcept { return opt.has_value(); }

    template <typename U>
    decltype (auto) orFallback (U&& fallback) const& { return opt.value_or (std::forward<U> (fallback)); }

    template <typename U>
    decltype (auto) orFallback (U&& fallback) & { return opt.value_or (std::forward<U> (fallback)); }

    #define X(op) \
        template <typename T, typename U> friend bool operator op (const Optional<T>&, const Optional<U>&); \
        template <typename T> friend bool operator op (const Optional<T>&, Nullopt); \
        template <typename T> friend bool operator op (Nullopt, const Optional<T>&); \
        template <typename T, typename U> friend bool operator op (const Optional<T>&, const U&); \
        template <typename T, typename U> friend bool operator op (const T&, const Optional<U>&);

    JUCE_OPTIONAL_OPERATORS

    #undef X

private:
    template <typename Other>
    friend class Optional;

    std::optional<Value> opt;
};

JUCE_END_IGNORE_WARNINGS_MSVC

template <typename Value>
Optional<std::decay_t<Value>> makeOptional (Value&& v)
{
    return std::forward<Value> (v);
}

#ifndef DOXYGEN
#define X(op) \
    template <typename T, typename U> bool operator op (const Optional<T>& lhs, const Optional<U>& rhs) { return lhs.opt op rhs.opt; } \
    template <typename T> bool operator op (const Optional<T>& lhs, Nullopt rhs) { return lhs.opt op rhs; } \
    template <typename T> bool operator op (Nullopt lhs, const Optional<T>& rhs) { return lhs op rhs.opt; } \
    template <typename T, typename U> bool operator op (const Optional<T>& lhs, const U& rhs) { return lhs.opt op rhs; } \
    template <typename T, typename U> bool operator op (const T& lhs, const Optional<U>& rhs) { return lhs op rhs.opt; }

JUCE_OPTIONAL_OPERATORS

#undef X
#undef JUCE_OPTIONAL_OPERATORS
#endif

} // namespace juce
