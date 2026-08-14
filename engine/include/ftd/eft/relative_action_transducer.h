#pragma once

/**
 * @file relative_action_transducer.h
 * @brief FTD-0860 isolated nonzero-carrier action-pump witness.
 *
 * For a nonzero real canonical pair z=(q,p), a signed event (s,B) applies
 *
 *   z' = sqrt((I+B)/I) s J z,  I=(q^2+p^2)/2,  J(q,p)=(-p,q).
 *
 * The map is deterministic, target blind, symplectic on I>0, and increases
 * the pair action by exactly B.  It is deliberately not a faithful event
 * history encoding on an arbitrary background: opposite event signs can
 * collide on opposite input phases, and I+B does not separate I from B.
 * Empty carriers fail closed because a positive rotation-equivariant output
 * has no phase at z=0.
 *
 * This interface is isolated under ftd::eft.  It is not called by Voxel or a
 * production tick phase and supplies no local-mode construction, controller,
 * relative-energy ledger, Born weight, G* cadence, or physical vacuum claim.
 */

#include <cstdint>

namespace ftd::eft {

enum class RelativeActionTransducerStatus : std::uint8_t {
  Valid = 0,
  InvalidEventSign,
  InvalidEventEnergy,
  InvalidCoordinate,
  EmptyCarrier,
  OutsideInverseImage,
  NonFiniteOutput,
};

struct RelativeActionPumpInput {
  std::int8_t event_sign = 0;
  double event_energy = 0.0;
  double canonical_q = 0.0;
  double canonical_p = 0.0;
};

struct RelativeActionPumpResult {
  RelativeActionTransducerStatus status =
      RelativeActionTransducerStatus::InvalidEventSign;
  std::int8_t event_sign = 0;
  double event_energy = 0.0;
  double canonical_q_before = 0.0;
  double canonical_p_before = 0.0;
  double action_before = 0.0;
  double radial_gain = 0.0;
  double canonical_q_after = 0.0;
  double canonical_p_after = 0.0;
  double action_after = 0.0;
  double action_residual = 0.0;
  double oriented_area = 0.0;
  double jacobian_determinant = 0.0;

  bool valid() const {
    return status == RelativeActionTransducerStatus::Valid;
  }
};

/** Apply the selected signed-quarter-turn action pump on the domain I>0. */
RelativeActionPumpResult pump_relative_action(
    const RelativeActionPumpInput& input);

struct RelativeActionInverseInput {
  std::int8_t event_sign = 0;
  double event_energy = 0.0;
  double canonical_q_after = 0.0;
  double canonical_p_after = 0.0;
};

struct RelativeActionInverseResult {
  RelativeActionTransducerStatus status =
      RelativeActionTransducerStatus::InvalidEventSign;
  double action_after = 0.0;
  double inverse_radial_gain = 0.0;
  double canonical_q_before = 0.0;
  double canonical_p_before = 0.0;
  double action_before = 0.0;
  double inverse_residual = 0.0;

  bool valid() const {
    return status == RelativeActionTransducerStatus::Valid;
  }
};

/**
 * Invert the pump when the erased event parameters (s,B) are supplied.
 *
 * This known-event inverse is not a claim that (s,B) can be recovered from
 * the unlabelled output pair.  The inverse image is the strict exterior
 * action_after > B because the forward reference domain excludes I=0.
 */
RelativeActionInverseResult invert_relative_action_pump(
    const RelativeActionInverseInput& input);

}  // namespace ftd::eft

