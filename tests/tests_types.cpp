/*
 * tests_types.cpp
 *
 * Copyright 2009-2021
 * Giuseppe Penone <giuspen@gmail.com>
 * Evgenii Gurianov <https://github.com/txe>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "ct_types.h"
#include "ct_filesystem.h"
#include "tests_common.h"

TEST(TestTypesGroup, ctMaxSizedList)
{
    CtMaxSizedList<int> maxSizedList{3};
    maxSizedList.push_back(1);
    maxSizedList.push_back(2);
    maxSizedList.push_back(3);
    ASSERT_EQ(3, maxSizedList.size());
    ASSERT_EQ(1, maxSizedList.front());
    ASSERT_EQ(3, maxSizedList.back());
    maxSizedList.move_or_push_back(1);
    ASSERT_EQ(3, maxSizedList.size());
    ASSERT_EQ(2, maxSizedList.front());
    ASSERT_EQ(1, maxSizedList.back());
    maxSizedList.move_or_push_front(3);
    ASSERT_EQ(3, maxSizedList.size());
    ASSERT_EQ(3, maxSizedList.front());
    ASSERT_EQ(1, maxSizedList.back());
    maxSizedList.move_or_push_front(4);
    ASSERT_EQ(3, maxSizedList.size());
    ASSERT_EQ(4, maxSizedList.front());
    ASSERT_EQ(2, maxSizedList.back());
}

TEST(TestTypesGroup, scope_guard)
{
    int var = -1;
    {
        auto on_scope_exit = scope_guard([&](void*) {
            var = 0;
        });
        ASSERT_EQ(-1, var);
    }
    ASSERT_EQ(0, var);
}

TEST(TestTypesGroup, ThreadSafeDEQueue_1)
{
    ThreadSafeDEQueue<int,500> threadSafeDEQueue;
    auto f_first = [&threadSafeDEQueue](){
        ASSERT_EQ(1, threadSafeDEQueue.pop_front());
        threadSafeDEQueue.push_back(2);
        while (true) {
            g_usleep(1);
            std::optional<int> peeked = threadSafeDEQueue.peek();
            if (not peeked.has_value() or 3 == peeked.value()) {
                break;
            }
        }
        ASSERT_EQ(3, threadSafeDEQueue.pop_front());
    };
    auto f_second = [&threadSafeDEQueue](){
        ASSERT_TRUE(threadSafeDEQueue.empty());
        threadSafeDEQueue.push_back(1);
        while (true) {
            g_usleep(1);
            std::optional<int> peeked = threadSafeDEQueue.peek();
            if (not peeked.has_value() or 2 == peeked.value()) {
                break;
            }
        }
        ASSERT_EQ(2, threadSafeDEQueue.pop_front());
        threadSafeDEQueue.push_back(3);
    };
    std::thread first(f_first);
    std::thread second(f_second);
    first.join();
    second.join();
}

TEST(TestTypesGroup, ThreadSafeDEQueue_2)
{
    ThreadSafeDEQueue<int,500> threadSafeDEQueue;
    auto f_first = [&threadSafeDEQueue](){
        for (int i = 0; i < 100; ++i) {
            g_usleep(1);
            threadSafeDEQueue.push_back(i);
        }
    };
    auto f_second = [&threadSafeDEQueue](){
        for (int i = 0; i < 200; ++i) {
            threadSafeDEQueue.pop_front();
        }
    };
    std::thread first(f_first);
    std::thread second(f_second);
    std::thread third(f_first);
    first.join();
    second.join();
    third.join();
    ASSERT_TRUE(threadSafeDEQueue.empty());
}

TEST(TestTypesGroup, SafeQueue_3)
{
    ThreadSafeDEQueue<int,3> threadSafeDEQueue;
    for (int i = 0; i < 5; ++i) {
        threadSafeDEQueue.push_back(i);
    }
    ASSERT_EQ(3, threadSafeDEQueue.size());
}
