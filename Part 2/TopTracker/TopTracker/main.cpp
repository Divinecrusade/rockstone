
#include <iostream>

#include "PlayerAction.h"

int main()
{
	PlayerAction a(3, PlayerAction::Type::BUY);
	std::clog << a.get_time_stamp().time_since_epoch() << "\n";
	std::clog << a.get_player_id() << "\n";
	std::clog << static_cast<int>(a.get_type()) << "\n";

	PlayerAction b(a);
	std::clog << a.get_time_stamp().time_since_epoch() << "\n";
	std::clog << a.get_player_id() << "\n";
	std::clog << static_cast<int>(a.get_type()) << "\n";

	PlayerAction c(32, PlayerAction::Type::LOSE);
	std::clog << c.get_time_stamp().time_since_epoch() << "\n";
	std::clog << c.get_player_id() << "\n";
	std::clog << static_cast<int>(c.get_type()) << "\n";

	return 0;
}