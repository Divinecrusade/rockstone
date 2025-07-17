
#include "TopTracker.h"

#include <cassert>
#include <ranges>
#include <algorithm>

TopTracker::TopTracker(std::chrono::seconds timeout, std::size_t actions_max_count) noexcept
	: timeout(timeout), actions_max_count(actions_max_count)
{
	assert(("Argument 'actions_max_count' in constructor of TopTracker must not be zero", actions_max_count > 0));
}

void TopTracker::on_action(PlayerAction::PlayerId player_id, PlayerAction::Type action_type) noexcept(std::is_nothrow_constructible_v<PlayerAction, PlayerAction::PlayerId, PlayerAction::Type>)
{
	assert(("Member 'actions_max_count' in TopTracker must not be zero", this->actions_max_count > 0));

	if (this->actions.size() == this->actions_max_count)
	{
		this->actions.pop_front();
	}
	this->actions.emplace_back(std::move(player_id), std::move(action_type));
}

void TopTracker::delete_old_actions() noexcept(noexcept(std::declval<PlayerAction>().get_time_stamp()))
{
#ifdef _DEBUG
	bool const actions_sorted = std::ranges::is_sorted(this->actions, std::less<>(), &PlayerAction::get_time_stamp);
#endif // _DEBUG
	assert(("Member 'actions' in TopTracker must be auto-sorted by time_stamp", actions_sorted));

	const PlayerAction::TimeStamp expiration_timepoint = PlayerAction::Clock::now() - this->timeout;

	const auto erase_to = std::ranges::lower_bound(
		this->actions,
		expiration_timepoint,
		std::less<>(),
		&PlayerAction::get_time_stamp
	);

	this->actions.erase(this->actions.begin(), erase_to);
}

const std::deque<PlayerAction>& TopTracker::get_actions_view() const noexcept
{
	return this->actions;
}

std::vector<PlayerAction> TopTracker::get_actions_copy() const noexcept(std::is_nothrow_copy_constructible_v<PlayerAction>)
{
	return this->actions | std::ranges::to<std::vector>();
}
