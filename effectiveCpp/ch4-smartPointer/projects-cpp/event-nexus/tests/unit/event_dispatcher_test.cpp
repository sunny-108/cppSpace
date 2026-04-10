/// @file event_dispatcher_test.cpp
/// @brief Unit tests for EventDispatcher.

#include <event_nexus/event_dispatcher.h>

#include <gtest/gtest.h>

#include <string>

using namespace nexus;

struct Msg {
    int code;
    std::string text;
};

struct Other {
    double val;
};

class EventDispatcherTest : public ::testing::Test {
protected:
    EventDispatcher dispatcher;
};

TEST_F(EventDispatcherTest, DispatchToMatchingSubscriber) {
    int received = 0;
    auto sub = makeSubscriber<Msg>([&received](const Msg& m) { received = m.code; });
    dispatcher.subscribe(sub);

    auto event = makeEvent<Msg>(42, "hello");
    auto count = dispatcher.dispatch(*event);

    EXPECT_EQ(count, 1u);
    EXPECT_EQ(received, 42);
}

TEST_F(EventDispatcherTest, NoDispatchForUnmatchedType) {
    int received = 0;
    auto sub = makeSubscriber<Msg>([&received](const Msg& m) { received = m.code; });
    dispatcher.subscribe(sub);

    auto event = makeEvent<Other>(3.14);
    auto count = dispatcher.dispatch(*event);

    EXPECT_EQ(count, 0u);
    EXPECT_EQ(received, 0);
}

TEST_F(EventDispatcherTest, ExpiredSubscriberIsSkipped) {
    int received = 0;
    {
        auto sub = makeSubscriber<Msg>([&received](const Msg& m) { received = m.code; });
        dispatcher.subscribe(sub);
    }
    // sub destroyed — weak_ptr expired.

    auto event = makeEvent<Msg>(99, "expired");
    auto count = dispatcher.dispatch(*event);

    EXPECT_EQ(count, 0u);
    EXPECT_EQ(received, 0);
}

TEST_F(EventDispatcherTest, PurgeRemovesExpired) {
    auto sub1 = makeSubscriber<Msg>([](const Msg&) {});
    auto sub2 = makeSubscriber<Msg>([](const Msg&) {});

    dispatcher.subscribe(sub1);
    dispatcher.subscribe(sub2);
    EXPECT_EQ(dispatcher.subscriberCount(), 2u);

    sub1.reset();
    auto purged = dispatcher.purgeExpired();
    EXPECT_EQ(purged, 1u);
    EXPECT_EQ(dispatcher.subscriberCount(), 1u);
}

TEST_F(EventDispatcherTest, ActiveVsTotalCount) {
    auto sub1 = makeSubscriber<Msg>([](const Msg&) {});
    auto sub2 = makeSubscriber<Msg>([](const Msg&) {});

    dispatcher.subscribe(sub1);
    dispatcher.subscribe(sub2);

    EXPECT_EQ(dispatcher.subscriberCount(), 2u);
    EXPECT_EQ(dispatcher.activeSubscriberCount(), 2u);

    sub1.reset();
    EXPECT_EQ(dispatcher.subscriberCount(), 2u);       // total (includes expired)
    EXPECT_EQ(dispatcher.activeSubscriberCount(), 1u);  // live only
}

TEST_F(EventDispatcherTest, NullSubscriberIgnored) {
    dispatcher.subscribe(nullptr);
    EXPECT_EQ(dispatcher.subscriberCount(), 0u);
}

TEST_F(EventDispatcherTest, MultipleEventTypes) {
    int msgCount = 0;
    int otherCount = 0;

    auto sub1 = makeSubscriber<Msg>([&msgCount](const Msg&) { ++msgCount; });
    auto sub2 = makeSubscriber<Other>([&otherCount](const Other&) { ++otherCount; });

    dispatcher.subscribe(sub1);
    dispatcher.subscribe(sub2);

    dispatcher.dispatch(*makeEvent<Msg>(1, "a"));
    dispatcher.dispatch(*makeEvent<Other>(1.0));

    EXPECT_EQ(msgCount, 1);
    EXPECT_EQ(otherCount, 1);
}
