
#include <iostream>
#include <string_view>
#include <thread>
#include <array>
#include <ranges>
#include <algorithm>
#include <future>
#include <execution>
#include <format>

#include "../TopTracker/TopTracker.h"

namespace test
{
	inline namespace stream_output
	{
		static void print_test_passed(std::string_view msg) noexcept
		{
			static constexpr std::string_view SUCCESS_FMT = "\033[32m[OK]   \033[0m{}\n";
			std::clog << std::format(SUCCESS_FMT, msg);
		}

		static void print_test_failed(std::string_view msg) noexcept
		{
			static constexpr std::string_view FAIL_FMT = "\033[31m[FAIL] \033[0m{}\n";
			std::cerr << std::format(FAIL_FMT, msg);
		}
	}

	namespace compiletime
	{
		namespace is_noexcept
		{
			static void constructible_player_action() noexcept
			{
				if constexpr (std::is_nothrow_constructible_v<PlayerAction, PlayerAction::PlayerId, PlayerAction::Type>)
					print_test_passed("PlayerAction is noexcept-constructible");
				else
					print_test_failed("PlayerAction is NOT noexcept-constructible");
			}

			static void copyable_player_action() noexcept
			{
				if constexpr (std::is_nothrow_copy_constructible_v<PlayerAction> &&
							  std::is_nothrow_move_constructible_v<PlayerAction> &&
							  std::is_nothrow_copy_assignable_v<PlayerAction> &&
							  std::is_nothrow_move_assignable_v<PlayerAction> &&
							  std::is_nothrow_destructible_v<PlayerAction>)
					print_test_passed("PlayerAction is fully noexcept-copyable and destructible");
				else
					print_test_failed("PlayerAction is NOT fully noexcept-copyable or destructible");
			}

			static void public_interface_player_action() noexcept
			{
				constexpr bool get_id = noexcept(std::declval<const PlayerAction&>().get_player_id());
				constexpr bool get_type = noexcept(std::declval<const PlayerAction&>().get_type());
				constexpr bool get_time = noexcept(std::declval<const PlayerAction&>().get_time_stamp());

				if constexpr (get_id && get_type && get_time)
					print_test_passed("PlayerAction public interface methods are noexcept");
				else
					print_test_failed("PlayerAction public interface methods are NOT noexcept");
			}

			static void constructible_toptracker() noexcept
			{
				if constexpr (std::is_nothrow_constructible_v<TopTracker, std::chrono::seconds, std::size_t>)
					print_test_passed("TopTracker is noexcept-constructible");
				else
					print_test_failed("TopTracker is NOT noexcept-constructible");
			}

			static void public_interface_toptracker() noexcept
			{
				constexpr bool on_act = noexcept(std::declval<TopTracker&>().on_action(0, PlayerAction::Type::BUY));
				constexpr bool del_old = noexcept(std::declval<TopTracker&>().delete_old_actions());
				constexpr bool view = noexcept(std::declval<const TopTracker&>().get_actions_view());
				constexpr bool copy = noexcept(std::declval<const TopTracker&>().get_actions_copy());

				if constexpr (on_act && del_old && view && copy)
					print_test_passed("TopTracker public interface methods are noexcept");
				else
					print_test_failed("TopTracker public interface methods are NOT noexcept");
			}

			static constexpr std::array TESTS
			{
				constructible_player_action, copyable_player_action,
				public_interface_player_action, constructible_toptracker,
				public_interface_toptracker
			};
		}
	}

	namespace runtime
	{
		static void basic_insertion()
		{
			TopTracker tracker(std::chrono::seconds{10}, 5);

			tracker.on_action(1, PlayerAction::Type::BUY);
			tracker.on_action(2, PlayerAction::Type::SELL);

			const auto& actions = tracker.get_actions_view();

			const bool passed = actions.size() == 2 &&
								actions[0].get_player_id() == 1 &&
								actions[1].get_player_id() == 2;
			
			if (passed)
				print_test_passed("Basic insertion works");
			else
				print_test_failed("Basic insertion failed");
		}

		static void capacity_limit()
		{
			TopTracker tracker(std::chrono::seconds{10}, 3);

			tracker.on_action(1, PlayerAction::Type::BUY);
			tracker.on_action(2, PlayerAction::Type::SELL);
			tracker.on_action(3, PlayerAction::Type::WIN);
			tracker.on_action(4, PlayerAction::Type::LOSE); // первый должен быть вытеснен

			const auto& actions = tracker.get_actions_view();

			const bool passed = actions.size() == 3 &&
								actions[0].get_player_id() == 2 &&
								actions[1].get_player_id() == 3 &&
								actions[2].get_player_id() == 4;
			
			if (passed)
				print_test_passed("Capacity limit and pop_front logic works");
			else
				print_test_failed("Capacity limit logic failed");
		}

		static void timeout_cleanup()
		{
			using namespace std::chrono_literals;
			TopTracker tracker(2s, 10);

			tracker.on_action(10, PlayerAction::Type::BUY);
			std::this_thread::sleep_for(3s);
			tracker.on_action(11, PlayerAction::Type::SELL);

			tracker.delete_old_actions();

			const auto& actions = tracker.get_actions_view();

			const bool passed = actions.size() == 1 && actions[0].get_player_id() == 11;

			if (passed)
				print_test_passed("Timeout cleanup works correctly");
			else
				print_test_failed("Timeout cleanup failed");
		}

		static void action_ordering()
		{
			TopTracker tracker(std::chrono::seconds{10}, 5);

			tracker.on_action(1, PlayerAction::Type::WIN);
			tracker.on_action(2, PlayerAction::Type::LOSE);
			tracker.on_action(3, PlayerAction::Type::SELL);

			const auto v = tracker.get_actions_copy();

			const bool passed = v.size() == 3 &&
								v[0].get_player_id() == 1 &&
								v[1].get_player_id() == 2 &&
								v[2].get_player_id() == 3;

			if (passed)
				print_test_passed("Action ordering is preserved");
			else
				print_test_failed("Action ordering is broken");
		}

