
#include <iostream>
#include <string_view>
#include <thread>

#include "../TopTracker/TopTracker.h"

namespace test
{
	inline namespace stream_output
	{
		inline namespace colors
		{
			static constexpr std::string_view RED = "\033[31m";
			static constexpr std::string_view GREEN = "\033[32m";
			static constexpr std::string_view RESET = "\033[0m";

			static constexpr std::string_view TAG_SUCCESS = "[OK]   ";
			static constexpr std::string_view TAG_FAILURE = "[FAIL] ";
		}

		static void print_test_passed(std::string_view msg) noexcept
		{
			std::clog << GREEN << TAG_SUCCESS << RESET << msg << std::endl;
		}

		static void print_test_failed(std::string_view msg) noexcept
		{
			std::cerr << RED << TAG_FAILURE << RESET << msg << std::endl;
		}
	}

	namespace compiletime
	{
		namespace is_noexcept
		{
			static constexpr bool constructible_player_action() noexcept
			{
				using PA = PlayerAction;
				return std::is_nothrow_constructible_v<PA, PA::PlayerId, PA::Type>;
			}

			static constexpr bool copyable_player_action() noexcept
			{
				using PA = PlayerAction;
				return std::is_nothrow_copy_constructible_v<PA> &&
					std::is_nothrow_move_constructible_v<PA> &&
					std::is_nothrow_copy_assignable_v<PA> &&
					std::is_nothrow_move_assignable_v<PA> &&
					std::is_nothrow_destructible_v<PA>;
			}

			static constexpr bool public_interface_player_action() noexcept
			{
				using PA = PlayerAction;

				constexpr bool get_id = noexcept(std::declval<const PA&>().get_player_id());
				constexpr bool get_type = noexcept(std::declval<const PA&>().get_type());
				constexpr bool get_time = noexcept(std::declval<const PA&>().get_time_stamp());

				return get_id && get_type && get_time;
			}

			static constexpr bool constructible_toptracker() noexcept
			{
				using TT = TopTracker;
				using Sec = std::chrono::seconds;
				return std::is_nothrow_constructible_v<TT, Sec, std::size_t>;
			}

			static constexpr bool public_interface_toptracker() noexcept
			{
				using TT = TopTracker;
				using PA = PlayerAction;

				constexpr bool on_act = noexcept(std::declval<TT&>().on_action(0, PA::Type::BUY));
				constexpr bool del_old = noexcept(std::declval<TT&>().delete_old_actions());
				constexpr bool view = noexcept(std::declval<const TT&>().get_actions_view());
				constexpr bool copy = noexcept(std::declval<const TT&>().get_actions_copy());

				return on_act && del_old && view && copy;
			}
		}
	}

	namespace runtime
	{
		static bool basic_insertion()
		{
			TopTracker tracker(std::chrono::seconds{10}, 5);

			tracker.on_action(1, PlayerAction::Type::BUY);
			tracker.on_action(2, PlayerAction::Type::SELL);

			const auto& actions = tracker.get_actions_view();

			return actions.size() == 2 &&
				actions[0].get_player_id() == 1 &&
				actions[1].get_player_id() == 2;
		}

		static bool capacity_limit()
		{
			TopTracker tracker(std::chrono::seconds{10}, 3);

			tracker.on_action(1, PlayerAction::Type::BUY);
			tracker.on_action(2, PlayerAction::Type::SELL);
			tracker.on_action(3, PlayerAction::Type::WIN);
			tracker.on_action(4, PlayerAction::Type::LOSE); // первый должен быть вытеснен

			const auto& actions = tracker.get_actions_view();

			return actions.size() == 3 &&
				actions[0].get_player_id() == 2 &&
				actions[1].get_player_id() == 3 &&
				actions[2].get_player_id() == 4;
		}

		static bool timeout_cleanup()
		{
			using namespace std::chrono_literals;
			TopTracker tracker(2s, 10);

			tracker.on_action(10, PlayerAction::Type::BUY);
			std::this_thread::sleep_for(3s);
			tracker.on_action(11, PlayerAction::Type::SELL);

			tracker.delete_old_actions();

			const auto& actions = tracker.get_actions_view();
			return actions.size() == 1 && actions[0].get_player_id() == 11;
		}

		static bool action_ordering()
		{
			TopTracker tracker(std::chrono::seconds{10}, 5);

			tracker.on_action(1, PlayerAction::Type::WIN);
			tracker.on_action(2, PlayerAction::Type::LOSE);
			tracker.on_action(3, PlayerAction::Type::SELL);

			const auto v = tracker.get_actions_copy();

			return v.size() == 3 &&
				v[0].get_player_id() == 1 &&
				v[1].get_player_id() == 2 &&
				v[2].get_player_id() == 3;
		}

