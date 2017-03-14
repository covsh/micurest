/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * network_esp8266.hpp - session layer definitions for ESP8266
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The µcuREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */


#include <cstddef>
#include <stdarg.h>
#include "network.hpp"
#include "miculog.hpp"
extern "C" {
#include "ets_sys.h"
#include "ip_addr.h"
#include "espconn.h"
#include "user.h"
}

namespace micurest {
namespace network_esp {
using namespace micurest::network;


using cojson::char_t;
using cojson::details::noncopyable;
using cojson::details::istream;
using cojson::details::ostream;
using cojson::details::error_t;

/*****************************************************************************/
namespace proto {

class tcp {
public:
	static constexpr uint16_t timeout = 60;
	static constexpr size_t mtu_size = 1400;
	inline tcp(const application* a) noexcept : app(a) {}

	struct connection {
		inline connection(tcp& sock) noexcept : socket(sock) {}
		bool accept() noexcept { return true; }
	private:
		tcp& socket;
	};

	bool listen(port_t p) noexcept;
	virtual void close() noexcept;


protected:
	/** espconn callbacks */
	struct on {
		static void connect(void *arg) noexcept;
		static void received(void *arg, char *data, unsigned short len) noexcept;
		static void error(void *arg, sint8 err) noexcept;
		static void disconnect(void *arg) noexcept;
		static void sent(void *arg) noexcept;
		static void sending(void *arg) noexcept;
	};
	virtual void send(const void* data, size_t size) noexcept;
	virtual void receive(const void* data, size_t len) noexcept;
	virtual bool accept() noexcept;
	void keep() noexcept;
	void nokeep() noexcept;

protected:
	espconn con;
	esp_tcp tpc;
	const application* const app;
};

class tlsbase : public tcp {
public:
	inline tlsbase(const application* a,
		const uint8* c, uint16 cl, const uint8* k, uint16 kl) noexcept
	  : tcp(a), cert(c), cert_len(cl), key(k), key_len(kl) {}
	void send(const char* data, size_t size) noexcept;
	void receive(const char* data, size_t len) noexcept;
	bool accept() noexcept;
	void close() noexcept;
private:
	const uint8* const cert;
	const uint16 cert_len;
	const uint8* const key;
	const uint16 key_len;
};


template<class C>
class tls : public tlsbase {
public:
	inline tls(const application* a) noexcept :
	tlsbase(a, C::cert, sizeof(C::cert), C::key, sizeof(C::key)) {}
};
}}}
