#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#if defined(WIN32) || defined(DOXYGEN)
#include <cstddef>

namespace dolbyio {
namespace comms {

/**
 * @attention This is only available on Windows.
 *
 * The custom allocator that allows applications to override operators new and delete.
 *
 * This struct should be used by applications that define their own global operators new and delete. The SDK passes
 * some memory ownership to the application, and it must use the same allocator that the application uses, otherwise
 * memory corruption occurs.
 */
struct app_allocator {
 public:
  /**
   * The memory allocation function.
   *
   * The allocation function must satisfy the following requirements:
   * - The allocated memory must have alignment suitable for holding object of given size
   * - The inability to allocate memory should result in returning the null pointer, or throwing the std::bad_alloc{}
   * exception
   *
   * If this function throws an exception, the std::new_handler will not be invoked by the caller.
   *
   * @param size The non-zero requested memory size.
   * @return The pointer to allocated memory, or nullptr.
   */
  typedef void* (*alloc_fn)(size_t size);

  /**
   * The aligned memory allocation function.
   *
   * The allocation function must satisfy the following requirements:
   * - The allocated memory must be aligned to the provided alignment value
   * - The inability to allocate memory should result in returning the null pointer, or throwing the std::bad_alloc{}
   * exception
   *
   * If this function throws an exception, the std::new_handler will not be invoked by the caller.
   *
   * @param size The non-zero requested memory size.
   * @param alignment The alignment of the memory.
   * @return The pointer to allocated memory, or nullptr.
   */
  typedef void* (*alloc_aligned_fn)(size_t size, size_t alignment);

  /**
   * The memory deallocation function.
   *
   * The following guarantees are provided by the SDK:
   * - Only the pointers obtained from alloc_fn will be passed to this function
   * - Null pointers will not be passed to this function
   *
   * @param ptr The valid pointer to the deallocated memory.
   */
  typedef void (*dealloc_fn)(void* ptr);

  /**
   * The aligned memory deallocation function.
   *
   * The following guarantees are provided by the SDK:
   * - Only the pointers obtained from alloc_aligned_fn will be passed to this function
   * - For each passed memory pointer, the value of the alignment argument will be the same as the one used for
   * allocating that memory in alloc_aligned_fn
   * - Null pointers will not be passed to this function
   *
   * @param ptr The valid pointer to the deallocated memory.
   * @param alignment The alignments value matching the value provided to the alloc_aligned_fn() call that caused
   * allocation of the memory.
   */
  typedef void (*dealloc_aligned_fn)(void* ptr, size_t alignment);

  /**
   * The constructor.
   *
   * This constructor constructs the allocator that uses application-provided functions for memory management. All
   * provided function pointers must be valid and non-null.
   */
  constexpr app_allocator(alloc_fn alloc,
                          alloc_aligned_fn aligned_alloc,
                          dealloc_fn dealloc,
                          dealloc_aligned_fn aligned_dealloc)
      : alloc_fn_(alloc),
        aligned_alloc_fn_(aligned_alloc),
        dealloc_fn_(dealloc),
        aligned_dealloc_fn_(aligned_dealloc) {}

  bool operator==(const app_allocator& other) const noexcept {
    return alloc_fn_ == other.alloc_fn_ && aligned_alloc_fn_ == other.aligned_alloc_fn_ &&
           dealloc_fn_ == other.dealloc_fn_ && aligned_dealloc_fn_ == other.aligned_dealloc_fn_;
  }

  alloc_fn alloc_fn_;
  alloc_aligned_fn aligned_alloc_fn_;
  dealloc_fn dealloc_fn_;
  dealloc_aligned_fn aligned_dealloc_fn_;
};

}  // namespace comms
}  // namespace dolbyio
#endif  // Windows
