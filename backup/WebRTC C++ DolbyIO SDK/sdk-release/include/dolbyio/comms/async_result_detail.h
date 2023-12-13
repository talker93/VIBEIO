#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/exports.h>
#include <dolbyio/comms/sdk_exceptions.h>

#include <cassert>
#include <memory>
#include <optional>
#include <variant>

namespace dolbyio::comms {

namespace async_result_tags {

/**
 * @ingroup async_ops
 * @brief The tag type for constructing the resolved async_result objects in
 * place.
 */
enum resolved_tag { resolved };

/**
 * @ingroup async_ops
 * @brief The tag type for constructing the resolved with failure async_result
 * objects in place.
 */
enum failed_tag { failed };

/**
 * @ingroup async_ops
 * @brief The tag type for using low-level APIs of the low_level_solver
 * and async_result.
 *
 * Use this tag only if you are aware of the requirements
 * that need to be satisfied in order to let APIs work correctly.
 */
enum low_level_tag { intentional_usage };
}  // namespace async_result_tags

namespace detail {

/**
 * @cond DO_NOT_DOCUMENT
 */
template <typename T, typename traits>
class DLB_COMMS_TEMPLATE_EXPORT low_level_solver;

template <typename T, typename traits>
class DLB_COMMS_TEMPLATE_EXPORT async_result;

enum class wrap_async_type {
  async_result,
  value,
};

struct async_result_holds {
  static constexpr size_t nothing = 0;
  static constexpr size_t solver = 1;
  static constexpr size_t value = 2;
  static constexpr size_t error = 3;
};

template <typename U, typename PrevSolverTraits>
struct return_type_helper {
  using type = U;
  using async_result_type = async_result<type, PrevSolverTraits>;
  using solver_type = low_level_solver<type, PrevSolverTraits>;
  using solver_traits = PrevSolverTraits;
  constexpr static const wrap_async_type wraps = wrap_async_type::value;
};

template <typename U, typename T, typename PrevSolverTraits>
struct return_type_helper<async_result<U, T>, PrevSolverTraits> {
  using type = U;
  using async_result_type = async_result<type, T>;
  using solver_type = low_level_solver<type, T>;
  using solver_traits = T;
  constexpr static const wrap_async_type wraps = wrap_async_type::async_result;
};

template <typename ArgType, typename CbType>
struct cb_type_helper;

template <typename CbType>
struct cb_type_helper<void, CbType> {
  using cb_ret_type = decltype(std::declval<CbType>()());
};

template <typename ArgType, typename CbType>
struct cb_type_helper {
  using cb_ret_type = decltype(std::declval<CbType>()(std::declval<ArgType>()));
};

template <typename CallableType, typename CallableRet, typename PrevSolverTraits>
struct wrap_as_solver {
  static auto invoke_and_wrap(CallableType& callable) {
    using ret_type_helper = return_type_helper<CallableRet, PrevSolverTraits>;
    try {
      if constexpr (ret_type_helper::wraps == wrap_async_type::async_result) {
        return callable();
      }
      if constexpr (ret_type_helper::wraps == wrap_async_type::value) {
        if constexpr (std::is_same_v<void, CallableRet>) {
          callable();
          return typename ret_type_helper::async_result_type{};
        } else {
          return typename ret_type_helper::async_result_type{callable(), async_result_tags::resolved};
        }
      }
    } catch (...) {
      return typename ret_type_helper::async_result_type{std::current_exception(), async_result_tags::failed};
    }
  }
};

template <typename ResultType>
class typed_lambda_helpers {
 public:
  static auto empty() {
    if constexpr (std::is_same_v<void, ResultType>) {
      return []() {};
    } else {
      return [](ResultType&&) {};
    }
  }
};

template <typename T, typename traits>
class solver_lambda_helpers {
 public:
  static auto chain_error(low_level_solver<T, traits>* slv) {
    return [slv](std::exception_ptr&& err) { slv->fail(std::move(err)); };
  }

  static auto chain_value(low_level_solver<T, traits>* slv) {
    if constexpr (std::is_same_v<void, T>) {
      return [slv]() { slv->resolve(); };
    } else {
      return [slv](T&& val) { slv->resolve(std::move(val)); };
    }
  }
};
/**
 * @endcond
 */

/**
 * @brief A shared pointer to the low_level_solver object.
 * @ingroup async_ops
 */
template <typename T, typename U>
using low_level_solver_ptr = std::shared_ptr<low_level_solver<T, U>>;

/**
 * @cond DO_NOT_DOCUMENT
 */
template <typename traits>
struct solver_synchronisation_guard : public traits::lock_guard_type {
 public:
  solver_synchronisation_guard(typename traits::lock_type& self) : traits::lock_guard_type(self) {}
};

template <typename traits>
class DLB_COMMS_TEMPLATE_EXPORT solver_base : public traits::lock_type {
 public:
  using err_cb_type = typename traits::template callback_type<void(std::exception_ptr&&)>;

  ~solver_base() {
    assert(!propagate_err_cb_);
    assert(!local_err_cb_);
  }

  const std::exception_ptr& get_error() const { return err_; }

  bool fail(std::exception_ptr&& error) {
    if (local_err_cb_) {
      handle_error_locally(std::exception_ptr(error));
    }

    if (propagate_err_cb_) {
      propagate_error(std::move(error));
      return true;
    } else {
      err_ = std::move(error);
    }
    return false;
  }

  bool set_propagate_error_callback(err_cb_type&& cb) {
    assert(!propagate_err_cb_);
    propagate_err_cb_ = std::move(cb);
    if (!err_)
      return false;

    propagate_error(std::move(this->err_));
    return true;
  }

  bool set_error_callback(err_cb_type&& cb) {
    assert(!propagate_err_cb_);

    local_err_cb_ = std::move(cb);
    if (!err_)
      return false;

    handle_error_locally(std::exception_ptr(err_));
    // leave the local error stored; when the propagate callback is set, it
    // needs to access the error
    return true;
  }

  void reset() {
    local_err_cb_ = {};
    propagate_err_cb_ = {};
    err_ = {};
  }

 private:
  void handle_error_locally(std::exception_ptr&& error) {
    try {
      local_err_cb_(std::move(error));
    } catch (...) {
    }
    local_err_cb_ = {};
  }

