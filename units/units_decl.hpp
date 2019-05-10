/*
Copyright � 2019,
Lawrence Livermore National Security, LLC;
See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <ctgmath>
#include <functional>

namespace units
{
namespace detail
{
    /** Class representing base unit data
    @details the seven SI base units https://en.m.wikipedia.org/wiki/SI_base_unit
    + currency, count, and radians, with flags for per_unit and temperature
    */
    class unit_data
    {
      public:
        // construct from powers
        constexpr unit_data(int meter,
                            int kilogram,
                            int second,
                            int ampere,
                            int kelvin,
                            int mole,
                            int candela,
                            int currency,
                            int count,
                            int radians,
                            unsigned int per_unit,
                            unsigned int flag,
                            unsigned int flag2,
                            unsigned int equation)
            : meter_(meter), second_(second), kilogram_(kilogram), ampere_(ampere), candela_(candela),
              kelvin_(kelvin), mole_(mole), radians_(radians), currency_(currency), count_(count),
              per_unit_(per_unit), flag_(flag), e_flag_(flag2), equation_(equation)
        {
        }
        /** Construct with the error flag triggered*/
        explicit constexpr unit_data(std::nullptr_t)
            : meter_(0), second_(0), kilogram_(0), ampere_(0), candela_(0), kelvin_(0), mole_(0), radians_(0),
              currency_(0), count_(0), per_unit_(0), flag_(1), e_flag_(1), equation_(0)
        {
        }

        // compose data -- saturating multiply equivalent operator
        constexpr unit_data operator+(unit_data other) const
        {
            return {
              meter_ + other.meter_,
              kilogram_ + other.kilogram_,
              second_ + other.second_,
              ampere_ + other.ampere_,
              kelvin_ + other.kelvin_,
              mole_ + other.mole_,
              candela_ + other.candela_,
              currency_ + other.currency_,
              count_ + other.count_,
              radians_ + other.radians_,
              (per_unit_ != 0 || other.per_unit_ != 0) ? 1u : 0,
              (flag_ != 0 || other.flag_ != 0) ? 1u : 0,
              (e_flag_ != 0 || other.e_flag_ != 0) ? 1u : 0,
              (equation_ != 0 || other.equation_ != 0) ? 1u : 0,
            };
        }
        /// Division equivalent operator
        constexpr unit_data operator-(unit_data other) const
        {
            return {meter_ - other.meter_,
                    kilogram_ - other.kilogram_,
                    second_ - other.second_,
                    ampere_ - other.ampere_,
                    kelvin_ - other.kelvin_,
                    mole_ - other.mole_,
                    candela_ - other.candela_,
                    currency_ - other.currency_,
                    count_ - other.count_,
                    radians_ - other.radians_,
                    (per_unit_ != 0 || other.per_unit_ != 0) ? 1u : 0,
                    (flag_ != 0 || other.flag_ != 0) ? 1u : 0,
                    (e_flag_ != 0 || other.e_flag_ != 0) ? 1u : 0,
                    (equation_ != 0 || other.equation_ != 0) ? 1u : 0};
        }
        constexpr unit_data inv() const
        {
            return {-meter_,    -kilogram_, -second_,  -ampere_,  -kelvin_, -mole_,  -candela_,
                    -currency_, -count_,    -radians_, per_unit_, flag_,    e_flag_, equation_};
        }
        /// take a unit_data to some power
        constexpr unit_data pow(int power) const
        {  // the +e_flag_ on seconds is to handle a few weird operations that generate a square_root hz operation,
           // the e_flag allows some recovery of that unit and handling of that peculiar situation
            return {meter_ * power,
                    kilogram_ * power,
                    second_ * power -
                      ((e_flag_ > 0u && second_ != 0) ? ((second_ < 0) ? (power >> 1) : -(power >> 1)) : 0),
                    ampere_ * power,
                    kelvin_ * power,
                    mole_ * power,
                    candela_ * power,
                    currency_ * power,
                    count_ * power,
                    radians_ * power,
                    per_unit_,
                    0,  // zero out e_flag
                    0,  // zero out flag
                    equation_};
        }
        constexpr unit_data root(int power) const
        {
            return (hasValidRoot(power)) ?
                     unit_data(meter_ / power, kilogram_ / power, second_ / power, ampere_ / power,
                               kelvin_ / power, 0, 0, 0, 0, radians_ / power, per_unit_, 0, e_flag_, 0) :
                     unit_data(nullptr);
        }
        // comparison operators
        constexpr bool operator==(unit_data other) const
        {
            return equivalent_non_counting(other) && mole_ == other.mole_ && count_ == other.count_ &&
                   radians_ == other.radians_ && per_unit_ == other.per_unit_ && flag_ == other.flag_ &&
                   e_flag_ == other.e_flag_ && equation_ == other.equation_;
        }
        constexpr bool operator!=(unit_data other) const { return !(*this == other); }

        // support for specific unitConversion calls
        constexpr bool is_per_unit() const { return per_unit_ != 0; }
        constexpr bool is_temperature() const
        {
            return (flag_ == 1 && kelvin_ == 1 && meter_ == 0 && second_ == 0 && kilogram_ == 0 && ampere_ == 0 &&
                    candela_ == 0 && mole_ == 0 && radians_ == 0 && currency_ == 0 && count_ == 0 &&
                    equation_ == 0);
        }
        constexpr bool is_flag() const { return (flag_ != 0); }
        constexpr bool has_e_flag() const { return e_flag_ != 0; }
        constexpr bool is_equation() const { return equation_ != 0; }
        /// Check if the unit bases are the same
        constexpr bool has_same_base(unit_data other) const
        {
            return equivalent_non_counting(other) && mole_ == other.mole_ && count_ == other.count_ &&
                   radians_ == other.radians_;
        }
        // Check equivalence for non-counting base units
        constexpr bool equivalent_non_counting(unit_data other) const
        {
            return meter_ == other.meter_ && second_ == other.second_ && kilogram_ == other.kilogram_ &&
                   ampere_ == other.ampere_ && candela_ == other.candela_ && kelvin_ == other.kelvin_ &&
                   currency_ == other.currency_;
        }
        // Check if the unit is empty
        constexpr bool empty() const
        {
            return meter_ == 0 && second_ == 0 && kilogram_ == 0 && ampere_ == 0 && candela_ == 0 &&
                   kelvin_ == 0 && mole_ == 0 && radians_ == 0 && currency_ == 0 && count_ == 0 && equation_ == 0;
        }
        constexpr int meter() const { return meter_; }
        constexpr int kg() const { return kilogram_; }
        constexpr int second() const { return second_; }
        constexpr int ampere() const { return ampere_; }
        constexpr int kelvin() const { return kelvin_; }
        constexpr int mole() const { return mole_; }
        constexpr int candela() const { return candela_; }
        constexpr int currency() const { return currency_; }
        constexpr int count() const { return count_; }
        constexpr int radian() const { return radians_; }

        /// set all the flags to 0;
        void clear_flags() { per_unit_ = flag_ = e_flag_ = equation_ = 0; }

      private:
        constexpr unit_data()
            : meter_(0), second_(0), kilogram_(0), ampere_(0), candela_(0), kelvin_(0), mole_(0), radians_(0),
              currency_(0), count_(0), per_unit_(0), flag_(0), e_flag_(0), equation_(0)
        {
        }

        constexpr bool hasValidRoot(int power) const
        {
            return meter_ % power == 0 && second_ % power == 0 && kilogram_ % power == 0 && ampere_ % power == 0 &&
                   candela_ == 0 && kelvin_ % power == 0 && mole_ == 0 && radians_ % power == 0 &&
                   currency_ == 0 && count_ == 0 && equation_ == 0 && e_flag_ == 0;
        }
        // needs to be defined for the full 32 bits
        signed int meter_ : 4;
        signed int second_ : 4;  // 8
        signed int kilogram_ : 3;
        signed int ampere_ : 3;
        signed int candela_ : 2;  // 16
        signed int kelvin_ : 3;
        signed int mole_ : 2;
        signed int radians_ : 3;  // 24
        signed int currency_ : 2;
        signed int count_ : 2;  // 28
        unsigned int per_unit_ : 1;
        unsigned int flag_ : 1;  // 30
        unsigned int e_flag_ : 1;  //
        unsigned int equation_ : 1;  // 32
    };
    // We want this to be exactly 4 bytes by design
    static_assert(sizeof(unit_data) == 4, "Unit data is too large");

}  // namespace detail
}  // namespace units

