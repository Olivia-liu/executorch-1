/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <executorch/runtime/core/event_tracer.h>

/**
 * @file
 *
 * This file contains the hooks that can be used by runtime delegate backend
 * authors to log profiling and debugging events from backend code. In order to
 * use these hooks delegate authors would have needed to generate a delegate
 * debug identifier mapping using the DelegateMappingBuilder library present in
 * executorch/exir/backend/utils.py. The delegate debug identifiers generated by
 * that library are the ones that need to be passed to these hooks to log
 * events. Using any other identifiers will cause post-processing of the events
 * data to not properly link back to the nodes in the original lowered graph.
 *
 * The benefit of defining these hooks is that we can easily control whether or
 * not we want to compile in the EventTracer code based on the status of the
 * ET_EVENT_TRACER_ENABLED flag.
 */

namespace torch {
namespace executor {

/**
 * Start the profiling of a delegate event. Similar to start_profiling it will
 * return an instance of EventTracerEntry that contains the details of this
 * event. Can be left in production code as these hooks compile conditionally.
 *
 * @param[in] event_tracer The event tracer instance that is doing the logging.
 * @param[in] name Human readable name for the delegate event. This name has
 * to be the same name that was passed in during the Debug delegate mapping
 * generation in the export/ahead-of-time process. If indices and not names
 * are used by this delegate to identify ops executed in the backend then
 * nullptr can be passed in. Users calling this interface do not need to keep
 * the memory pointed to by this pointer around. The string must be copied over
 * into internal memory during this call.
 * @param[in] delegate_debug_id The id of the delegate event. If string
 * based names are used by this delegate to identify ops executed in the
 * backend then kUnsetDebugHandle should be passed in here.
 */
inline EventTracerEntry event_tracer_start_profiling_delegate(
    EventTracer* event_tracer,
    const char* name,
    DebugHandle delegate_debug_id) {
#ifdef ET_EVENT_TRACER_ENABLED
  if (event_tracer) {
    return event_tracer->start_profiling_delegate(name, delegate_debug_id);
  }
#else //! ET_EVENT_TRACER_ENABLED
  (void)name;
  (void)delegate_debug_id;
#endif
  // There is no active tracer; this value will be ignored.
  return EventTracerEntry();
}
/**
 * Signal the end of the delegate profiling event contained in
 * event_tracer_entry. Users also have the option to log some some free-from
 * string based metadata along with this. Can be left in production code as
 * these hooks compile conditionally.
 *
 * @param[in] event_tracer The event tracer instance that is doing the logging.
 * @param[in] event_tracer_entry The EventTracerEntry returned by a call to
 * start_profiling_delegate().
 * @param[in] metadata Optional data relevant to the execution that the user
 * wants to log along with this event. Pointer to metadata doesn't need to be
 * valid after the call to this function. The contents and format of the data
 * are transparent to the event tracer. It will just pipe along the data and
 * make it available for the user again in the post-processing stage.
 * @param[in] metadata_len Length of the metadata buffer.
 */
inline void event_tracer_end_profiling_delegate(
    EventTracer* event_tracer,
    EventTracerEntry event_tracer_entry,
    const void* metadata = nullptr,
    size_t metadata_len = 0) {
#ifdef ET_EVENT_TRACER_ENABLED
  if (event_tracer) {
    event_tracer->end_profiling_delegate(
        event_tracer_entry, metadata, metadata_len);
  }
#else //! ET_EVENT_TRACER_ENABLED
  (void)event_tracer_entry;
  (void)metadata;
  (void)metadata_len;
#endif
}

/**
 * Some delegates get access to the profiling details only after the complete
 * graph has been executed. This interface is to support such use cases. It
 * can be called in a loop etc. to log any number of profiling events that are
 * part of this delegate. Can be left in production code as these hooks
 * compile conditionally.
 *
 * @param[in] event_tracer The event tracer instance that is doing the logging.
 * @param[in] name Human readable name for the delegate event. This name has
 * to be the same name that was passed in during the Debug delegate mapping
 * generation in the export/ahead-of-time process. If indices and not names
 * are used by this delegate to identify ops executed in the backend then
 * nullptr can be passed in. Users calling this interface do not need to keep
 * the memory pointed to by this pointer around. The string must
 * be copied over into internal memory during this call.
 * @param[in] delegate_debug_id The id of the delegate event. If string
 * based names are used by this delegate to identify ops executed in the
 * backend then -1 should be passed in here.
 * @param[in] start_time The timestamp when the delegate event started.
 * @param[in] end_time The timestamp when the delegate event finished.
 * @param[in] metadata Optional data relevant to the execution that the user
 * wants to log along with this event. Pointer to metadata doesn't need to be
 * valid after the call to this function. The contents and format of the data
 * are transparent to the event tracer. It will just pipe along the data and
 * make it available for the user again in the post-processing stage.
 * @param[in] metadata_len Length of the metadata buffer.
 */
inline void event_tracer_log_profiling_delegate(
    EventTracer* event_tracer,
    const char* name,
    DebugHandle delegate_debug_id,
    et_timestamp_t start_time,
    et_timestamp_t end_time,
    const void* metadata = nullptr,
    size_t metadata_len = 0) {
#ifdef ET_EVENT_TRACER_ENABLED
  if (event_tracer) {
    event_tracer->log_profiling_delegate(
        name, delegate_debug_id, start_time, end_time, metadata, metadata_len);
  }
#else //! ET_EVENT_TRACER_ENABLED
  (void)name;
  (void)delegate_debug_id;
  (void)start_time;
  (void)end_time;
  (void)metadata;
  (void)metadata_len;
#endif
}

/**
 * This templated interfaces can be called in a loop etc. to log any number of
 * debug events that are part of this delegate. Supported values types are int,
 * bool, double, tensor and array of tensors. Can be left in production code as
 * these hooks compile conditionally.
 *
 * @param[in] event_tracer The event tracer instance that is doing the logging.
 * @param[in] name Human readable name for the delegate event. This name has
 * to be the same name that was passed in during the Debug delegate mapping
 * generation in the export/ahead-of-time process. If indices and not names
 * are used by this delegate to identify ops executed in the backend then
 * nullptr can be passed in. Users calling this interface do not need to keep
 * the memory pointed to by this pointer around. The string must
 * be copied over into internal memory during this call.
 * @param[in] delegate_debug_id The id of the delegate event. If string
 * based names are used by this delegate to identify ops executed in the
 * backend then -1 should be passed in here.
 * @param[in] output The output to be logged.
 */
template <typename T>
inline void event_tracer_log_output_delegate(
    EventTracer* event_tracer,
    const char* name,
    DebugHandle delegate_debug_id,
    const T& output) {
#ifdef ET_EVENT_TRACER_ENABLED
  if (event_tracer) {
    static_assert(
        std::is_same<T, int>::value || std::is_same<T, bool>::value ||
            std::is_same<T, double>::value || std::is_same<T, Tensor>::value ||
            std::is_same<T, ArrayRef<Tensor>>::value,
        "Unsupported type for intermediate output");
    event_tracer->log_intermediate_output_delegate(
        name, delegate_debug_id, output);
  }
#else //! ET_EVENT_TRACER_ENABLED
  (void)name;
  (void)delegate_debug_id;
  (void)output;
#endif
}

} // namespace executor
} // namespace torch
