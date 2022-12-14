#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/async_result_detail.h>

#include <functional>
#include <future>
#include <mutex>

/**
 * @defgroup async_ops Asynchronous Operations
 * The Asynchronous Operations group gathers classes and functions responsible
 * for asynchronous operations in the C++ SDK.
 */
namespace dolbyio::comms {

/**
 * @brief Traits that allow for safe implementation of asynchronous classes.
 * They allow public APIs to safely use the objects of these classes.
 * @ingroup async_ops
 */
struct thread_safe_solver_traits {
  /**
   * @brief The lock type of the solver, for the Public API this
   * is a mutex.
   */
  using lock_type = std::mutex;

  /**
   * @brief The lock guard type of the solver, for the Pubcic API
   * this is a lock_guard.
   */
  using lock_guard_type = std::lock_guard<std::mutex>;

  /**
   * @brief The callback type for the callbacks attached to the
   * solver. For the Public API this is a std::function object.
   */
  template <typename T>
  using callback_type = std::function<T>;
};

/**
 * @brief The thread-safe version of detail::async_result.
 * @tparam T The result type that is wrapped by the underlying solver.
 * @ingroup async_ops
 */
template <typename T>
using async_result = detail::async_result<T, thread_safe_solver_traits>;

/**
 * @brief The thread-safe version of the detail::low_level_solver.
 * @tparam T The result type that is wrapped by the underlying solver.
 * @ingroup async_ops
 */
template <typename T>
using low_level_solver = detail::low_level_solver<T, thread_safe_solver_traits>;

/**
 * @brief The thread-safe version of detail::low_level_solver_ptr.
 * @tparam T The result type that is wrapped by the underlying solver.
 * @ingroup async_ops
 */
template <typename T>
using low_level_solver_ptr = detail::low_level_solver_ptr<T, thread_safe_solver_traits>;

/**
 * @brief Partial specialization of the detail::solver template class, ensuring that the operations are
 * thread-safe.
 * @ingroup async_ops
 */
template <typename T>
using solver = dolbyio::comms::detail::solver<T, thread_safe_solver_traits>;

/**
 * @brief Partial specialization of the detail::async_result_with_solver template class, ensuring that the operations are
 * thread-safe.
 * @ingroup async_ops
 */
template <typename T>
using async_result_with_solver = dolbyio::comms::detail::async_result_with_solver<T, thread_safe_solver_traits>;

/**
 * @brief Waits for the asynchronous operation to complete and returns the T
 * type when the operation finishes. This call is synchronous and blocks the
 * calling thread until the result is available. The method either returns an
 * object of type T or throws an exception if the #solver of #async_result
 * fails.
 * @param asyncop The async_result object that wraps a solver that contains the
 * T return type.
 * @tparam T The type of object that is returned asynchronously.
 * @return The object of type T.
 * @exception dolbyio::comms::exception or any subclass of exceptions.
 * @ingroup async_ops
 *
 * @code
 * try {
 *   auto value = wait(some_async_operation());
 * }
 * catch (std::exception& e) {
 *   std::cerr << e.what() << std::endl;
 * }
 *
 * @endcode
 */
template <typename T>
T wait(async_result<T>&& asyncop) {
  auto prom = std::make_shared<std::promise<T>>();
  auto fut = prom->get_future();
  if constexpr (std::is_same_v<void, T>) {
    std::move(asyncop).then([prom]() { prom->set_value(); }).on_error([prom](auto&& e) {
      prom->set_exception(std::move(e));
    });
  } else {
    std::move(asyncop).then([prom](T&& val) { prom->set_value(std::move(val)); }).on_error([prom](auto&& e) {
      prom->set_exception(std::move(e));
    });
  }
  return fut.get();
}

}  // namespace dolbyio::comms