  void propagate_error(std::exception_ptr&& error) {
    try {
      propagate_err_cb_(std::move(error));
    } catch (...) {
    }

    // if there is a set local error callback, it should be fired
    // before the propagation and be cleared:
    assert(!local_err_cb_);
  }

  std::exception_ptr err_{};
  err_cb_type propagate_err_cb_{};
  err_cb_type local_err_cb_{};
};

template <typename traits>
class DLB_COMMS_TEMPLATE_EXPORT low_level_solver_chain {
 public:
  void set_chained_solver(const std::shared_ptr<low_level_solver_chain<traits>>& slv) {
    assert(!chained_solver_);
    chained_solver_ = slv;
  }

  const std::shared_ptr<low_level_solver_chain<traits>> get_chained_solver() const {
    assert(chained_solver_);
    return chained_solver_;
  }

 protected:
  low_level_solver_chain() = default;
  ~low_level_solver_chain() { assert(!chained_solver_); }
  void reset() { chained_solver_ = {}; }

 private:
  std::shared_ptr<low_level_solver_chain<traits>> chained_solver_{};
};
/**
 * @endcond
 */

/**
 * @brief A shared state of the solver and async_result objects.
 *
 * The class that stores a low-level building block for code that executes
 * asynchronously. The class allows the caller of the asynchronous
 * function to set the callbacks for notification informing that the function
 * has finished. The class also allows executing code asynchronously to notify
 * that it is finished.
 *
 * The low_level_solver is templated over two types: the type of the produced
 * value and the traits type. Refer to the documentation of the async_result
 * class for the description of the traits type.
 *
 * The low_level_solver provides a low-level API. The user of this class
 * must adhere to the following rules:
 * - Each low_level_solver must be resolved with success or failure only once
 * - Each low_level_solver can construct the async_result value only once
 * - If any callbacks are set on the low_level_solver, the solver must be
 * resolved and then destroyed
 * - Each low_level_solver must be created using the std::make_shared function
 *
 * We do not recommend using the low_level_solver directly. This solver is used
 * as a state object shared between the async_result and the associated solver
 * instance. Using these two types hides the complexity of the low_level_solver
 * and provides a more convenient interface.
 *
 * @tparam T The type of the result produced by the low_level_solver.
 * @tparam traits The traits type.
 * @ingroup async_ops
 */
template <typename T, typename traits>
class DLB_COMMS_TEMPLATE_EXPORT low_level_solver : public low_level_solver_chain<traits> {
  solver_base<traits> base_{};

 public:
  /// @cond DO_NOT_DOCUMENT
  using WrappedType = T;
  using result_cb_type = typename traits::template callback_type<void(T&&)>;
  using err_cb_type = typename solver_base<traits>::err_cb_type;
  solver_base<traits>& base() { return base_; }
  /// @endcond

  /**
   * @brief The constructor.
   */
  low_level_solver(async_result_tags::low_level_tag) {}

  /**
   * @brief The destructor.
   */
  ~low_level_solver() { assert(!cb_); }

  /**
   * @brief Chains the current solver with another solver.
   *
   * Use this method only if the current low_level_solver does not have
   * any assigned callbacks, except for the local error callback. When the
   * current low_level_solver is resolved, the resolve() or fail() method of
   * the chained solver is called. If the current low_level_solver is already
   * resolved, the result is propagated immediately to the chained solver.
   *
   * @param other The shared pointer to the next solver.
   */
  void set_callbacks_from(low_level_solver<T, traits>* other) {
    solver_synchronisation_guard<traits> lock(base_);
    assert(!cb_);
    assert(this->get_chained_solver() && other == this->get_chained_solver().get());
    cb_ = solver_lambda_helpers<T, traits>::chain_value(other);
    bool has_error = base_.set_propagate_error_callback(solver_lambda_helpers<T, traits>::chain_error(other));

    if (has_error) {
      reset();
    } else if (result_) {
      cb_(*std::move(result_));
      reset();
    }
  }

  /**
   * @brief Resolves the solver.
   *
   * If there is a callback stored in the solver, the callback is executed
   * immediately with the result. Otherwise, the result is stored for later
   * consumption when the callback is set.
   * @param res The value to be moved to the callback function.
   */
  void resolve(T&& res) {
    solver_synchronisation_guard<traits> lock(base_);
    assert(!result_);
    assert(!base_.get_error());
    if (cb_) {
      cb_(std::move(res));
      reset();
    } else {
      result_ = std::move(res);
    }
  }

  /**
   * @brief Fails the solver.
   *
   * If the error callback is set for the solver, the solver
   * is invoked. If the current solver is chained with another solver, the
   * fail() method of the chained solver is invoked. If the current solver is
   * not chained with the next solver, the error is stored to be propagated
   * later.
   *
   * @param error The exception pointer that contains the exception.
   */
  void fail(std::exception_ptr&& error) {
    solver_synchronisation_guard<traits> lock(base_);
    assert(!result_);
    if (base_.fail(std::move(error)))
      reset();
  }

  /**
   * @brief Sets a result callback.
   *
   * If the resolve() method has already been called, the callback is executed
   * immediately. Otherwise, the callback is stored to be invoked if
   * the resolve() method is invoked on the solver.
   * @param cb The function object to call when the result is resolved.
   * @return True if the function object has been invoked immediately, false
   * otherwise.
   */
  bool set_callback(result_cb_type&& cb) {
    cb_ = std::move(cb);
    if (!result_)
      return false;

    cb_(*std::move(result_));
    reset();
    return true;
  }

  /**
   * @brief Sets an error callback.
   *
   * If the fail() method has already been called, the error callback is
   * executed immediately. Otherwise, the callback is stored to be
   * executed when the fail() method is called.
   *
   * @param cb The function object to be called in the case of an error.
   * @return True if the function object has been invoked immediately, false
   * otherwise.
   */
  bool set_propagate_error_callback(err_cb_type&& cb) {
    bool has_error = base_.set_propagate_error_callback(std::move(cb));
    if (has_error)
      reset();
    return has_error;
  }

