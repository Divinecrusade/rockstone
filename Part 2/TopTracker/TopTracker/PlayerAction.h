
#pragma once

#include <chrono>
#include <concepts>

class PlayerAction final
{
public:
	enum class Type
	{
		BUY, SELL, WIN, LOSE,
	};

public:
	using PlayerId = uint64_t;
	using Clock = std::chrono::steady_clock;
	using TimeStamp = Clock::time_point;

public:
	static_assert(std::is_nothrow_move_constructible_v<PlayerId>);
	PlayerAction(PlayerId player_id, Type type) noexcept(std::is_nothrow_copy_constructible_v<PlayerId>);

	[[nodiscard("Pure method")]] const PlayerId& get_player_id() const noexcept;
	[[nodiscard("Pure method")]] const Type& get_type() const noexcept;
	[[nodiscard("Pure method")]] const TimeStamp& get_time_stamp() const noexcept;

private:
	PlayerId player_id;
	Type type;
	TimeStamp time_stamp = Clock::now();
};