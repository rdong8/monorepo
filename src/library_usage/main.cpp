#include <boost/cobalt/channel.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/cobalt/promise.hpp>
#include <quill/LogMacros.h>

import std;

import quill;

import library_usage.mathematics;

namespace
{

[[maybe_unused]]
auto configure_logger() -> quill::Logger *
{
    quill::Backend::start();

    auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console");

    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
        "library_usage.log",
        [] static
        {
            quill::FileSinkConfig config{};
            config.set_open_mode('w');
            config.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
            return config;
        }(),
        quill::FileEventNotifier{});

    return quill::Frontend::create_or_get_logger("root", {std::move(console_sink), std::move(file_sink)});
}

auto producer(quill::Logger *logger, boost::cobalt::channel<int> &channel) -> boost::cobalt::promise<void>
{
    for (auto const i : std::views::iota(0, 5))
    {
        QUILL_LOG_INFO(logger, "Producing {}", i);
        co_await channel.write(i);
    }

    channel.close();
}

auto cobalt_demo(quill::Logger *logger) -> boost::cobalt::promise<void>
{
    QUILL_LOG_INFO(logger, "COBALT DEMO:");

    boost::cobalt::channel<int> channel{};

    auto promise = producer(logger, channel);

    while (channel.is_open())
    {
        QUILL_LOG_INFO(logger, "Consumer received {}", co_await channel.read());
    }

    // Force the producer to finish
    co_await promise;
}

auto vec_demo(quill::Logger *logger) -> void
{
    QUILL_LOG_INFO(logger, "VECTOR DEMO:");

    math::Vec<2> const north{0., 1.};
    math::Vec<2> const east{1., 0.};
    math::Vec<2> const northeast{1., 1.};

    QUILL_LOG_INFO(logger, "Dot product of {} and {} is: {}", north, east, north.dot(east));
    QUILL_LOG_INFO(logger, "Dot product of {} and {} is: {}", north, northeast, north.dot(northeast));

    QUILL_LOG_INFO(logger, "Norm of {} is: {}", northeast, northeast.norm());

    QUILL_LOG_INFO(logger, "Is {} < {}? {}", east, northeast, east < northeast);
}

auto differentiation_demo(quill::Logger *logger) -> void
{
    using math::d_dx;

    QUILL_LOG_INFO(logger, "DIFFERENTIATION DEMO:");

    // NOLINTBEGIN(*-magic-numbers)
    auto constexpr F = [](double x) static { return 3 * x * x - x + 16; };
    auto constexpr DF_DX = d_dx<F>;

    QUILL_LOG_INFO(logger, "f(x) = 3x^2 - x + 16");
    QUILL_LOG_INFO(logger, "f'(4) = {}", DF_DX(4.0));
    QUILL_LOG_INFO(logger, "f''(4) = {}", d_dx<DF_DX>(4.0));
    // NOLINTEND(*-magic-numbers)
}

} // namespace

auto co_main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> boost::cobalt::main
{
    auto *logger = configure_logger();

    co_await cobalt_demo(logger);

    vec_demo(logger);

    differentiation_demo(logger);

    co_return 0;
}