  /**
   * @brief Sets the local error callback.
   *
   * If the fail() method has already been called, the error callback is
   * executed immediately. Otherwise, the callback is stored to be
   * executed when the fail() method is called.
   *
   * The local error callback differs from the error callback that is set by the
   * set_propagate_error_callback(), where the chaining solvers reset the
   * propagated error callbacks and, in case of errors, only the callback
   * of the last solver is invoked. The local error callbacks are
   * invoked for each solver in the chain.
   *
   * The set_error_callback() function must not be invoked after
   * set_propagate_error_callback() or set_callbacks_from().
   *
   * @param cb The function object to be called in the case of an error.
   * @return True if the function object has been invoked immediately, false
   * otherwise.
   */
  bool set_error_callback(err_cb_type&& cb) { return base_.set_error_callback(std::move(cb)); }

 private:
  void reset() {
    result_ = {};
    cb_ = {};

    base_.reset();
    low_level_solver_chain<traits>::reset();
  }

  std::optional<T> result_{};
  result_cb_type cb_{};
};

/**
 * @brief A specialized template of the low_level_solver class that wraps void
 * results.
 *
 *The low_level_solver provides a low-level API. The user of this class
 * must adhere to the following rules:
 * - Each low_level_solver must be resolved with success or failure only once
 * - Each low_level_solver can construct the async_result value only once
 * - If any callbacks are set on the low_level_solver, the solver must be
 * resolved and then destroyed
 * - Each low_level_solver must be created using the std::make_shared function
 *
 * We do not recommend using the low_level_solver directly.
 *
 * @tparam traits The traits type.
 * @ingroup async_ops
 */
template <typename traits>
class DLB_COMMS_TEMPLATE_EXPORT low_level_solver<void, traits> : public low_level_solver_chain<traits> {
  solver_base<traits> base_{};

 public:
  /// @cond DO_NOT_DOCUMENT
  using WrappedType = void;
  using result_cb_type = typename traits::template callback_type<void()>;
  using err_cb_type = typename solver_base<traits>::err_cb_type;
  solver_base<traits>& base() { return base_; }
  /// @endcond

  /**
   * @brief The constructor.
   */
  low_level_solver(async_result_tags::low_level_tag) {}

  /**
   * @brief The destructor.
   */
  ~low_level_solver() { assert(!cb_); }

  /**
   * @brief Chains the current solver with another solver.
   *
   * This method must be used only if the current low_level_solver does not have
   * any callbacks assigned, except for the local error callback. When the
   * current low_level_solver is resolved, the resolve() or fail() method of
   * the chained solver is called. If the current low_level_solver is already
   * resolved, the chained solver is resolved immediately.
   *
   * @param other The shared pointer to the next solver.
   */
  void set_callbacks_from(low_level_solver<void, traits>* other) {
    solver_synchronisation_guard<traits> lock(base_);
    assert(!cb_);
    assert(this->get_chained_solver() && other == this->get_chained_solver().get());
    // do we need anythong for local error here?
    cb_ = solver_lambda_helpers<void, traits>::chain_value(other);
    bool has_error = base_.set_propagate_error_callback(solver_lambda_helpers<void, traits>::chain_error(other));

    if (has_error) {
      reset();
    } else if (has_result_) {
      cb_();
      reset();
    }
  }

  /**
   * @brief Resolves the solver.
   *
   * If there is a callback stored in the solver, the callback is executed
   * immediately. Otherwise, the callback is invoked when it is assigned.
   */
  void resolve() {
    solver_synchronisation_guard<traits> lock(base_);
    assert(!has_result_);
    assert(!base_.get_error());
    if (cb_) {
      cb_();
      reset();
    } else {
      has_result_ = true;
    }
  }

  /**
   * @brief Fails the solver.
   *
   * If the error callback is set for the solver, it
   * is invoked. If the current solver is chained with another solver, the
   * fail() method of that chained solver is invoked. If the current solver is
   * not chained with the next solver, the error is stored to be propagated
   * later.
   *
   * @param error The exception pointer that contains the exception.
   */
  void fail(std::exception_ptr&& error) {
    solver_synchronisation_guard<traits> lock(base_);
    assert(!has_result_);
    if (base_.fail(std::move(error)))
      reset();
  }

  /**
   * @brief Sets a result callback.
   *
   * If the resolve() method has already been called, the callback is executed
   * immediately. Otherwise, the callback is stored to be invoked when
   * the resolve() method is invoked on the solver.
   * @param cb The function object to call when the result is resolved.
   * @return True if the function object has been invoked immediately, false
   * otherwise.
   */
  bool set_callback(result_cb_type&& cb) {
    cb_ = std::move(cb);
    if (!has_result_)
      return false;

    cb_();
    reset();
    return true;
  }

  /**
   * @brief Sets an error callback.
   *
   * If the fail() method has already been called, the error callback is
   * executed immediately. Otherwise, the callback is stored to be
   * executed when the fail() method is called.
   *
   * @param cb The function object to be called in the case of an error.
   * @return True if the function object has been invoked immediately, false
   * otherwise.
   */
  bool set_propagate_error_callback(err_cb_type&& cb) {
    bool has_error = base_.set_propagate_error_callback(std::move(cb));
    if (has_error)
      reset();
    return has_error;
  }

  /**
   * @brief Sets the local error callback.
   *
   * If the fail() method has already been called, the error callback is
   * executed immediately. Otherwise, the callback is stored to be
   * executed when the fail() method is called.
   *
   * The local error callback differs from the error callback that is set by the
   * set_propagate_error_callback(), where the chaining solvers reset the
   * propagated error callbacks and, in case of errors, only the callback of
   * the last solver is invoked. The local error callbacks are
   * invoked for each solver in the chain.
   *
   * The set_error_callback() function must not be invoked after
   * set_propagate_error_callback() or set_callbacks_from().
   *
   * @param cb The function object to be called in the case of an error.
   * @return True if the function object has been invoked immediately, false
   * otherwise.
   */
  bool set_error_callback(err_cb_type&& cb) { return base_.set_error_callback(std::move(cb)); }

 private:
  void reset() {
    has_result_ = false;
    cb_ = {};
    base_.reset();
    low_level_solver_chain<traits>::reset();
  }