namespace std
{
/// Hash function for unit_data
template <>
struct hash<units::detail::unit_data>
{
    size_t operator()(const units::detail::unit_data &x) const noexcept
    {
        return hash<unsigned int>()(*reinterpret_cast<const unsigned int *>(&x));
    }
};
}  // namespace std

namespace units
{
namespace detail
{
    /// constexpr operator to generate an integer power of a number
    template <typename X>
    constexpr X power_const(X val, int power)
    {
        return (power > 0) ? val * power_const(val, power - 1) :
                             (power < 0) ? X(1.0) / (val * power_const(val, -power - 1)) : X(1.0);
    }

    /// Round the multiplier to the expected level of precision
    inline float cround(float val)
    {
        int exp;
        auto f = frexpf(val, &exp);
        f = roundf(f * 1e6f);
        return ldexpf(f * 1e-6f, exp);
    }

    /// Round a value to the expected level of precision of a double
    inline double cround_precise(double val)
    {
        int exp;
        auto f = frexp(val, &exp);
        f = round(f * 1e12);
        return ldexp(f * 1e-12, exp);
    }
}  // namespace detail

/**Class defining a basic unit module with float32 precision on the multiplier.
@details The class consists of a unit_base along with a 32 bit floating point number
*/
class unit
{
  public:
    /// Default constructor
    constexpr unit() = default;
    explicit constexpr unit(detail::unit_data base_unit) : base_units_(base_unit) {}
    /// Construct unit from base unit and a multiplier
    constexpr unit(detail::unit_data base_unit, double multiplier)
        : base_units_(base_unit), multiplier_(static_cast<float>(multiplier))
    {
    }
    /// Take the double and unit in either order for simplicity
    constexpr unit(double multiplier, unit other) : unit(other.base_units_, multiplier * other.multiplier()) {}
    /// Unit multiplication
    constexpr unit operator*(unit other) const
    {
        return {base_units_ + other.base_units_, multiplier() * other.multiplier()};
    }
    /// Division operator
    constexpr unit operator/(unit other) const
    {
        return {base_units_ - other.base_units_, multiplier() / other.multiplier()};
    }
    /// Invert the unit (take 1/unit)
    constexpr unit inv() const { return {base_units_.inv(), 1.0 / multiplier()}; }
    /// take a unit to an integral power
    constexpr unit pow(int power) const
    {
        return {base_units_.pow(power), detail::power_const(multiplier_, power)};
    }
#ifndef UNITS_HEADER_ONLY
    /// take the root of a unit to some power
    unit root(int power) const;
#endif
    /// Test for unit equivalence to within nominal numerical tolerance (6 decimal digits)
    bool operator==(unit other) const
    {
        return base_units_ == other.base_units_ &&
               detail::cround(multiplier_) == detail::cround(other.multiplier_);
    }
    bool operator!=(unit other) const { return !operator==(other); }
    /// Check if the unit has an error
    constexpr bool is_error() const
    {
        return (multiplier_ != multiplier_ ||
                (base_units_.has_e_flag() && base_units_.is_flag() && base_units_.empty()));
    }
    // Test for exact numerical equivalence
    constexpr bool is_exactly_the_same(unit other) const
    {
        return base_units_ == other.base_units_ && multiplier_ == other.multiplier_;
    }
    /// Check if the units have the same base unit (ie they measure the same thing)
    constexpr bool has_same_base(unit other) const { return base_units_.has_same_base(other.base_units_); }
    constexpr bool has_same_base(detail::unit_data base) const { return base_units_.has_same_base(base); }
    /// Check if the units have the same base unit (ie they measure the same thing)
    constexpr bool equivalent_non_counting(unit other) const
    {
        return base_units_.equivalent_non_counting(other.base_units_);
    }
    constexpr bool equivalent_non_counting(detail::unit_data base) const
    {
        return base_units_.equivalent_non_counting(base);
    }
    /// Check if the units are in some way convertible to one another
    constexpr bool is_convertible(unit other) const
    {
        return base_units_.equivalent_non_counting(other.base_units_);
    }
    /// Check if the base units are in some way convertible to one another
    constexpr bool is_convertible(detail::unit_data base) const
    {
        return base_units_.equivalent_non_counting(base);
    }
    /// Check if the unit is a temperature value
    constexpr bool is_temperature() const { return base_units_.is_temperature(); }
    /// Check if the unit is the default unit
    constexpr bool is_default() const { return base_units_.empty() && base_units_.is_flag(); }
    /// Check if the unit is a per_unit notation
    constexpr bool is_per_unit() const { return base_units_.is_per_unit(); }
    /// Check if the unit is a per_unit notation
    constexpr bool is_equation() const { return base_units_.is_equation(); }
    /// Extract the base unit Multiplier
    constexpr double multiplier() const { return static_cast<double>(multiplier_); }
    /// generate a rounded version of the multiplier
    float cround() const { return detail::cround(multiplier_); }
    constexpr detail::unit_data base_units() const { return base_units_; }
    /// set all the flags to 0;
    void clear_flags() { base_units_.clear_flags(); }

