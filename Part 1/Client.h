
#pragma once

#include "IO.h"
#include "Requests.h"
#include "Player.h"
#include "ClientEvent.h"

#include "server/Packet.h"

#include <memory>
#include <string>

class Client final
{
public:
	explicit Client(IO *io);
	~Client();

	void disconnect() const;
	void send(const server::Packet &packet) const;
	void on_event(const ClientEvent &event);

private:
	IO *io;
	std::shared_ptr<Requests> requests;
	Player *player = nullptr;

	[[nodiscard]] auto get_ip() const -> IP;

	void on_packet(const server::Packet &packet);

	// Примеры обработчиков пакетов
	void params_set(const server::Packet &packet) const;
	void buy(const server::Packet &packet);
	void login(const server::Packet &packet);
};