  bool has_result_ = false;
  result_cb_type cb_{};
};

template <typename T, typename traits>
class [[nodiscard]] async_result_with_solver;

/**
 * @brief An interface that notifies about the completion of
 * asynchronous operations.
 *
 * When the asynchronous operation is finished, the fail() or resolve()
 * method of the solver is invoked to bring the associated async_result to the
 * resolved state or invoke the callbacks if the async_result has already been
 * finalized. The solver class ensures that the resolution can happen only once,
 * and that the destruction of the pending solver results in resolving the
 * operation with failure.
 *
 * The solver class implements move semantics and should be always passed by
 * move to the entity which eventually resolves it. The moved-from state of the
 * solver is invalid and the only allowed operations on the invalid solver are
 * the destruction, move-assignment, and validity check.
 *
 * @tparam T The type of the result produced by the asynchronous operation.
 * @tparam traits The traits type. For more information, refer to the
 * documentation of the async_result class.
 * @ingroup async_ops
 */
template <typename T, typename traits>
class [[nodiscard]] solver {
  // For accessing the slv_ by the async_result_with_solver class:
  friend async_result_with_solver<T, traits>;

 public:
  /**
   * @brief The default constructor.
   *
   * Constructs an invalid solver. Invoking resolve() or fail() on this solver
   * instance results in assertion failures.
   */
  solver() noexcept = default;

  /**
   * @brief The constructor.
   *
   * Constructs a solver. We do not recommend to invoke this constructor
   * directly. The recommended way of creating solvers is to use the
   * async_result::make() method which creates the solver and the associated
   * async_result in a safe way.
   */
  solver(low_level_solver_ptr<T, traits> slv) noexcept : slv_(std::move(slv)) { assert(slv_); }

  /**
   * @brief The destructor.
   *
   * If the solver is valid and has not been resolved or failed, the destructor
   * fails the solver with the dolbyio::comms::async_operation_canceled
   * exception.
   */
  ~solver() { fail_if_not_destroyed(); }

  /**
   * @brief Notifies about the failed async operation.
   *
   * This method causes the associated async_result to change the state to
   * `resolved with failure` or the method invokes the callbacks in the failure
   * paths directly if the associated async_result object is already in the
   * finalized state. This method consumes the solver object and changes its
   * state to `invalid`.
   * @param e the exception.
   */
  void fail(std::exception_ptr&& e) && {
    auto slv = std::move(slv_);
    assert(slv);
    if (!slv)
      throw dolbyio::comms::async_operation_canceled("Already resolved");
    slv->fail(std::move(e));
  }

  /**
   * @brief Notifies about the successful async operation.
   *
   * This method causes the associated async_result to change the state to
   * `resolved with success` or the method invokes the value callback directly
   * if the associated async_result object is already in the finalized state.
   * This method consumes the solver object and changes its state to `invalid`.
   */
  template <typename TT = T, typename X = std::enable_if_t<std::is_same_v<void, TT>, TT>>
  void resolve() && {
    auto slv = std::move(slv_);
    assert(slv);
    if (!slv)
      throw dolbyio::comms::async_operation_canceled("Already resolved");
    slv->resolve();
  }

  /**
   * @brief Notifies about the successful async operation.
   *
   * This method causes the associated async_result to change the state to
   * `resolved with success` or the method invokes the value callback directly
   * if the associated async_result object is already in the finalized state.
   * This method consumes the solver object and changes its state to `invalid`.
   *
   * @param val The value produced by the asynchronous operation.
   */
  template <typename TT = T, typename X = std::enable_if_t<!std::is_same_v<void, TT>, TT>>
  void resolve(TT&& val) && {
    auto slv = std::move(slv_);
    assert(slv);
    if (!slv)
      throw dolbyio::comms::async_operation_canceled("Already resolved");
    slv->resolve(std::forward<TT>(val));
  }

  /**
   * @brief Checks validity of the solver object.
   *
   * @return true if the solver is valid, false otherwise.
   */
  explicit operator bool() const { return slv_.get() != nullptr; }

  /**
   * @brief Default move constructor for solver.
   */
  solver(solver<T, traits>&&) noexcept = default;

  /**
   * @brief Move assignment operator for solver.
   * @param other The solver to be moved from.
   * @return solver Reference to the Solver which was moved too.
   */
  solver& operator=(solver<T, traits>&& other) noexcept {
    fail_if_not_destroyed();
    slv_ = std::move(other.slv_);
    return *this;
  }

  /// @cond DO_NOT_DOCUMENT
  solver(const solver<T, traits>&) = delete;
  solver& operator=(const solver<T, traits>&) = delete;
  /// @endcond

 private:
  void fail_if_not_destroyed() {
    auto slv = std::move(slv_);
    if (slv) {
      slv->fail(std::make_exception_ptr(dolbyio::comms::async_operation_canceled("Destroyed")));
    }
  }
  low_level_solver_ptr<T, traits> slv_{};
};

/**
 * @brief The pair of the solver and the associated async_result.
 *
 * A helper type used for initiating asynchronous
 * operations. The recommended way of creating an async_result representing an
 * asynchronous operation is to use the async_result::make() static method.
 *
 * @tparam T The type of the result
 * @tparam traits The traits type. For more information, see the async_result
 * documentation.
 * @ingroup async_ops
 */
template <typename T, typename traits>
class [[nodiscard]] async_result_with_solver {
 public:
  /**
   * @brief The constructor.
   */
  async_result_with_solver()
      : solver_(std::make_shared<class low_level_solver<T, traits>>(async_result_tags::intentional_usage)),
        result_(solver_.slv_, async_result_tags::intentional_usage) {}

  /**
   * @brief Gets the solver.
   *
   * This function should be invoked only once, subsequent invocations
   * yield an invalid solver.
   *
   * @return The solver for notifying about the completion of the
   * operation.
   */
  solver<T, traits> get_solver() {
    assert(solver_);
    return std::move(solver_);
  }

  /**
   * @brief Get the result.
   *
   * This function should be invoked only once, subsequent invocations
   * yield a finalized async_result.
   *
   * @return The async_result for connecting the completion
   * callbacks.
   */
  async_result<T, traits> get_result() {
    assert(result_);
    return std::move(result_);
  }

