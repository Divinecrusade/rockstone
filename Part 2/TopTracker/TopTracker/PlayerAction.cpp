
#include "PlayerAction.h"

PlayerAction::PlayerAction(PlayerId player_id, Type type) noexcept(std::is_nothrow_copy_constructible_v<PlayerId>)
	: player_id(player_id), type(type)
{}

const PlayerAction::PlayerId& PlayerAction::get_player_id() const noexcept
{
	return this->player_id;
}

const PlayerAction::Type& PlayerAction::get_type() const noexcept
{
	return this->type;
}

const PlayerAction::TimeStamp& PlayerAction::get_time_stamp() const noexcept
{
	return this->time_stamp;
}
