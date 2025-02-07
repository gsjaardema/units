/*
Copyright (c) 2019,
Lawrence Livermore National Security, LLC;
See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "test.hpp"
#include "units/units.hpp"

#include <type_traits>

using namespace units;
TEST(MeasurementStrings, basic)
{
    auto pm = measurement_from_string("45 m");
    EXPECT_EQ(pm, 45.0 * precise::m);

    // pm = measurement_from_string("9.0 * 5.0 m");
    // EXPECT_EQ(pm, 45.0 * precise::m);

    pm = measurement_from_string("23.7 m/s");
    EXPECT_EQ(pm, 23.7 * precise::m / precise::s);
    pm = measurement_from_string("99.9 N * m");
    EXPECT_EQ(pm, 99.9 * precise::N * precise::m);
}

TEST(MeasurementStrings, invalid)
{
    auto pm = measurement_from_string("345 blarg");
    EXPECT_FALSE(is_valid(pm.units()));
}

TEST(MeasurementStrings, currency)
{
    auto pm = measurement_from_string("$9.99");
    EXPECT_EQ(pm, 9.99 * precise::currency);

    pm = measurement_from_string("$ 9.99");
    EXPECT_EQ(pm, 9.99 * precise::currency);
}

TEST(MeasurementToString, simple)
{
    auto pm = precision_measurement(45.0, precise::m);
    EXPECT_EQ(to_string(pm), "45 m");
    auto meas = 45.0 * m;
    EXPECT_EQ(to_string(meas), "45 m");
    auto meas_f = 45.0f * m;
    EXPECT_EQ(to_string(meas_f), "45 m");

    measurement_f meas_f2(45.0f, m);
    EXPECT_EQ(to_string(meas_f2), "45 m");
}