  solver<T, traits> solver_;
  async_result<T, traits> result_;
};

template <typename V>
struct variant_value {
  using type = V;
};

template <>
struct variant_value<void> {
  using type = std::monostate;
};

#ifdef __GNUC__
#define DLB_COMMS_AR_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define DLB_COMMS_AR_UNREACHABLE() __assume(false)
#else
#define DLB_COMMS_AR_UNREACHABLE() throw dolbyio::comms::async_operation_canceled("Unreachable code")
#endif

/**
 * @brief An object that represents the eventual completion or
 * failure of an asynchronous operation and its resulting value.
 *
 * The asynchronous operation may be in one of these states:
 * - Pending: when the asynchronous operation is pending
 * - Resolved: when the asynchronous operation is finished
 *
 * The asynchronous operation may produce a resource, which can be passed to
 * the application, or may not produce anything and only serve and mutate a
 * state. The first template parameter of the async_result object denotes the
 * type of the resource produced by the async operation, with the void type used
 * if the operation does not produce anything besides information about its
 * completion. This type is called a value type of the async operation and the
 * value type of the async_result.
 *
 * The async_result object may be in one of these states:
 * - Pending: The asynchronous operation is pending.
 * - Resolved: The asynchronous operation is resolved.
 * - Finalized: Introduced by a user of the async_result object by doing
 * operations which consume the async_result object, such as moving the object
 * (move-construction or move-assignment) or setting any callbacks. This state
 * is independent of the asynchronous operation state and means that the
 * async_result object has served its purpose of setting callbacks and can
 * not be used anymore.
 *
 * A function that executes asynchronously can return the async_result instance.
 * The caller of the function may use the async_result object to set the
 * callbacks for a success and failure notification. When the asynchronous
 * operation is resolved, the respective callbacks are invoked.
 *
 * If the async_result is resolved by the time the caller sets the
 * callbacks, the callbacks are invoked immediately on the caller's thread.
 * Otherwise, the async_result is pending and the callbacks are invoked when
 * the async operation finishes. It is forbidden to set callbacks on the
 * finalized async_result.
 *
 * The async_result interface allows setting the value of the callback (also
 * called the result callback) and the local error callback. The result callback
 * is invoked when the asynchronous operation changes state from pending to
 * resolved with success. If the async operation generates errors and fails, the
 * local error callback is invoked instead.
 *
 * Setting the value callback constitutes chaining a new async operation. Even
 * if the callback itself works fully synchronously, a new async_result object
 * is created. If the value callback returns another async_result, the
 * async operation created by chaining is resolved when the async operation
 * returned by the value callback is resolved. Otherwise, the async operation
 * created by chaining is resolved immediately after the value callback returns.
 *
 * The consumer of the async_result object can also set the consume errors
 * callback which is mutually exclusive with the value and local error
 * callbacks. If the previous async operation in the chain resolves with
 * failure, the consume errors callback is invoked. It receives the produced
 * error and must return a placeholder or a substitute value of the type of the
 * failed async operation. This value is passed down the chain to the next
 * async operation as if it was successfully generated by the async operation
 * that generated the error. If the previous async operation does not fail,
 * its generated value is passed down the chain and the consume
 * errors callback is not invoked.
 *
 * If the async operation resolves with an error, the subsequent async
 * operations get their local error callbacks invoked in the proper order, one
 * by one, up to the async operation with the set consume errors callback or the
 * final error callback. This mechanism is called error propagation. Note that
 * the error propagation is synchronous and no error callbacks can return
 * async_result types. There is also no backtracking; generating errors
 * in the chain does not allow going back to the previous async operations and
 * performing any rollback. These mechanisms, if needed, should be implemented
 * by the user in terms of uni-directional chains of operations.
 *
 * The final error callback is the last of the callbacks which can be set on
 * the async_result. If set, the error callback ends the operations chain.
 * This callback is invoked when the error is
 * generated by or propagated to the last async operation. In case of the
 * successful resolution of the last async operation in the chain, the final
 * error callback is not invoked.
 *
 * The async_result interface does not provide any possibility of aborting or
 * cancelling the ongoing asynchronous operation. The interface also does not
 * allow modifying the already constructed async operations chain. Such
 * features, if needed, should be implemented by the user in terms of adding
 * hooks in the value callbacks.
 *
 * @tparam T The type of the value produced by the asynchronous operation. If
 * non-void, the value is passed as an rvalue reference to the result
 * callback. Otherwise, the result callback does not take any arguments.
 * @tparam traits The type of traits for configuring internal types
 * used by the async_result. The traits should be provided as a structure which
 * contains definitions of the following types:
 * - The lock_type and the lock_guard_type. The lock_type needs to be
 * constructible, while the lock_guard_type must have the
 * std::lock_guard<lock_type> semantics. By using lock_type = std::mutex
 * and lock_guard_type = std::lock_guard<std::mutex> the async_result is made
 * thread-safe and the resolution can occur on a different thread than the one
 * which sets the callbacks.
 * - The callback_type that should be a template functor class
 * constructible from a lambda. This type must support the bool() operator and
 * move-assignment, but does not need to be copy-constructible and does not need
 * a const operator. A simple implementation of the traits type would just
 * define the callback_type using callback_type = std::function;
 * @ingroup async_ops
 */
template <typename T, typename traits>
class [[nodiscard]] async_result {
  // For accessing the extract_solver() method by this helper class:
  template <typename CallableType, typename CallableRet, typename PrevSolverTraits>
  friend struct wrap_as_solver;

  template <typename X, typename Y>
  friend class async_result;

 public:
  /// @cond DO_NOT_DOCUMENT
  using traits_type = traits;
  using err_cb_type = typename low_level_solver<T, traits>::err_cb_type;
  /// @endcond

  /**
   * @brief A constructor that is used by the asynchronous operation initiator.
   *
   * We do not recommend constructing the async_result manually using this
   * constructor. The recommended way is to use the make() static method.
   *
   * @param on_result The shared pointer to the low_level_solver.
   */
  async_result(low_level_solver_ptr<T, traits> on_result, async_result_tags::low_level_tag)
      : state_(std::in_place_index_t<async_result_holds::solver>{}, std::move(on_result)) {}