  private:
    friend class precise_unit;
    detail::unit_data base_units_{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float multiplier_{1.0};
};

/**Class defining a basic unit module with double precision on the multiplier.
@details The class consists of a unit_base along with a 64 bit floating point number
*/
class precise_unit
{
  public:
    /// Default constructor
    constexpr precise_unit() = default;
    explicit constexpr precise_unit(detail::unit_data base_unit) : base_units_(base_unit) {}
    /// copy constructor from a less precise unit
    explicit constexpr precise_unit(unit other) : base_units_(other.base_units_), multiplier_(other.multiplier())
    {
    }
    /// Construct from base_unit and multiplier
    constexpr precise_unit(detail::unit_data base_unit, double multiplier)
        : base_units_(base_unit), multiplier_(multiplier)
    {
    }
    /// Construct from base_unit, commodity and multiplier
    constexpr precise_unit(detail::unit_data base_unit, uint32_t commodity, double multiplier)
        : base_units_(base_unit), commodity_(commodity), multiplier_(multiplier)
    {
    }
    /// Copy constructor with a multiplier
    constexpr precise_unit(precise_unit other, double multiplier)
        : precise_unit(other.base_units_, multiplier * other.multiplier_)
    {
    }
    /// Constructor for a double and other unit
    constexpr precise_unit(unit other, double multiplier)
        : precise_unit(other.base_units(), multiplier * other.multiplier())
    {
    }
    constexpr precise_unit(double multiplier, precise_unit other)
        : precise_unit(other.base_units_, multiplier * other.multiplier_)
    {
    }
    /// Build a unit from another with a multiplier and commodity
    constexpr precise_unit(double multiplier, precise_unit other, uint32_t commodity)
        : precise_unit(other.base_units_, commodity, multiplier * other.multiplier_)
    {
    }
    /// Constructor for a double and other unit
    constexpr precise_unit(double multiplier, unit other)
        : precise_unit(other.base_units(), multiplier * other.multiplier())
    {
    }
    /// take the reciprocal of a unit
    constexpr precise_unit inv() const
    {
        return {base_units_.inv(), (commodity_ == 0) ? 0 : ~commodity_, 1.0 / multiplier()};
    }
    /// Multiply with another unit
    constexpr precise_unit operator*(precise_unit other) const
    {
        return {base_units_ + other.base_units_,
                (commodity_ == 0) ? other.commodity_ :
                                    ((other.commodity_ == 0) ? commodity_ : commodity_ & other.commodity_),
                multiplier() * other.multiplier()};
    }
    /// Multiplication operator with a lower precision unit
    constexpr precise_unit operator*(unit other) const
    {
        return {base_units_ + other.base_units_, commodity_, multiplier() * other.multiplier()};
    }
    /// Division operator
    constexpr precise_unit operator/(precise_unit other) const
    {
        return {base_units_ - other.base_units_,
                (commodity_ == 0) ? ((other.commodity_ == 0) ? 0 : ~other.commodity_) :
                                    ((other.commodity_ == 0) ? commodity_ : commodity_ & (~other.commodity_)),
                multiplier() / other.multiplier()};
    }
    /// Divide by a less precise unit
    constexpr precise_unit operator/(unit other) const
    {
        return {base_units_ - other.base_units_, commodity_, multiplier() / other.multiplier()};
    }
    /// take a unit to a power
    constexpr precise_unit pow(int power) const
    {
        return {base_units_.pow(power), commodity_, detail::power_const(multiplier_, power)};
    }
#ifndef UNITS_HEADER_ONLY
    /// take the root of a unit to some power
    precise_unit root(int power) const;
#endif
    /// Overloaded equality operator
    bool operator==(precise_unit other) const
    {
        return base_units_ == other.base_units_ && commodity_ == other.commodity_ &&
               detail::cround_precise(multiplier_) == detail::cround_precise(other.multiplier_);
    }
    bool operator!=(precise_unit other) const { return !operator==(other); }
    // Test for exact numerical equivalence
    constexpr bool is_exactly_the_same(precise_unit other) const
    {
        return base_units_ == other.base_units_ && commodity_ == other.commodity_ &&
               multiplier_ == other.multiplier_;
    }
    /// Check if the units have the same base unit (ie they measure the same thing)
    constexpr bool has_same_base(precise_unit other) const { return base_units_.has_same_base(other.base_units_); }
    bool operator==(unit other) const
    {
        return base_units_ == other.base_units_ && detail::cround(static_cast<float>(multiplier())) ==
                                                     detail::cround(static_cast<float>(other.multiplier()));
    }
    bool operator!=(unit other) const { return !operator==(other); }
    /// Check if the units have the same base unit (ie they measure the same thing)
    constexpr bool has_same_base(unit other) const { return base_units_.has_same_base(other.base_units_); }

