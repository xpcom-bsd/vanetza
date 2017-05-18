#include <gtest/gtest.h>
#include <vanetza/common/clock.hpp>
#include <vanetza/dcc/regular_budget.hpp>
#include <vanetza/dcc/state_machine.hpp>

using namespace vanetza::dcc;
using vanetza::Clock;
using std::chrono::milliseconds;

static const Clock::duration immediately = milliseconds(0);

class RegularBudgetTest : public ::testing::Test
{
protected:
    RegularBudgetTest() : now(std::chrono::seconds(4711)), budget(fsm, now) {}

    Clock::time_point now;
    StateMachine fsm;
    RegularBudget budget;
};

TEST_F(RegularBudgetTest, relaxed)
{
    Relaxed relaxed;
    const auto relaxed_tx_interval = relaxed.transmission_interval();
    ASSERT_EQ(relaxed_tx_interval, fsm.transmission_interval());

    EXPECT_EQ(immediately, budget.delay());
    budget.notify();
    EXPECT_EQ(relaxed_tx_interval, budget.delay());

    now += relaxed_tx_interval - milliseconds(10);
    EXPECT_EQ(milliseconds(10), budget.delay());

    now += milliseconds(20);
    EXPECT_EQ(immediately, budget.delay());
}

TEST_F(RegularBudgetTest, restrictive)
{
    Restrictive restrictive;
    const auto restrictive_tx_interval = restrictive.transmission_interval();

    // put FSM into restrictive state
    for (unsigned i = 0; i < 10; ++i) {
        fsm.update(ChannelLoad(0.6));
    }
    ASSERT_STREQ("Restrictive", fsm.state().name());

    EXPECT_EQ(immediately, budget.delay());
    budget.notify();
    EXPECT_EQ(restrictive_tx_interval, budget.delay());

    now += restrictive_tx_interval / 2;
    EXPECT_EQ(restrictive_tx_interval / 2, budget.delay());
    now += restrictive_tx_interval / 2;
    EXPECT_EQ(immediately, budget.delay());
}
