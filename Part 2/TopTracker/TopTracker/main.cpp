
#include <iostream>
#include <thread>
#include <ranges>
#include <algorithm>

#include "TopTracker.h"

using namespace std::chrono_literals;

int main()
{
	TopTracker t(5s, 1000ull);

	t.on_action(1, PlayerAction::Type::BUY);
	t.on_action(2, PlayerAction::Type::SELL);

	std::this_thread::sleep_for(2s);

	t.on_action(3, PlayerAction::Type::BUY);
	t.on_action(2, PlayerAction::Type::WIN);

	for (const auto& a : t.get_actions_copy())
	{
		std::clog << a.get_player_id() << " " << static_cast<int>(a.get_type()) << " " << a.get_time_stamp().time_since_epoch() << "\n";
	}

	for (const auto& a : t.get_actions_view())
	{
		std::clog << a.get_player_id() << " " << static_cast<int>(a.get_type()) << " " << a.get_time_stamp().time_since_epoch() << "\n";
	}

	t.on_action(4, PlayerAction::Type::WIN);
	std::this_thread::sleep_for(3s);
	t.delete_old_actions();

	for (const auto& a : t.get_actions_view())
	{
		std::clog << a.get_player_id() << " " << static_cast<int>(a.get_type()) << " " << a.get_time_stamp().time_since_epoch() << "\n";
	}

	return 0;
}