    constexpr bool has_same_base(detail::unit_data base) const { return base_units_.has_same_base(base); }

    /// Check if the units have the same base unit (ie they measure the same thing)
    constexpr bool equivalent_non_counting(precise_unit other) const
    {
        return base_units_.equivalent_non_counting(other.base_units_);
    }
    constexpr bool equivalent_non_counting(unit other) const
    {
        return base_units_.equivalent_non_counting(other.base_units_);
    }
    constexpr bool equivalent_non_counting(detail::unit_data base) const
    {
        return base_units_.equivalent_non_counting(base);
    }

    /// Check if the units are in some way convertible to one another
    constexpr bool is_convertible(precise_unit other) const
    {
        return commodity_ == other.commodity_ && base_units_.equivalent_non_counting(other.base_units_);
    }
    constexpr bool is_convertible(unit other) const
    {
        return base_units_.equivalent_non_counting(other.base_units_);
    }
    constexpr bool is_convertible(detail::unit_data base) const
    {
        return base_units_.equivalent_non_counting(base);
    }
    /// Check unit equality (base units equal and equivalent multipliers to specified precision
    friend bool operator==(unit val1, precise_unit val2)
    {
        return val1.base_units() == val2.base_units() && detail::cround(static_cast<float>(val1.multiplier())) ==
                                                           detail::cround(static_cast<float>(val2.multiplier()));
    }
    /// Check if the unit is the default unit
    constexpr bool is_default() const { return base_units_.empty() && base_units_.is_flag(); }
    /// Check if the unit is a temperature
    constexpr bool is_temperature() const { return base_units_.is_temperature(); }
    /// Check if the unit is a per unit value
    constexpr bool is_per_unit() const { return base_units_.is_per_unit(); }
    /// Check if the unit has an error
    constexpr bool is_error() const
    {
        return (multiplier_ != multiplier_ ||
                (base_units_.has_e_flag() && base_units_.is_flag() && base_units_.empty()));
    }
    /// Check if the unit is a per_unit notation
    constexpr bool is_equation() const { return base_units_.is_equation(); }
    /// Get the commodity code
    constexpr int32_t commodity() const { return commodity_; }
    /// Extract the base unit Multiplier
    constexpr double multiplier() const { return multiplier_; }
    /// Generate a rounded value of the multiplier rounded to the defined precision
    double cround() const { return detail::cround_precise(multiplier_); }
    /// Get the base units
    constexpr detail::unit_data base_units() const { return base_units_; }
    /// set all the flags to 0;
    void clear_flags() { base_units_.clear_flags(); }
    /// Set the commodity
    precise_unit &commodity(unsigned int newCommodity)
    {
        commodity_ = newCommodity;
        return *this;
    }

