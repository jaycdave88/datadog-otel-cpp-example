// Copyright 2020, Open Telemetry Authors
// Copyright 2017, OpenCensus Authors
//
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
// limitations under the License.

#include "opentelemetry/sdk/trace/samplers/trace_id_ratio.h"

#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace trace_api = opentelemetry::trace;

namespace
{
/**
 * Converts a ratio in [0, 1] to a threshold in [0, UINT64_MAX]
 *
 * @param ratio a required value top be converted to uint64_t. is
 * bounded by 1 >= ratio >= 0.
 * @return Returns threshold value computed after converting ratio to
 * uint64_t datatype
 */
uint64_t CalculateThreshold(double ratio) noexcept
{
  if (ratio <= 0.0)
    return 0;
  if (ratio >= 1.0)
    return UINT64_MAX;

  // We can't directly return ratio * UINT64_MAX.
  //
  // UINT64_MAX is (2^64)-1, but as a double rounds up to 2^64.
  // For probabilities >= 1-(2^-54), the product wraps to zero!
  // Instead, calculate the high and low 32 bits separately.
  const double product = UINT32_MAX * ratio;
  double hi_bits, lo_bits = ldexp(modf(product, &hi_bits), 32) + product;
  return (static_cast<uint64_t>(hi_bits) << 32) + static_cast<uint64_t>(lo_bits);
}

/**
 * @param trace_id a required value to be converted to uint64_t. trace_id must
 * at least 8 bytes long
 * @return Returns threshold value computed after converting trace_id to
 * uint64_t datatype
 */
uint64_t CalculateThresholdFromBuffer(const trace_api::TraceId &trace_id) noexcept
{
  // We only use the first 8 bytes of TraceId.
  static_assert(trace_api::TraceId::kSize >= 8, "TraceID must be at least 8 bytes long.");

  uint64_t res = 0;
  std::memcpy(&res, &trace_id, 8);

  double ratio = (double)res / UINT64_MAX;

  return CalculateThreshold(ratio);
}
}  // namespace

OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdk
{
namespace trace
{
TraceIdRatioBasedSampler::TraceIdRatioBasedSampler(double ratio)
    : threshold_(CalculateThreshold(ratio))
{
  if (ratio > 1.0)
    ratio = 1.0;
  if (ratio < 0.0)
    ratio = 0.0;
  description_ = "TraceIdRatioBasedSampler{" + std::to_string(ratio) + "}";
}

SamplingResult TraceIdRatioBasedSampler::ShouldSample(
    const trace_api::SpanContext & /*parent_context*/,
    trace_api::TraceId trace_id,
    nostd::string_view /*name*/,
    trace_api::SpanKind /*span_kind*/,
    const opentelemetry::common::KeyValueIterable & /*attributes*/,
    const trace_api::SpanContextKeyValueIterable & /*links*/) noexcept
{
  if (threshold_ == 0)
    return {Decision::DROP, nullptr};

  if (CalculateThresholdFromBuffer(trace_id) <= threshold_)
  {
    return {Decision::RECORD_AND_SAMPLE, nullptr};
  }

  return {Decision::DROP, nullptr};
}

nostd::string_view TraceIdRatioBasedSampler::GetDescription() const noexcept
{
  return description_;
}
}  // namespace trace
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