  /**
   * @brief Constructs an async_result paired with the solver.
   *
   * We recommend using this method for constructing a new async_result by the
   * initiator of the asynchronous operation. The method returns a pair of
   * objects: the async_result and the associated solver. The async_result
   * object should be returned to the caller, which is the consumer of the
   * asynchronous operation's result. The solver should be passed to the
   * asynchronous code, which eventually resolves the solver when the operation
   * is finished. Resolving the solver changes the state of the async_result to
   * resolved.
   *
   * @return A pair of the async_result and the associated solver.
   *
   * @code
   *  // Enqueues a job on the async operations queue, and returns the
   *  // async_result:
   * async_result<void> do_something() { auto [res, solver] =
   * async_result<void>::make();
   *    async_operations_queue.emplace_back(std::move(solver));
   *    return std::move(res);
   *  }
   *
   *  // Real implementation of the async operation, executed at some point:
   *  void do_something_impl() {
   *    // do stuff
   *
   *    // Causes the async_result<void> returned from do_something() to resolve
   *    // and invoke the callbacks:
   *    std::move(async_operations_queue.front()).resolve();
   *    async_operations_queue.pop_front();
   *  }
   * @endcode
   */
  static async_result_with_solver<T, traits> make() { return async_result_with_solver<T, traits>{}; }

  /**
   * @brief The move constructor.
   *
   * Constructs the async_result by moving the internal state from another
   * async_result. The result passed as an argument is left in the finalized
   * state after the constructor returns.
   *
   * @param other A parameter moved from async_result.
   */
  async_result(async_result<T, traits>&& other) : state_(std::move(other.state_)) { other.reset(); }

  /**
   * @brief Constructs the resolved result.
   *
   * The async_result is brought to the resolved state with the provided value
   * at the moment of construction.
   *
   * The second argument, the async_result_tags::resolved_tag, has a default
   * value and only a single value can be used. Therefore, the
   * argument can be almost always omitted in code. The only scenario where
   * providing the tag is required is constructing the async_result carrying
   * std::exception_ptr as the result of the operation to distinguish
   * between the failed and resolved cases.
   *
   * @code
   * // Returns already-resolved result, carrying value 1:
   * async_result<int, traits> fun() { return 1; }
   *
   * // Returs already-resolved result, carrying the exception pointer.
   * // Note that the result is resolved with success, and the value callback
   * // set in the then() method is invoked:
   * async_result<std::exception_ptr, traits> fun2() {
   *   return {std::make_exception_ptr(std::runtime_error("")),
   *           async_result_tags::resolved};
   * }
   * @endcode
   *
   * @param val The result of the operation
   */
  template <typename TT = T,
            typename X = std::enable_if_t<!std::is_same<TT, void>::value && std::is_convertible<TT, T>::value, T>>
  async_result(TT&& val, async_result_tags::resolved_tag = async_result_tags::resolved)
      : state_(std::in_place_index_t<async_result_holds::value>{}, std::move(val)) {}

  /**
   * @brief Constructs the resolved result.
   *
   * The async_result changes state to `resolved` at the moment of
   * the construction.
   */
  template <typename TT = T, typename X = std::enable_if_t<std::is_same_v<TT, void>, T>>
  async_result() : state_(std::in_place_index_t<async_result_holds::value>{}, std::monostate{}) {}

  /**
   * @brief Constructs the failed result.
   *
   * The async_result changes state to `resolved` with failure at the
   * moment of the construction.
   *
   * The second argument, the async_result_tags::failed_tag, has a default
   * value and there's only a single value which can be used, therefore the
   * argument can almost always be omitted in the code. The only scenario where
   * providing the tag is required is constructing the async_result carrying
   * the std::exception_ptr as the result of the operation, to distinguish
   * between the failed and resolved cases.
   *
   * @code
   * // Returns already-failed result:
   * async_result<int, traits> fun() {
   *   return std::make_exception_ptr(std::runtime_error(""));
   * }
   *
   * // Returns already-failed result.
   * // Note that the result is resolved with failure, and the local error
   * // callback set in the then() method is invoked. The error will be
   * // propagated to the chained results, if any:
   * async_result<std::exception_ptr, traits> fun2() {
   *   return {std::make_exception_ptr(std::runtime_error("")),
   *           async_result_tags::failed};
   * }
   * @endcode
   */
  template <typename TT = T, typename X = std::enable_if_t<std::is_same<TT, std::exception_ptr>::value, T>>
  async_result(TT&& e, async_result_tags::failed_tag = async_result_tags::failed)
      : state_(std::in_place_index_t<async_result_holds::error>{}, std::move(e)) {}

  /**
   * @brief The destructor.
   *
   * The async_result object may be destroyed only after changing state to
   * `finalized`.
   */
  ~async_result() { assert(holds() == async_result_holds::nothing); }

  /**
   * @brief The move assignment operator.
   *
   * Moves the argument to the current async_result. The current async_result
   * must be in the finalized state. The move assignment operator leaves the
   * argument in the finalized state.
   *
   * @param other The parameter moved from async_result.
   * @return The reference to the async_result.
   */
  async_result<T, traits>& operator=(async_result<T, traits>&& other) {
    if (&other == this)
      return *this;
    this->~async_result();
    new (this) async_result(std::move(other));
    return *this;
  }

  /// @cond DO_NOT_DOCUMENT
  async_result(const async_result<T, traits>&) = delete;
  async_result<T, traits>& operator=(const async_result<T, traits>&) = delete;
  /// @endcond