  private:
    detail::unit_data base_units_{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int commodity_{0};  //!< a commodity specifier
    double multiplier_{1.0};  //!< unit multiplier
};
/// Check if a unit down cast is lossless
inline constexpr bool is_unit_cast_lossless(precise_unit val)
{
    return val.multiplier() == static_cast<double>(static_cast<float>(val.multiplier()));
}
/// Downcast a precise unit to the less precise version
constexpr unit unit_cast(precise_unit val) { return {val.base_units(), val.multiplier()}; }
constexpr unit unit_cast(unit val) { return val; }

/// Check if the multiplier is nan
inline bool isnan(precise_unit u) { return std::isnan(u.multiplier()); }
inline bool isnan(unit u) { return std::isnan(u.multiplier()); }
/// Check if the multiplier is inf
inline bool isinf(precise_unit u) { return std::isinf(u.multiplier()); }
inline bool isinf(unit u) { return std::isinf(u.multiplier()); }
// Verify that the units are the expected sizes
static_assert(sizeof(unit) == 8, "Unit type is too large");
static_assert(sizeof(precise_unit) == 16, "precise unit type is too large");

}  // namespace units

/// Defining the hash functions for a unit and precise_unit so they can be used in unordered_map
namespace std
{
template <>
struct hash<units::unit>
{
    size_t operator()(const units::unit &x) const
    {
        return hash<units::detail::unit_data>()(x.base_units()) ^ hash<float>()(x.cround());
    }
};

template <>
struct hash<units::precise_unit>
{
    size_t operator()(const units::precise_unit &x) const
    {
        return hash<units::detail::unit_data>()(x.base_units()) ^ hash<double>()(x.cround());
    }
};
}  // namespace std

// std::ostream &operator<<(std::ostream &stream, units::unit u);