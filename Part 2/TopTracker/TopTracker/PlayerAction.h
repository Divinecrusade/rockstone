
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
	// noexcept гарантии выставлены с учётом того, что PlayerId - простой тип (стандартный целочисленный)
	// если он будет заменён, то следует пересмотреть noexcept (для этого здесь и указан static_assert)
	static_assert(std::is_nothrow_copy_constructible_v<PlayerId>);

	PlayerAction(PlayerId player_id, Type type) noexcept;

	[[nodiscard]] PlayerId get_player_id() const noexcept;
	[[nodiscard]] Type get_type() const noexcept;
	[[nodiscard]] const TimeStamp& get_time_stamp() const noexcept;

private:
	PlayerId player_id;
	Type type;
	TimeStamp time_stamp;
};