  /**
   * @brief Sets function objects as callbacks on the asynchronous completion.
   *
   * If the async_result is in the pending state, then when the asynchronous
   * operation finishes, depending on the result, one of the passed callbacks
   * is invoked. If the async operation finishes successfully, then
   * the result callback is invoked. If the async operation fails, the
   * local error callback is invoked.
   *
   * If the async_result is already in the resolved state by the time this
   * method is invoked, the result or error callback is executed
   * immediately.
   *
   * This method consumes the current async_result and leaves the result in the
   * finalized state.
   *
   * This method constructs a new async_result. The type of the result depends
   * on the return type of the provided result callback:
   * - If the provided result callback returns async_result<X, Y>, then this
   * method returns async_result<X, Y>. The async_result returned from
   * then() is resolved when the instance returned by the callback function
   * is resolved.
   * - If the provided result callback returns any type Z (may be void), this
   * method returns async_result<Z, traits>. The returned async result is
   * resolved immediately when the callback function is invoked.
   *
   * In any case, if the result callback is invoked and throws an exception,
   * then the async_result returned from then() is resolved with failure.
   *
   * @param cb The result callback function object.
   * @param local_err_cb The local error function object. This functor should
   * not throw any exceptions.
   * @return The async_result representing the result of the callback
   * invocation.
   *
   * @code
   * // On success the following code prints `123`.
   *  method_returns_async_result()
   *    .then([](auto&&)
   *      { std::cerr << "1";
   *        return another_async_method()
   *               .then([](auto&&){ std::cerr << "2"; },
   *                     [](auto&& e) { // rethrow and handle exception
   *                })
   *               .then([](auto&&){ std::cerr << "3"; },
   *                     [](auto&& e) { // rethrow and handle exception
   *               });
   *      })
   *     .on_error([](auto&& e) { // rethrow and handle exception
   *     });
   * @endcode
   */
  template <typename U>
  auto then(U&& cb, err_cb_type&& local_err_cb = {}) && ->
      typename detail::return_type_helper<typename detail::cb_type_helper<T, U>::cb_ret_type,
                                          traits>::async_result_type {
    assert(holds() != async_result_holds::nothing);
    switch (holds()) {
      case async_result_holds::value: {
        return invoke_cb_and_consume_errors(std::forward<U&&>(cb), extract_value());
      }
      case async_result_holds::error: {
        std::exception_ptr e = extract_error();
        if (local_err_cb) {
          local_err_cb(std::exception_ptr{e});
        }
        return {std::move(e), async_result_tags::failed};
      }
      case async_result_holds::solver: {
        using ret_type_info = detail::return_type_helper<typename detail::cb_type_helper<T, U>::cb_ret_type, traits>;

        auto my_solver = extract_solver();
        solver_synchronisation_guard<traits> lock(my_solver->base());
        auto chain_result =
            set_local_error_callback_and_chain<typename ret_type_info::type>(std::move(local_err_cb), my_solver);

        bool already_has_result = false;
        if (!chain_result.already_has_error) {
          already_has_result = set_result_callback<typename ret_type_info::type>(my_solver, std::forward<U&&>(cb));
        }

        if (!already_has_result) {
          set_propagate_error_callback(my_solver, chain_result.chained_slv);
        }

        return {std::move(chain_result.chained_slv), async_result_tags::intentional_usage};
      }
    }
    DLB_COMMS_AR_UNREACHABLE();
  }

  /**
   * @brief Consumes errors produced during an asynchronous operation to not
   * propagate the errors further.
   *
   * The errors are consumed by the provided error
   * callback function object. If the asynchronous operation is
   * resolved with an error, the chained async operations cannot be executed
   * and error callbacks are invoked. By using the
   * consume_errors() call, the error propagation can be stopped. The caller
   * captures the generated error and replaces the error with a result that is
   * propagated to subsequent asynchronous operations. If the callback throws an
   * error, that error is propagated to subsequent async results instead
   * of the captured error. The callback function returns values of type T.
   *
   * This method consumes the current async_result and changes its state to
   * `finalized`.
   *
   * This method constructs a new async_result of the same type as the
   * current async_result's type.
   *
   * @param err_cb The consume errors function object.
   * @return The async_result which is resolved with a value of this result or,
   * in a case of errors, the value returned by the consume errors callback.
   *
   * @code
   *  method_returns_async_result().then([](auto&&){ // handle success
   *                               })
   *                               .consume_errors([](auto&&) { // handle exception
   *                               })
   *                               .then([](){ return another_async_method(); })
   *                               .on_error([](auto&&) { // handle exception
   *                               });
   * @endcode
   */
  auto consume_errors(
      typename traits::template callback_type<T(std::exception_ptr&&)>&& err_cb) && -> async_result<T, traits> {
    assert(holds() != async_result_holds::nothing);
    switch (holds()) {
      case async_result_holds::value:
        return std::move(*this);
      case async_result_holds::error: {
        try {
          if constexpr (std::is_same_v<T, void>) {
            err_cb(extract_error());
            return {};
          } else {
            return {err_cb(extract_error()), async_result_tags::resolved};
          }
        } catch (...) {
          return {std::current_exception(), async_result_tags::failed};
        }
      }
      case async_result_holds::solver: {
        auto end_propagation_slv = extract_solver();

        auto chained_slv = std::make_shared<low_level_solver<T, traits>>(async_result_tags::intentional_usage);

        solver_synchronisation_guard<traits> lock(end_propagation_slv->base());
        end_propagation_slv->set_chained_solver(chained_slv);

        bool already_has_error = end_propagation_slv->set_error_callback(
            [chained_slv = chained_slv.get(), err_cb{std::move(err_cb)}](std::exception_ptr&& e) mutable {
              try {
                constexpr bool void_result = std::is_same_v<T, void>;
                if constexpr (void_result) {
                  err_cb(std::move(e));
                  chained_slv->resolve();
                } else {
                  auto val = err_cb(std::move(e));
                  chained_slv->resolve(std::move(val));
                }
              } catch (...) {
                chained_slv->fail(std::current_exception());
              }
            });

        bool already_has_result = false;
        if (!already_has_error) {
          already_has_result =
              end_propagation_slv->set_callback(solver_lambda_helpers<T, traits>::chain_value(chained_slv.get()));
        }

        if (!already_has_result) {
          end_propagation_slv->set_propagate_error_callback(typed_lambda_helpers<std::exception_ptr>::empty());
        }

        return {std::move(chained_slv), async_result_tags::intentional_usage};
      }
    }
    DLB_COMMS_AR_UNREACHABLE();
  }

  /**
   * @brief Sets the final error callback on the async_result.
   *
   * This callback is executed when the async operation resolves with failure.
   * If the async operation resolves with a result, the final error callback is
   * never invoked. If the async_result is already in the resolved with failure
   * state when this function is called, the final error callback is
   * executed immediately.
   *
   * This method consumes the current async_result and changes its state to
   * `finalized`.
   *
   * This method does not return anything and should be invoked to close the
   * async operations chain. In order to handle errors in the middle of the
   * async operations chain, use the local error callbacks or the
   * consume errors callback.
   *
   * The final error callback is allowed to throw exceptions of its own, but
   * these exceptions are caught and ignored.
   *
   * @param err_cb The final error function object.
   *
   * @code
   *  method_returns_async_result().then([](auto&&){ // handle success
   *                                })
   *                               .on_error([](auto&& e) { // rethrow and handle exception
   *                                });
   * @endcode
   */
  void on_error(err_cb_type&& err_cb) && {
    assert(holds() != async_result_holds::nothing);
    switch (holds()) {
      case async_result_holds::value: {
        reset();
        return;
      }
      case async_result_holds::error: {
        err_cb(extract_error());
        return;
      }
      case async_result_holds::solver: {
        auto old_solver = extract_solver();
        solver_synchronisation_guard<traits> lock(old_solver->base());
        bool already_failed = old_solver->set_propagate_error_callback(std::move(err_cb));
        if (!already_failed) {
          old_solver->set_callback(typed_lambda_helpers<T>::empty());
        }
        return;
      }
    }
    DLB_COMMS_AR_UNREACHABLE();
  }