		static bool copy_interface()
		{
			TopTracker tracker(std::chrono::seconds{10}, 2);
			tracker.on_action(7, PlayerAction::Type::SELL);
			tracker.on_action(8, PlayerAction::Type::WIN);

			const auto copy = tracker.get_actions_copy();

			return copy.size() == 2 &&
				copy[0].get_player_id() == 7 &&
				copy[1].get_player_id() == 8;
		}

		static bool test_monotonic_timestamps()
		{
			TopTracker tracker(std::chrono::seconds{10}, 10);
			for (int i = 0; i < 5; ++i)
			{
				tracker.on_action(i, PlayerAction::Type::BUY);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			const auto& v = tracker.get_actions_view();
			for (size_t i = 1; i < v.size(); ++i)
			{
				if (v[i].get_time_stamp() < v[i - 1].get_time_stamp())
					return false;
			}
			return true;
		}

		static bool test_duplicate_actions_allowed()
		{
			TopTracker tracker(std::chrono::seconds{5}, 10);
			tracker.on_action(42, PlayerAction::Type::SELL);
			tracker.on_action(42, PlayerAction::Type::SELL);

			const auto& v = tracker.get_actions_view();
			return v.size() == 2 &&
				v[0].get_player_id() == 42 &&
				v[1].get_type() == PlayerAction::Type::SELL;
		}

		static bool test_delete_old_actions_keeps_fresh()
		{
			using namespace std::chrono_literals;
			TopTracker tracker(1s, 10);

			tracker.on_action(1, PlayerAction::Type::WIN);
			std::this_thread::sleep_for(2s);
			tracker.on_action(2, PlayerAction::Type::LOSE);

			tracker.delete_old_actions();

			const auto& v = tracker.get_actions_view();
			return v.size() == 1 && v[0].get_player_id() == 2;
		}

		static bool test_get_copy_equals_view()
		{
			TopTracker tracker(std::chrono::seconds{10}, 5);
			tracker.on_action(1, PlayerAction::Type::BUY);
			tracker.on_action(2, PlayerAction::Type::SELL);

			auto copy = tracker.get_actions_copy();
			const auto& view = tracker.get_actions_view();

			if (copy.size() != view.size()) return false;

			for (size_t i = 0; i < copy.size(); ++i)
			{
				if (copy[i].get_player_id() != view[i].get_player_id())
					return false;
			}
			return true;
		}
	}
}

int main()
{
	using namespace test::stream_output;

	std::clog << ">>> Starts compiletime-noexcept tests <<<\n";
	{
		using namespace test::compiletime::is_noexcept;

		if (constructible_player_action())
			print_test_passed("PlayerAction is noexcept-constructible");
		else
			print_test_failed("PlayerAction is NOT noexcept-constructible");

		if (copyable_player_action())
			print_test_passed("PlayerAction is fully noexcept-copyable and destructible");
		else
			print_test_failed("PlayerAction is NOT fully noexcept-copyable or destructible");

		if (public_interface_player_action())
			print_test_passed("PlayerAction public interface methods are noexcept");
		else
			print_test_failed("PlayerAction public interface methods are NOT noexcept");

		if (constructible_toptracker())
			print_test_passed("TopTracker is noexcept-constructible");
		else
			print_test_failed("TopTracker is NOT noexcept-constructible");

		if (public_interface_toptracker())
			print_test_passed("TopTracker public interface methods are noexcept");
		else
			print_test_failed("TopTracker public interface methods are NOT noexcept");
	}
	std::clog << ">>> Ends compiletime-noexcept tests <<<\n";

	std::clog << ">>> Starts runtime tests <<<\n";
	{
		using namespace test::runtime;

		if (basic_insertion())
			print_test_passed("Basic insertion works");
		else
			print_test_failed("Basic insertion failed");

		if (capacity_limit())
			print_test_passed("Capacity limit and pop_front logic works");
		else
			print_test_failed("Capacity limit logic failed");
		
		if (timeout_cleanup())
			print_test_passed("Timeout cleanup works correctly");
		else
			print_test_failed("Timeout cleanup failed");

		if (action_ordering())
			print_test_passed("Action ordering is preserved");
		else
			print_test_failed("Action ordering is broken");

		if (copy_interface())
			print_test_passed("Copy interface returns correct data");
		else
			print_test_failed("Copy interface returned incorrect result");

		if (test_duplicate_actions_allowed())
			print_test_passed("Duplicated actions are saving");
		else
			print_test_failed("Duplicated actions are not saving");

		if (test_delete_old_actions_keeps_fresh())
			print_test_passed("Deletion and detection of expired actions are correct");
		else
			print_test_failed("Deletion and detection of expired actions are not correct");

		if (test_get_copy_equals_view())
			print_test_passed("Copy and view getters return same actions");
		else
			print_test_failed("Copy and view getters return different actions");
	}
	std::clog << ">>> Ends runtime tests <<<\n";


	return 0;
}