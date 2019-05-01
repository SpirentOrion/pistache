/* endpoint.cc
   Mathieu Stefani, 22 janvier 2016

   Implementation of the http endpoint
*/


#include <pistache/endpoint.h>
#include <pistache/tcp.h>
#include <pistache/peer.h>

namespace Pistache {
namespace Http {

Endpoint::Options::Options()
    : threads_(1)
    , flags_()
    , backlog_(Const::MaxBacklog)
    , maxPayload_(Const::DefaultMaxPayload)
{ }

Endpoint::Options&
Endpoint::Options::threads(int val) {
    threads_ = val;
    return *this;
}

Endpoint::Options&
Endpoint::Options::flags(Flags<Tcp::Options> flags) {
    flags_ = flags;
    return *this;
}

Endpoint::Options&
Endpoint::Options::backlog(int val) {
    backlog_ = val;
    return *this;
}

Endpoint::Options&
Endpoint::Options::maxPayload(size_t val) {
    maxPayload_ = val;
    return *this;
}

Endpoint::Endpoint()
{ }

Endpoint::Endpoint(const Address& addr)
    : listener(addr)
{ }

void
Endpoint::init(const Endpoint::Options& options) {
    listener.init(options.threads_, options.flags_);
    ArrayStreamBuf<char>::maxSize = options.maxPayload_;
}

void
Endpoint::setHandler(const std::shared_ptr<Handler>& handler) {
    handler_ = handler;
}

void
Endpoint::bind() {
    listener.bind();
}

void
Endpoint::bind(const Address& addr) {
    listener.bind(addr);
}

void
Endpoint::serve()
{
    std::promise<void> promise;
    serve(promise);
}

void
Endpoint::serveThreaded()
{
    std::promise<void> promise;
    auto future = promise.get_future();

    serveThreaded(promise);

    auto status = future.wait_for(std::chrono::seconds(Const::MaxServeStartTime));
    switch(status) {
        case std::future_status::ready:
            break;
        case std::future_status::timeout:
            throw std::runtime_error("Endpoint server failed to start within timeout period.");
            break;
        case std::future_status::deferred:
            break;
    }
}

void
Endpoint::serve(std::promise<void>& promise)
{
    serveImpl(&Tcp::Listener::run, promise);
}

void
Endpoint::serveThreaded(std::promise<void>& promise)
{
    serveImpl(&Tcp::Listener::runThreaded, promise);
}

void
Endpoint::shutdown()
{
    listener.shutdown();
}

Async::Promise<Tcp::Listener::Load>
Endpoint::requestLoad(const Tcp::Listener::Load& old) {
    return listener.requestLoad(old);
}

Endpoint::Options
Endpoint::options() {
    return Options();
}

} // namespace Http
} // namespace Pistache