  /**
   * @brief Checks if the async_result is not finalized.
   *
   * @return False if the async_result is finalized, true otherwise.
   */
  explicit operator bool() const { return holds() != async_result_holds::nothing; }

 private:
  size_t holds() const { return state_.index(); }
  void reset() { state_.template emplace<async_result_holds::nothing>(); }

  template <typename U, typename Val>
  static auto invoke_cb_and_consume_errors(U&& cb, Val&& value) ->
      typename detail::return_type_helper<typename detail::cb_type_helper<T, U>::cb_ret_type,
                                          traits>::async_result_type {
    auto invoke_cb = [](auto&& cb, auto&& val) {
      if constexpr (!std::is_same_v<void, T>) {
        return cb(std::move(val));
      } else {
        return cb();
      }
    };

    try {
      if constexpr (std::is_same_v<std::exception_ptr, typename detail::cb_type_helper<T, U>::cb_ret_type>) {
        return {invoke_cb(std::forward<U&&>(cb), std::forward<Val&&>(value)), async_result_tags::resolved};
      } else if constexpr (!std::is_same_v<void, typename detail::cb_type_helper<T, U>::cb_ret_type>) {
        return invoke_cb(std::forward<U&&>(cb), std::forward<Val&&>(value));
      } else {
        invoke_cb(std::forward<U&&>(cb), std::forward<Val&&>(value));
        return {};
      }
    } catch (...) {
      return {std::current_exception(), async_result_tags::failed};
    }
  }

  low_level_solver_ptr<T, traits> extract_solver() {
    assert(holds() == async_result_holds::solver);
    low_level_solver_ptr<T, traits> my_solver = std::move(std::get<async_result_holds::solver>(state_));
    reset();
    return my_solver;
  }

  typename variant_value<T>::type extract_value() {
    assert(holds() == async_result_holds::value);
    typename variant_value<T>::type val = std::move(std::get<async_result_holds::value>(state_));
    reset();
    return val;
  }

  std::exception_ptr extract_error() {
    assert(holds() == async_result_holds::error);
    std::exception_ptr e = std::move(std::get<async_result_holds::error>(state_));
    reset();
    assert(e);
    return e;
  }

  static bool set_local_error_callback(err_cb_type&& local_err_cb, low_level_solver_ptr<T, traits>& my_solver) {
    bool already_has_error = false;
    if (local_err_cb) {
      already_has_error = my_solver->set_error_callback(std::move(local_err_cb));
    }
    return already_has_error;
  }

  template <typename U>
  struct set_local_error_callback_and_chain_result {
    low_level_solver_ptr<U, traits> chained_slv{};
    bool already_has_error = false;
  };

  template <typename U>
  static auto set_local_error_callback_and_chain(err_cb_type&& local_err_cb,
                                                 low_level_solver_ptr<T, traits>& my_solver) {
    auto already_has_error = set_local_error_callback(std::move(local_err_cb), my_solver);

    auto slv = std::make_shared<low_level_solver<U, traits>>(async_result_tags::intentional_usage);
    my_solver->set_chained_solver(slv);
    return set_local_error_callback_and_chain_result<U>{std::move(slv), already_has_error};
  }

  template <typename SlvType, typename CbType>
  static void propagate_nested_callback_value(low_level_solver<T, traits>* my_solver, CbType& cb) {
    auto nested_result = detail::wrap_as_solver<decltype(cb), decltype(cb()), traits>::invoke_and_wrap(cb);
    auto* slv = static_cast<low_level_solver<SlvType, traits>*>(my_solver->get_chained_solver().get());
    switch (nested_result.holds()) {
      case async_result_holds::value: {
        auto val = nested_result.extract_value();
        if constexpr (std::is_same_v<variant_value<void>::type, decltype(val)>) {
          slv->resolve();
        } else {
          slv->resolve(std::move(val));
        }
      } break;
      case async_result_holds::error: {
        slv->fail(nested_result.extract_error());
      } break;
      case async_result_holds::solver: {
        // that solver obviously does not have any callbacks attached, and is not chained:
        auto extracted_slv = nested_result.extract_solver();
        extracted_slv->set_chained_solver(my_solver->get_chained_solver());
        extracted_slv->set_callbacks_from(slv);
      } break;
    }
  }

  template <typename SlvType, typename U, typename TT = T>
  static std::enable_if_t<!std::is_same_v<void, TT>, bool> set_result_callback(
      const low_level_solver_ptr<T, traits>& my_solver,
      U&& cb) {
    return my_solver->set_callback([cb{std::forward<U&&>(cb)}, my_solver = my_solver.get()](TT&& val) mutable {
      auto wrapper_cb = [val{std::move(val)}, &cb]() mutable { return cb(std::move(val)); };

      propagate_nested_callback_value<SlvType>(my_solver, wrapper_cb);
    });
  }

  template <typename SlvType, typename U, typename TT = T>
  static std::enable_if_t<std::is_same_v<void, TT>, bool> set_result_callback(
      const low_level_solver_ptr<T, traits>& my_solver,
      U&& cb) {
    return my_solver->set_callback([cb{std::forward<U&&>(cb)}, my_solver = my_solver.get()]() mutable {
      propagate_nested_callback_value<SlvType>(my_solver, cb);
    });
  }

  template <typename U>
  static void set_propagate_error_callback(const low_level_solver_ptr<T, traits>& from,
                                           const low_level_solver_ptr<U, traits>& to) {
    from->set_propagate_error_callback([to = to.get()](auto&& err) { to->fail(std::move(err)); });
  }

  std::variant<std::monostate,                   // async_result_holds::nothing
               low_level_solver_ptr<T, traits>,  // async_result_holds::solver
               typename variant_value<T>::type,  // async_result_holds::value
               std::exception_ptr                // async_result_holds::error
               >
      state_;
};

}  // namespace detail
}  // namespace dolbyio::comms
