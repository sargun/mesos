// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#ifndef __ASYNC_HPP__
#define __ASYNC_HPP__

#include <boost/type_traits.hpp> // TODO(benh): Use C++11 type_traits.

#include <process/dispatch.hpp>
#include <process/future.hpp>
#include <process/id.hpp>
#include <process/process.hpp>

#include <stout/lambda.hpp>
#include <stout/nothing.hpp>
#include <stout/preprocessor.hpp>
#include <stout/result_of.hpp>

namespace process {

// Provides an abstraction for asynchronously executing a function
// (note the declarations are here and definitions below since
// defining and declaring below will require defining the default
// argument when declaring these as friends in AsyncExecutor which is
// brittle).

template <typename F>
Future<typename result_of<F()>::type> async(
    const F& f,
    typename boost::disable_if<boost::is_void<typename result_of<F()>::type> >::type* = nullptr); // NOLINT(whitespace/line_length)


template <typename F>
Future<Nothing> async(
    const F& f,
    typename boost::enable_if<boost::is_void<typename result_of<F()>::type> >::type* = nullptr); // NOLINT(whitespace/line_length)


#define TEMPLATE(Z, N, DATA)                                            \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  Future<typename result_of<F(ENUM_PARAMS(N, A))>::type> async( \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::disable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type* = nullptr); /* NOLINT(whitespace/line_length) */ \
                                                                        \
                                                                        \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  Future<Nothing> async(                                                \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::enable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type* = nullptr); // NOLINT(whitespace/line_length)

  REPEAT_FROM_TO(1, 11, TEMPLATE, _) // Args A0 -> A9.
#undef TEMPLATE


// TODO(vinod): Merge this into ExecutorProcess.
class AsyncExecutorProcess : public Process<AsyncExecutorProcess>
{
private:
  friend class AsyncExecutor;

  AsyncExecutorProcess() : ProcessBase(ID::generate("__async_executor__")) {}
  virtual ~AsyncExecutorProcess() {}

  // Not copyable, not assignable.
  AsyncExecutorProcess(const AsyncExecutorProcess&);
  AsyncExecutorProcess& operator=(const AsyncExecutorProcess&);

  template <typename F>
  typename result_of<F()>::type execute(
      const F& f,
      typename boost::disable_if<boost::is_void<typename result_of<F()>::type> >::type* = nullptr) // NOLINT(whitespace/line_length)
  {
    terminate(self()); // Terminate process after function returns.
    return f();
  }

  template <typename F>
  Nothing execute(
      const F& f,
      typename boost::enable_if<boost::is_void<typename result_of<F()>::type> >::type* = nullptr) // NOLINT(whitespace/line_length)
  {
    terminate(self()); // Terminate process after function returns.
    f();
    return Nothing();
  }

#define TEMPLATE(Z, N, DATA)                                            \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  typename result_of<F(ENUM_PARAMS(N, A))>::type execute(       \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::disable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type* = nullptr) /* NOLINT(whitespace/line_length) */ \
  {                                                                     \
    terminate(self()); /* Terminate process after function returns. */  \
    return f(ENUM_PARAMS(N, a));                                        \
  }                                                                     \
                                                                        \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  Nothing execute(                                                      \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::enable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type* = nullptr) /* NOLINT(whitespace/line_length) */ \
  {                                                                     \
    terminate(self()); /* Terminate process after function returns. */  \
    f(ENUM_PARAMS(N, a));                                               \
    return Nothing();                                                   \
  }

  REPEAT_FROM_TO(1, 11, TEMPLATE, _) // Args A0 -> A9.
#undef TEMPLATE
};


// This is a wrapper around AsyncExecutorProcess.
class AsyncExecutor
{
private:
  // Declare async functions as friends.
  template <typename F>
  friend Future<typename result_of<F()>::type> async(
      const F& f,
      typename boost::disable_if<boost::is_void<typename result_of<F()>::type> >::type*); // NOLINT(whitespace/line_length)

  template <typename F>
  friend Future<Nothing> async(
      const F& f,
      typename boost::enable_if<boost::is_void<typename result_of<F()>::type> >::type*); // NOLINT(whitespace/line_length)

#define TEMPLATE(Z, N, DATA)                                            \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  friend Future<typename result_of<F(ENUM_PARAMS(N, A))>::type> async( \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::disable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type*); /* NOLINT(whitespace/line_length) */ \
                                                                        \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  friend Future<Nothing> async(                                         \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::enable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type*); // NOLINT(whitespace/line_length)

  REPEAT_FROM_TO(1, 11, TEMPLATE, _) // Args A0 -> A9.
#undef TEMPLATE