		static void copy_interface()
		{
			TopTracker tracker(std::chrono::seconds{10}, 2);
			tracker.on_action(7, PlayerAction::Type::SELL);
			tracker.on_action(8, PlayerAction::Type::WIN);

			const auto copy = tracker.get_actions_copy();

			const bool passed = copy.size() == 2 &&
								copy[0].get_player_id() == 7 &&
								copy[1].get_player_id() == 8;

			if (passed)
				print_test_passed("Copy interface returns correct data");
			else
				print_test_failed("Copy interface returned incorrect result");
		}

		static void monotonic_timestamps()
		{
			TopTracker tracker(std::chrono::seconds{10}, 10);
			for (PlayerAction::PlayerId i = 0; i < PlayerAction::PlayerId(5); ++i)
			{
				tracker.on_action(i, PlayerAction::Type::BUY);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			const auto& v = tracker.get_actions_view();
			bool passed = true;
			for (size_t i = 1; i < v.size(); ++i)
			{
				if (v[i].get_time_stamp() < v[i - 1].get_time_stamp())
				{
					passed = false;
					break;
				}
			}
			if (passed)
				print_test_passed("Actions is ordered correctly by timestamp");
			else
				print_test_failed("Actions is not correctly ordered by timestamp");
		}

		static void duplicate_actions_allowed()
		{
			TopTracker tracker(std::chrono::seconds{5}, 10);
			tracker.on_action(42, PlayerAction::Type::SELL);
			tracker.on_action(42, PlayerAction::Type::SELL);

			const auto& v = tracker.get_actions_view();
			const bool passed = v.size() == 2 &&
								v[0].get_player_id() == 42 &&
								v[1].get_type() == PlayerAction::Type::SELL;


			if (passed)
				print_test_passed("Duplicated actions are saving");
			else
				print_test_failed("Duplicated actions are not saving");
		}

		static void delete_old_actions_keeps_fresh()
		{
			using namespace std::chrono_literals;
			TopTracker tracker(1s, 10);

			tracker.on_action(1, PlayerAction::Type::WIN);
			std::this_thread::sleep_for(2s);
			tracker.on_action(2, PlayerAction::Type::LOSE);

			tracker.delete_old_actions();

			const auto& v = tracker.get_actions_view();
			const bool passed = v.size() == 1 && v[0].get_player_id() == 2;

			if (passed)
				print_test_passed("Deletion and detection of expired actions are correct");
			else
				print_test_failed("Deletion and detection of expired actions are not correct");
		}

		static void get_copy_equals_view()
		{
			TopTracker tracker(std::chrono::seconds{10}, 5);
			tracker.on_action(1, PlayerAction::Type::BUY);
			tracker.on_action(2, PlayerAction::Type::SELL);

			auto copy = tracker.get_actions_copy();
			const auto& view = tracker.get_actions_view();

			bool passed = true;
			if (copy.size() != view.size()) passed = false;
			else
			{
				for (size_t i = 0; i < copy.size(); ++i)
				{
					if (copy[i].get_player_id() != view[i].get_player_id())
					{
						passed = false;
						break;
					}
				}
			}

			if (passed)
				print_test_passed("Copy and view getters return same actions");
			else
				print_test_failed("Copy and view getters return different actions");
		}
	
		static void concurrent_on_action()
		{
			constexpr int thread_count = 8;
			constexpr int actions_per_thread = 1000;

			TopTracker tracker(std::chrono::seconds{60}, thread_count * actions_per_thread);

			std::atomic<bool> failed = false;
			{
				std::vector<std::jthread> threads;

				for (PlayerAction::PlayerId i = 0; i < PlayerAction::PlayerId(thread_count); ++i)
				{
					threads.emplace_back([&, i]
					{
						for (int j = 0; j < actions_per_thread; ++j)
						{
							try
							{
								tracker.on_action(i, PlayerAction::Type::WIN);
							}
							catch (...)
							{
								failed = true;
							}
						}
					});
				}
			}

			// Простой sanity check: либо всё добавлено, либо нет
			const auto& v = tracker.get_actions_view();
			bool correct_size = v.size() == static_cast<size_t>(thread_count * actions_per_thread);

			if (!failed && correct_size)
				print_test_passed("Multithreaded on action worked (threadsafe)");
			else
				print_test_failed("Multithreaded on action failed (not threadsafe)");
		}

		static constexpr std::array TESTS
		{
			basic_insertion, capacity_limit, timeout_cleanup, 
			action_ordering, copy_interface, monotonic_timestamps,
			duplicate_actions_allowed, delete_old_actions_keeps_fresh,
			get_copy_equals_view, concurrent_on_action
		};
	}
}

int main()
{
	using namespace test::stream_output;
	const auto run_tests = [](const auto& tests)
	{
		const auto future_tests =
			tests | std::views::transform([](const auto& test) { return std::async(test); });
		std::for_each(std::execution::par_unseq, future_tests.begin(), future_tests.end(),
			[](const auto& future_test) { future_test.wait(); });
	};

	std::clog << ">>> Starts compiletime-noexcept tests <<<\n";
	{
		using namespace test::compiletime::is_noexcept;
		run_tests(TESTS);
	}
	std::clog << ">>> Ends compiletime-noexcept tests <<<\n";

	std::clog << ">>> Starts runtime tests <<<\n";
	{
		using namespace test::runtime;
		run_tests(TESTS);
	}
	std::clog << ">>> Ends runtime tests <<<\n";

	return 0;
}