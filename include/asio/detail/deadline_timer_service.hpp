//
// detail/deadline_timer_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_DEADLINE_TIMER_SERVICE_HPP
#define ASIO_DETAIL_DEADLINE_TIMER_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <cstddef>
#include "asio/associated_cancellation_slot.hpp"
#include "asio/cancellation_type.hpp"
#include "asio/config.hpp"
#include "asio/error.hpp"
#include "asio/execution_context.hpp"
#include "asio/detail/bind_handler.hpp"
#include "asio/detail/fenced_block.hpp"
#include "asio/detail/memory.hpp"
#include "asio/detail/noncopyable.hpp"
#include "asio/detail/socket_ops.hpp"
#include "asio/detail/socket_types.hpp"
#include "asio/detail/timer_queue.hpp"
#include "asio/detail/timer_queue_ptime.hpp"
#include "asio/detail/timer_scheduler.hpp"
#include "asio/detail/wait_handler.hpp"
#include "asio/detail/wait_op.hpp"

#if defined(ASIO_WINDOWS_RUNTIME)
# include <chrono>
# include <thread>
#endif // defined(ASIO_WINDOWS_RUNTIME)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename TimeTraits>
class deadline_timer_service
  : public execution_context_service_base<deadline_timer_service<TimeTraits>>
{
public:
  // The time type.
  typedef typename TimeTraits::time_type time_type;

  // The duration type.
  typedef typename TimeTraits::duration_type duration_type;

  // The allocator type.
  typedef execution_context::allocator<void> allocator_type;

  // The implementation type of the timer. This type is dependent on the
  // underlying implementation of the timer service.
  struct implementation_type
    : private asio::detail::noncopyable
  {
    time_type expiry;
    bool might_have_pending_waits;
    typename timer_queue<TimeTraits, allocator_type>::per_timer_data timer_data;
  };

  // Constructor.
  deadline_timer_service(execution_context& context)
    : execution_context_service_base<
        deadline_timer_service<TimeTraits>>(context),
      timer_queue_(allocator_type(context),
          config(context).get("timer", "heap_reserve", 0U)),
      scheduler_(asio::use_service<timer_scheduler>(context))
  {
    scheduler_.init_task();
    scheduler_.add_timer_queue(timer_queue_);
  }

  // Destructor.
  ~deadline_timer_service()
  {
    scheduler_.remove_timer_queue(timer_queue_);
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown()
  {
  }

  // Construct a new timer implementation.
  void construct(implementation_type& impl)
  {
    impl.expiry = time_type();
    impl.might_have_pending_waits = false;
  }

  // Destroy a timer implementation.
  void destroy(implementation_type& impl)
  {
    asio::error_code ec;
    cancel(impl, ec);
  }

  // Move-construct a new timer implementation.
  void move_construct(implementation_type& impl,
      implementation_type& other_impl)
  {
    if (other_impl.might_have_pending_waits)
    {
      scheduler_.move_timer(timer_queue_,
          impl.timer_data, other_impl.timer_data);
    }

    impl.expiry = other_impl.expiry;
    other_impl.expiry = time_type();

    impl.might_have_pending_waits = other_impl.might_have_pending_waits;
    other_impl.might_have_pending_waits = false;
  }

  // Move-assign from another timer implementation.
  void move_assign(implementation_type& impl,
      deadline_timer_service& other_service,
      implementation_type& other_impl)
  {
    if (this != &other_service)
      if (impl.might_have_pending_waits)
        scheduler_.cancel_timer(timer_queue_, impl.timer_data);

    other_service.scheduler_.move_timer(other_service.timer_queue_,
        impl.timer_data, other_impl.timer_data);

    impl.expiry = other_impl.expiry;
    other_impl.expiry = time_type();

    impl.might_have_pending_waits = other_impl.might_have_pending_waits;
    other_impl.might_have_pending_waits = false;
  }

  // Move-construct a new timer implementation.
  void converting_move_construct(implementation_type& impl,
      deadline_timer_service&, implementation_type& other_impl)
  {
    move_construct(impl, other_impl);
  }

  // Move-assign from another timer implementation.
  void converting_move_assign(implementation_type& impl,
      deadline_timer_service& other_service,
      implementation_type& other_impl)
  {
    move_assign(impl, other_service, other_impl);
  }

  // Cancel any asynchronous wait operations associated with the timer.
  std::size_t cancel(implementation_type& impl, asio::error_code& ec)
  {
    if (!impl.might_have_pending_waits)
    {
      ec = asio::error_code();
      return 0;
    }

    ASIO_HANDLER_OPERATION((scheduler_.context(),
          "deadline_timer", &impl, 0, "cancel"));

    std::size_t count = scheduler_.cancel_timer(timer_queue_, impl.timer_data);
    impl.might_have_pending_waits = false;
    ec = asio::error_code();
    return count;
  }

  // Cancels one asynchronous wait operation associated with the timer.
  std::size_t cancel_one(implementation_type& impl,
      asio::error_code& ec)
  {
    if (!impl.might_have_pending_waits)
    {
      ec = asio::error_code();
      return 0;
    }

    ASIO_HANDLER_OPERATION((scheduler_.context(),
          "deadline_timer", &impl, 0, "cancel_one"));

    std::size_t count = scheduler_.cancel_timer(
        timer_queue_, impl.timer_data, 1);
    if (count == 0)
      impl.might_have_pending_waits = false;
    ec = asio::error_code();
    return count;
  }

  // Get the expiry time for the timer as an absolute time.
  time_type expiry(const implementation_type& impl) const
  {
    return impl.expiry;
  }

  // Get the expiry time for the timer as an absolute time.
  time_type expires_at(const implementation_type& impl) const
  {
    return impl.expiry;
  }

  // Get the expiry time for the timer relative to now.
  duration_type expires_from_now(const implementation_type& impl) const
  {
    return TimeTraits::subtract(this->expiry(impl), TimeTraits::now());
  }

  // Set the expiry time for the timer as an absolute time.
  std::size_t expires_at(implementation_type& impl,
      const time_type& expiry_time, asio::error_code& ec)
  {
    std::size_t count = cancel(impl, ec);
    impl.expiry = expiry_time;
    ec = asio::error_code();
    return count;
  }

  // Set the expiry time for the timer relative to now.
  std::size_t expires_after(implementation_type& impl,
      const duration_type& expiry_time, asio::error_code& ec)
  {
    return expires_at(impl,
        TimeTraits::add(TimeTraits::now(), expiry_time), ec);
  }

  // Set the expiry time for the timer relative to now.
  std::size_t expires_from_now(implementation_type& impl,
      const duration_type& expiry_time, asio::error_code& ec)
  {
    return expires_at(impl,
        TimeTraits::add(TimeTraits::now(), expiry_time), ec);
  }

  // Perform a blocking wait on the timer.
  void wait(implementation_type& impl, asio::error_code& ec)
  {
    time_type now = TimeTraits::now();
    ec = asio::error_code();
    while (TimeTraits::less_than(now, impl.expiry) && !ec)
    {
      this->do_wait(TimeTraits::to_posix_duration(
            TimeTraits::subtract(impl.expiry, now)), ec);
      now = TimeTraits::now();
    }
  }

  // Start an asynchronous wait on the timer.
  template <typename Handler, typename IoExecutor>
  void async_wait(implementation_type& impl,
      Handler& handler, const IoExecutor& io_ex)
  {
    associated_cancellation_slot_t<Handler> slot
      = asio::get_associated_cancellation_slot(handler);

    // Allocate and construct an operation to wrap the handler.
    typedef wait_handler<Handler, IoExecutor> op;
    typename op::ptr p = { asio::detail::addressof(handler),
      op::ptr::allocate(handler), 0 };
    p.p = new (p.v) op(handler, io_ex);

    // Optionally register for per-operation cancellation.
    if (slot.is_connected())
    {
      p.p->cancellation_key_ =
        &slot.template emplace<op_cancellation>(this, &impl.timer_data);
    }

    impl.might_have_pending_waits = true;

    ASIO_HANDLER_CREATION((scheduler_.context(),
          *p.p, "deadline_timer", &impl, 0, "async_wait"));

    scheduler_.schedule_timer(timer_queue_, impl.expiry, impl.timer_data, p.p);
    p.v = p.p = 0;
  }

private:
  // Helper function to wait given a duration type. The duration type should
  // either be of type boost::posix_time::time_duration, or implement the
  // required subset of its interface.
  template <typename Duration>
  void do_wait(const Duration& timeout, asio::error_code& ec)
  {
#if defined(ASIO_WINDOWS_RUNTIME)
    std::this_thread::sleep_for(
        std::chrono::seconds(timeout.total_seconds())
        + std::chrono::microseconds(timeout.total_microseconds()));
    ec = asio::error_code();
#else // defined(ASIO_WINDOWS_RUNTIME)
    ::timeval tv;
    tv.tv_sec = timeout.total_seconds();
    tv.tv_usec = timeout.total_microseconds() % 1000000;
    socket_ops::select(0, 0, 0, 0, &tv, ec);
#endif // defined(ASIO_WINDOWS_RUNTIME)
  }

  // Helper class used to implement per-operation cancellation.
  class op_cancellation
  {
  public:
    op_cancellation(deadline_timer_service* s,
        typename timer_queue<TimeTraits, allocator_type>::per_timer_data* p)
      : service_(s),
        timer_data_(p)
    {
    }

    void operator()(cancellation_type_t type)
    {
      if (!!(type &
            (cancellation_type::terminal
              | cancellation_type::partial
              | cancellation_type::total)))
      {
        service_->scheduler_.cancel_timer_by_key(
            service_->timer_queue_, timer_data_, this);
      }
    }

  private:
    deadline_timer_service* service_;
    typename timer_queue<TimeTraits, allocator_type>::per_timer_data*
      timer_data_;
  };

  // The queue of timers.
  timer_queue<TimeTraits, allocator_type> timer_queue_;

  // The object that schedules and executes timers. Usually a reactor.
  timer_scheduler& scheduler_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_DEADLINE_TIMER_SERVICE_HPP
