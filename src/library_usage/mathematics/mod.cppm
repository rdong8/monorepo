/// @file
/// @brief A sample library containing math functions.
/// A documentation comment for the file like this one must be in any file you wish to be tracked by Doxygen.

module;

export module library_usage.mathematics;

import std;

import quill;

namespace math
{
/// An n-dimensional mathematical vector
export template <std::size_t N, std::floating_point Float = double> struct Vec
{
    using Self = Vec<N, Float>;        ///< Alias for the type of this vector (postfix doc comment)
    using Data = std::array<Float, N>; ///< Alias for the underlying data type

    /// The components of the vector (regular doc comment)
    Data components{};

    /// Construct a new @ref Vec with the given components.
    /// @param[in] components The components of the vector
    template <typename... Components>
    explicit constexpr Vec(Components &&...components)
        : components{std::forward<Components>(components)...}
    {
    }

    /// @name Rule of 5 special member functions
    ///@{
    /// This is a member group. Documentation comments here will be shared for all elements in the group.

    constexpr ~Vec() = default;

    constexpr Vec(Self const &other) noexcept = default;

    constexpr Vec(Self &&other) noexcept = default;

    auto constexpr operator=(Self const &other) noexcept -> Self & = default;

    auto constexpr operator=(Self &&other) noexcept -> Self & = default;

    ///@}

    /// Compute the dot product of this vector and @p other
    /// @param[in] other The second operand of the dot product operator
    /// @param[in] initial The initial value of the dot product, defaults to 0
    [[nodiscard]]
    auto constexpr dot(this Self const &self, Self const &other, Float initial = {}) noexcept -> Float
    {
        return std::inner_product(self.components.begin(), self.components.end(), other.components.begin(), initial);
    }

    /// Compute the norm of this vector
    [[nodiscard]]
    auto constexpr norm(this Self const &self) noexcept -> Float
    {
        return std::sqrt(self.dot(self));
    }

    /// Compare this vector and @p other by norm
    [[nodiscard]]
    auto constexpr operator<=>(this Self const &self, Self const &other) noexcept -> std::weak_ordering
    {
        return std::weak_order(self.norm(), other.norm());
    }

    /// Compute the scalar product of this vector and @p c
    /// @param[in] c The scalar
    [[nodiscard]]
    auto constexpr operator*(this Self const &self, Float c) noexcept -> Self
    {
        return Self{
            [&]<std::size_t... I>(std::index_sequence<I...>)
            { return std::array{c * self.components[I]...}; }(std::make_index_sequence<N>{}),
        };
    }

    /// Compute the scalar product of @p c and @p vec
    /// @param[in] c The scalar
    /// @param[in] vec The vector
    [[nodiscard]]
    friend auto constexpr operator*(Float c, Vec const &vec) noexcept -> Vec
    {
        return vec * c;
    }
};

auto constexpr DEFAULT_DX = 0.0001;

/// Evaluate the approximate derivative of @p f at @p x
/// @tparam F A function @f$ f : \mathbb R \to \mathbb R @f$
/// @tparam Float A floating point type
/// @param[in] x The value to evaluate the derivative at
///
/// Example: @f$ f(x) = x^2 @f$
/// @code
/// auto constexpr F{[] static (double x) { return x * x; }};
/// std::println("{}", d_dx<F>(3.0)); // Prints 6
/// @endcode
export template <auto F, std::floating_point Float = double, Float DX = DEFAULT_DX>
    requires requires(Float x) {
        { F(x) } -> std::same_as<Float>;
    }
[[nodiscard]]
auto constexpr d_dx(Float x) noexcept(noexcept(F(std::declval<Float>()))) -> Float
{
    return (F(x + DX) - F(x)) / DX;
}

} // namespace math

// TODO: Make Vec an input range so that this isn't necessary
/// Formatter specialization for @ref math::Vec
export template <std::size_t N, std::floating_point Float>
struct fmtquill::formatter<math::Vec<N, Float>> : fmtquill::formatter<typename math::Vec<N, Float>::Data>
{
    using Self = fmtquill::formatter<typename math::Vec<N, Float>::Data>;

    template <typename FormatContext>
    auto format(this Self const &self, math::Vec<N, Float> const &vec, FormatContext &ctx) -> FormatContext::iterator
    {
        return self.format(vec.components, ctx);
    }
};

/// Codec specialization for @ref math::Vec
export template <std::size_t N, std::floating_point Float>
struct quill::Codec<math::Vec<N, Float>> : quill::DeferredFormatCodec<math::Vec<N, Float>>
{
};
