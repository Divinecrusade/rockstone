
#include <iostream>
#include <string_view>

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

			static constexpr bool copyable_toptracker() noexcept
			{
				using TT = TopTracker;
				return std::is_nothrow_copy_constructible_v<TT> &&
					std::is_nothrow_move_constructible_v<TT> &&
					std::is_nothrow_copy_assignable_v<TT> &&
					std::is_nothrow_move_assignable_v<TT> &&
					std::is_nothrow_destructible_v<TT>;
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
}

int main()
{
	using namespace test::stream_output;
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

	if (copyable_toptracker())
		print_test_passed("TopTracker is fully noexcept-copyable and destructible");
	else 
		print_test_failed("TopTracker is NOT fully noexcept-copyable or destructible");

	if (public_interface_toptracker())
		print_test_passed("TopTracker public interface methods are noexcept");
	else
		print_test_failed("TopTracker public interface methods are NOT noexcept");

	return 0;
}