  AsyncExecutor()
  {
    process = new AsyncExecutorProcess();
    spawn(process, true); // Automatically GC.
  }

  virtual ~AsyncExecutor() {}

  // Not copyable, not assignable.
  AsyncExecutor(const AsyncExecutor&);
  AsyncExecutor& operator=(const AsyncExecutor&);

  template <typename F>
  Future<typename result_of<F()>::type> execute(
      const F& f,
      typename boost::disable_if<boost::is_void<typename result_of<F()>::type> >::type* = nullptr) // NOLINT(whitespace/line_length)
  {
    // Need to disambiguate overloaded method.
    typename result_of<F()>::type(AsyncExecutorProcess::*method)(const F&, typename boost::disable_if<boost::is_void<typename result_of<F()>::type> >::type*) = // NOLINT(whitespace/line_length)
      &AsyncExecutorProcess::execute<F>;

    return dispatch(process, method, f, (void*) nullptr);
  }

  template <typename F>
  Future<Nothing> execute(
      const F& f,
      typename boost::enable_if<boost::is_void<typename result_of<F()>::type> >::type* = nullptr) // NOLINT(whitespace/line_length)
  {
    // Need to disambiguate overloaded method.
    Nothing(AsyncExecutorProcess::*method)(const F&, typename boost::enable_if<boost::is_void<typename result_of<F()>::type> >::type*) = // NOLINT(whitespace/line_length)
      &AsyncExecutorProcess::execute<F>;

    return dispatch(process, method, f, (void*) nullptr);
  }

#define TEMPLATE(Z, N, DATA)                                            \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  Future<typename result_of<F(ENUM_PARAMS(N, A))>::type> execute( \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::disable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type* = nullptr) /* NOLINT(whitespace/line_length) */ \
  {                                                                     \
    /* Need to disambiguate overloaded method. */                       \
    typename result_of<F(ENUM_PARAMS(N, A))>::type(AsyncExecutorProcess::*method)(const F&, ENUM_PARAMS(N, A), typename boost::disable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type*) = /* NOLINT(whitespace/line_length) */ \
      &AsyncExecutorProcess::execute<F, ENUM_PARAMS(N, A)>;             \
                                                                        \
    return dispatch(process, method, f, ENUM_PARAMS(N, a), (void*) nullptr); \
  }                                                                     \
                                                                        \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  Future<Nothing> execute(                                              \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::enable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type* = nullptr) /* NOLINT(whitespace/line_length) */ \
  {                                                                     \
    /* Need to disambiguate overloaded method. */                       \
    Nothing(AsyncExecutorProcess::*method)(const F&, ENUM_PARAMS(N, A), typename boost::enable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type*) = /* NOLINT(whitespace/line_length) */ \
      &AsyncExecutorProcess::execute<F, ENUM_PARAMS(N, A)>;             \
                                                                        \
    return dispatch(process, method, f, ENUM_PARAMS(N, a), (void*) nullptr); \
  }

  REPEAT_FROM_TO(1, 11, TEMPLATE, _) // Args A0 -> A9.
#undef TEMPLATE

  AsyncExecutorProcess* process;
};


// Provides an abstraction for asynchronously executing a function.
template <typename F>
Future<typename result_of<F()>::type> async(
    const F& f,
    typename boost::disable_if<boost::is_void<typename result_of<F()>::type> >::type*) // NOLINT(whitespace/line_length)
{
  return AsyncExecutor().execute(f);
}


template <typename F>
Future<Nothing> async(
    const F& f,
    typename boost::enable_if<boost::is_void<typename result_of<F()>::type> >::type*) // NOLINT(whitespace/line_length)
{
  return AsyncExecutor().execute(f);
}


#define TEMPLATE(Z, N, DATA)                                            \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  Future<typename result_of<F(ENUM_PARAMS(N, A))>::type> async( \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::disable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type*) /* NOLINT(whitespace/line_length) */ \
  {                                                                     \
    return AsyncExecutor().execute(f, ENUM_PARAMS(N, a));               \
  }                                                                     \
                                                                        \
  template <typename F, ENUM_PARAMS(N, typename A)>                     \
  Future<Nothing> async(                                                \
      const F& f,                                                       \
      ENUM_BINARY_PARAMS(N, A, a),                                      \
      typename boost::enable_if<boost::is_void<typename result_of<F(ENUM_PARAMS(N, A))>::type> >::type*) /* NOLINT(whitespace/line_length) */ \
  {                                                                     \
    return AsyncExecutor().execute(f, ENUM_PARAMS(N, a));               \
  }

  REPEAT_FROM_TO(1, 11, TEMPLATE, _) // Args A0 -> A9.
#undef TEMPLATE

} // namespace process {

#endif // __ASYNC_HPP__
