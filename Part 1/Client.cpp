
#include "Client.h"

#include "Log.h"
#include "LoginData.h"

Client::Client(IO *io):
	io(io)
{
	this->requests = std::make_shared<Requests>(this);
}

Client::~Client() = default;

auto Client::get_ip() const -> IP
{
	return this->io->get_ip();
}

void Client::disconnect() const
{
	this->io->stop();
}

void Client::send(const server::Packet &packet) const
{
	io->write(packet);
}

void Client::on_event(const ClientEvent &event)
{
	// Пример обработки событий клиента
	switch (event.get_type())
	{
		case ClientEvent::Type::DISCONNECT:
			this->disconnect();
			break;
		default:
			logger->warning("Unhandled event type {}", event.get_type());
	}
}

void Client::on_packet(const server::Packet &packet)
{
	switch (packet.get_type())
	{
		case server::PacketType::PARAMS_SET:
			this->params_set(packet);
			break;
		case server::PacketType::BUY:
			this->buy(packet);
			break;
		case server::PacketType::LOGIN:
			this->login(packet);
			break;
		default:
			logger->warning("Unhandled packet type {}", packet.get_type());
			break;
	}
}

void Client::params_set(const server::Packet &packet) const
{
	string params = packet.S(0);
	if (params.empty())
		return;

	this->player->params->set(params);

	logger->info("Client {} params set", this->player->id);
}

void Client::buy(const server::Packet &packet)
{
	uint32_t item_id = packet.I(0);

	if (!this->player->balance->can_afford(item_id))
	{
		logger->warning("Client {} can't afford item {}", this->player->id, item_id);
		return;
	}

	this->player->balance->deduct(item_id);
	this->player->inventory->add(item_id);

	logger->info("Client {} bought item {}", this->player->id, item_id);
}

void Client::login(const client::Packet *packet)
{
	uint64_t net_id = packet->L(0);
	uint8_t net_type = packet->B(1);
	const string &auth_key = packet->S(3);

	if (net_type >= NetType::MAX_TYPE)
	{
		logger->warning("Player net_id {} sent wrong net_type {}", net_id, net_type);
		this->send(server::Login(server::Login::Status::FAILED));
		return;
	}

	if (!Api::check_auth(net_id, net_type, auth_key))
	{
		logger->debug("Player net_id {}, net_type {} sent wrong auth_key '{}'", net_id, net_type, auth_key);
		this->send(server::Login(server::Login::Status::FAILED));
		return;
	}

	LoginData data;
	data.net_id = net_id;
	data.net_type = net_type;

	this->requests->add(&data, [&](const vector<Player*> &loaded) -> void
	{
		this->login_do(loaded[0], &data);
	});
}