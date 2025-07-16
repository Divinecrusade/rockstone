# Часть 1. Код-ревью
--- 

## Условие

**Дано:** фрагмент исходного кода сервера (файл `Client.cpp`).

**Задание:**
* Найти и кратко описать (в 2–3 предложениях) **три участка кода**, которые можно улучшить с точки зрения читаемости, архитектуры или производительности.
* Предложить и реализовать улучшения в **одном** из этих участков кода – в виде патча или отдельного нового файла с исправлениями.

---

## Решение

### 1-й участок кода. Потенциальное нарушение инварианта класса `Client::io != nullptr`

Методы `Client::get_ip() const`, `Client::disconnect() const`, `Client::send(const server::Packet &packet)` используют селектор членов класса (оператор `->`, или оператор указателя на член класса). К примеру тело метода `Client::get_ip() const` выглядит следующим образом:
```C++
auto Client::get_ip() const -> IP
{
	return this->io->get_ip();
}
```
Где инициализация члена `Client::io` происходит в конструкторе:
```C++
Client::Client(IO *io):
	io(io)
{
	this->requests = std::make_shared<Requests>(this);
}
```
Следовательно, нет синтаксической гарантии, что `Client::io` никогда не будет инициализирован `nullptr`. Поэтому следующий код приведёт к возникновению исключения:
```C++
Client cur_client{nullptr};

const auto cur_ip{cur_client.get_ip()}; // runtime error, pointer is null
```

#### 1-й подход. Игнорирование

Если архитектура приложения гарантирует, что аргумент `io` в конструкторе никогда не станет `nullptr`, то можно не вносить изменений.

Однако такое решение противоречит с одной стороны ООП, поскольку образует "неявный контракт" между пользователем и сервисом, а с другой нарушает guideline C++, т.к. синтаксически не выражает намерение программиста. Мы предполагаем, что пользователи класса `Client` "знают" о том, что `io` не должен быть `nullptr`. Это усложнение семантики класса можно допустить только при очень существенных обстоятельствах.

#### 2-й подход. Добавление утверждений

Допустим, текущая логика инварианта правильная, в таком случае стоит добавить утверждение в конструкторе класса и методах, использующих проблемный указатель:
```C++
#include <cassert>

#define ASSERT_CLIENT_IO_IS_NOT_NULL assert(this->io != nullptr);

Client::Client(IO *io):
	io(io)
{
	ASSERT_CLIENT_IO_IS_NOT_NULL

	this->requests = std::make_shared<Requests>(this);
}

auto Client::get_ip() const -> IP
{
	ASSERT_CLIENT_IO_IS_NOT_NULL

	return this->io->get_ip();
}

void Client::disconnect() const
{
	ASSERT_CLIENT_IO_IS_NOT_NULL

	this->io->stop();
}

void Client::send(const server::Packet &packet) const
{
	ASSERT_CLIENT_IO_IS_NOT_NULL

	io->write(packet);
}
```

Метод `assert` реализован так, что вызывает завершение программы при истинности выражения только при тогда, когда поднят флаг DEBUG. Соответственно, в релизной версии все вызовы функции `assert` будут опущены.

Это позволяет отловить во время тестирования возможные разыменования пустого указателя и при этом не повлияет на итоговую производительность.

#### 3-й подход. Использование `gsl::not_null<IO*>`

Существует (используемая в том числе Microsoft) библиотека [Guidelines Support Library для C++](https://github.com/Microsoft/GSL), которая немного расширяет функционал C++, добавляя новые возможности для синтаксических проверок ограничений, накладываемых либо предметной областью, либо архитектурой ПО. Она свободно распространяема и состоит только из заголовочных файлов.

Используя GSL можно явно наложить требование на аргумент `io` в конструкторе:

```C++
#include <gsl/gsl>

Client::Client(gsl::not_null<IO*> io):
	io(io)
{
	this->requests = std::make_shared<Requests>(this);
}
```

Это добавляет проверку времени исполнения при инициализации фактического параметра. Если `io == nullptr`, то будет вызвано немедленное завершение работы программы (`std::terminate`).

Такой подход лучше использования утверждений, поскольку он накладывает ограничение на уровне интерфейса (объявлении конструктора).

### 2-й участок кода. Разыменование пустого указателя

Указатель `Player*` `Client::player` по умолчанию равен `nullptr`, при этом в конструкторе его значение не изменяется (я предполагаю, что значение этого поля меняют методы, которые не приведены в этом файле). 
Это значит, что, нет гарантии не `nullptr` значений при попытки выполнения, например, следующих операций в методе `Client::buy(const server::Packet &packet)`:
```C++
this->player->balance->deduct(item_id);	// потенциальная попытка разыменование nullptr
this->player->inventory->add(item_id);	// потенциальная попытка разыменования nullptr
```

Логика методов, использующих этот указатель, не позволяет добавить if-guard (проверку на nullptr) без изменения семантики методов. Следовательно, эти методы **должны быть** вызваны после присваивания `Client::player` действующего адреса. Наиболее разумным выходом из положения без перепроектирования класса является добавление утверждений:

```C++
#include <cassert>

#define ASSERT_CLIENT_PLAYER_IS_NOT_NULL assert(this->player != nullptr);

void Client::params_set(const server::Packet &packet) const
{
	ASSERT_CLIENT_PLAYER_IS_NOT_NULL

	string params = packet.S(0);
	if (params.empty())
		return;

	this->player->params->set(params);

	logger->info("Client {} params set", this->player->id);
}

void Client::buy(const server::Packet &packet)
{
	ASSERT_CLIENT_PLAYER_IS_NOT_NULL

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
```

### 3-й участок кода. Выход за пределы контейнера (исправлен патчем)

*TODO: ссылка на коммит*

В методе `Client::login(const client::Packet *packet)` передается лямбда-объект:
```C++
this->requests->add(&data, [&](const vector<Player*> &loaded) -> void
{
	this->login_do(loaded[0], &data); // vector: out of range (если loaded.empty() == true)
});
```
Проблема в том, что нет синтаксической гарантии непустоты параметра `const vector<Player*> &loaded`, в результате чего `loaded[0]` может привести к возникновению исключения.

Необходима дополнительная проверка на пустоту, и в случае успеха `loaded.empty() == true` возврат из метода лямбды без попытки выполнить операцию `this->login_do(loaded[0], &data)`. 

На мой взгляд, эта самая опасная проблема, поэтому я её и решил исправить. По сравнению с предыдущими участками, здесь наиболее вероятно возникновение ошибки. Во-первых, неизвестен контекст вызова метода, во-вторых, тело метода никак не намекает на состояние аргумента метода лямбда-объекта.

### Прочие предложения по улучшению

- **Rule of zero:** правило позволяет убрать явное определение  `~Client() = default` (при чём стоит добавить `Client() = delete` или просто обернуть  `IO* Client::io` в `gsl::not_null<IO*> Client::io`)
- **noexcept:** следует отметить методы, никогда не выбрасывающие исключение (`Client::disconnect() const`, `Client::send(const server::Packet &packet) const`, `Client::get_ip() const`)
- **магические числа:** числа в выражених `packet.S(0)`, `packet.I(0)`, `packet->L(0)` и пр. рекомендуется заменить на константы времени компиляции (`constexpr`) или (лучше всего) перечисления
- **шаблон логирования:** по коду часто встречаются строки, одинаково начинающиеся, например, с `"Client {}"`. Их можно заменить на именованную константу (`constexpr std::string_view`) и/или метод, собирающий строку для логирования