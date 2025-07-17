
#pragma once

#include "PlayerAction.h"

#include <deque>
#include <mutex>

class TopTracker final
{
public:
	TopTracker() = delete;
	TopTracker(std::chrono::seconds timeout, std::size_t actions_max_count) noexcept;
	
	void on_action(PlayerAction::PlayerId player_id, PlayerAction::Type action_type) noexcept(std::is_nothrow_constructible_v<PlayerAction, PlayerAction::PlayerId, PlayerAction::Type>);
	void delete_old_actions() noexcept(noexcept(std::declval<PlayerAction>().get_time_stamp()));
	[[nodiscard("Pure method")]] const std::deque<PlayerAction>& get_actions_view() const noexcept;
	[[nodiscard("Pure method")]] std::vector<PlayerAction> get_actions_copy() const noexcept(std::is_nothrow_copy_constructible_v<PlayerAction>);
	
private:
	std::deque<PlayerAction> actions;
	std::chrono::seconds timeout;
	std::size_t actions_max_count;
	mutable std::mutex mtx;
};