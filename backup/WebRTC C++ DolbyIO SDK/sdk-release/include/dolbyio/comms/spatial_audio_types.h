#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/exports.h>

#include <string>
#include <variant>
#include <vector>

namespace dolbyio::comms {

/**
 * @brief The participant's position in space defined using Cartesian
 * coordinates.
 */
struct DLB_COMMS_EXPORT spatial_position {
  /**
   * @brief The constructor for spatial position.
   * @param x The x-coordinate of a new audio location.
   * @param y The y-coordinate of a new audio location.
   * @param z The z-coordinate of a new audio location.
   */
  spatial_position(double x, double y, double z) : x(x), y(y), z(z) {}

  double x; /**< The x-coordinate of a new audio location..*/
  double y; /**< The y-coordinate of a new audio location. */
  double z; /**< The z-coordinate of a new audio location. */
};

/**
 * @brief  The participant's position in a spherical coordinate system.
 */
struct DLB_COMMS_EXPORT polar_position {
  /**
   * @brief The constructor for the polar position in spherical coordinate
   * system.
   * @param azimuth The angle of rotation from the initial meridian plane.
   * @param elevation The polar angle to the point, angle with respect to the
   * polar axis.
   * @param distance The radial distance to the point from the origin.
   */
  polar_position(double azimuth, double elevation, double distance)
      : azimuth(azimuth), elevation(elevation), distance(distance) {}

  double azimuth;   /**< The angle of rotation from the initial meridian plane. */
  double elevation; /**< The polar angle to the point, angle with respect to the
                       polar axis. */
  double distance;  /**<  The radial distance to the point from the origin. */
};

/**
 * @brief The direction a participant is
 * facing in space. The spatial direction is specified as a set
 * of three Euler rotations about the corresponding
 * axis in the order of z-x-y.
 */
struct DLB_COMMS_EXPORT spatial_direction {
  /**
   * @brief The constructor taking the three Euler rotations.
   * @param x A rotation about the x-axis.
   * @param y A rotation about the y-axis.
   * @param z A rotation about the z-axis.
   */
  spatial_direction(double x, double y, double z) : x(x), y(y), z(z) {}

  double x; /**< A rotation about the x-axis. */
  double y; /**< A rotation about the y-axis. */
  double z; /**< A rotation about the z-axis. */
};

/**
 * @brief Information on how to convert units from the application's
 * coordinate system (pixels or centimeters) into meters used by the spatial
 * audio coordinate system.
 */
struct DLB_COMMS_EXPORT spatial_scale {
  /**
   * @brief The constructor taking the three scale parameters creating the scale
   * vector.
   * @param x The x component of the vector.
   * @param y The y component of the vector.
   * @param z The z component of the vector.
   */
  spatial_scale(double x, double y, double z) : x(x), y(y), z(z) {}

  double x; /**< The x component of the vector. */
  double y; /**< The y component of the vector. */
  double z; /**< The z component of the vector. */
};

/**
 * @brief Batched spatial audio updates that can be passed
 * to the conference service.
 *
 * This class contains the ordered list of spatial audio updates which will be
 * performed atomically in the conference. If you wish to
 * update multiple spatial audio parameters, for example
 * multiple spatial positions in the individual mode, use this method to batch
 * all relevant updates in a single batch update object. This approach prevents
 * delays between the application of the provided parameters.
 */
class DLB_COMMS_EXPORT spatial_audio_batch_update {
 public:
  /**
   * @brief A structure describing the position of a conference participant.
   */
  struct position {
    /**
     * @brief The constructor taking the participant ID and the preferred
     * spatial position of the participant.
     * @param participant_id The participant ID.
     * @param pos The preferred position for the participant.
     */
    position(const std::string& participant_id, const spatial_position& pos)
        : participant_id(participant_id), pos(pos) {}

    std::string participant_id; /**< The participant ID. */
    spatial_position pos;       /**< The preferred position for the participant. */
  };
  /**
   * @brief A structure describing the spatial environment of an application, so
   * the audio renderer understands which directions the application considers
   * forward, up, and right and which units it uses for distance.
   */
  struct environment {
    /**
     * @brief The constructor taking the spatial scale, and spatial positions
     * for the three directions.
     * @param scale A scale that defines how to convert units from the
     * coordinate system of an application (pixels or centimeters) into meters
     * used by the spatial audio coordinate system.
     * @param forward A vector describing the direction the application
     * considers as forward. The value can be either +1, 0, or -1 and must be
     * orthogonal to up and right.
     * @param up A vector describing the direction the application considers as
     * up. The value can be either +1, 0, or -1 and must be orthogonal to
     * forward and right.
     * @param right A vector describing the direction the application considers
     * as right. The value can be either +1, 0, or -1 and must be orthogonal to
     * forward and up.
     */
    environment(const spatial_scale& scale,
                const spatial_position& forward,
                const spatial_position& up,
                const spatial_position& right)
        : scale(scale), forward(forward), up(up), right(right) {}

    spatial_scale scale;      /**< A scale that defines how to convert units from the
                                 coordinate system of an application (pixels or centimeters)
                                 into meters used by the spatial audio coordinate system. */
    spatial_position forward; /**< A vector describing the direction the application
                                 considers as forward. The value can be either +1, 0, or -1
                                 and must be orthogonal to up and right. */
    spatial_position up;      /**< A vector describing the direction the application
                                 considers as up. The value can be either +1, 0, or
                                 -1 and must be orthogonal to forward and right. */
    spatial_position right;   /**< A vector describing the direction the application considers
                                 as right. The value can be either +1, 0, or -1 and must be
                                 orthogonal to forward and up. */
  };

  /**
   * @brief A helper for defining a variant for position, direction, and
   * environment.
   */
  using action = std::variant<position, spatial_direction, environment>;

  /**
   * @brief Sets a remote participant's position in space.
   *  If the remote participant does not have an established location,
   *  the participant does not have a default position and will remain
   *  muted until a position is specified.
   *  @param participant_id The ID of the remote participant.
   *  @param position The position of the remote participant.
   */
  void set_spatial_position(const std::string& participant_id, const spatial_position& position);

  /**
   * @brief Sets the direction the local participant is facing in space.
   * @param direction The direction the participant is facing.
   */
  void set_spatial_direction(const spatial_direction& direction);

  /**
   * @brief Configures a spatial environment of an application, so the audio
   * renderer understands which directions the application considers forward,
   * up, and right and which units it uses for distance.
   *
   * @attention Setting spatial environment causes invalidating all
   * participants' positions and the local participant's direction. If no new
   * values for positions, local participant's position in the shared
   * more, or local direction are provided, the previously supplied values are
   * reused and translated against the new environment. This is not the
   * desired behaviour, because the original values provided by the application
   * were intended to be used with the previously set environment, not
   * the new one. If the environment update is required,
   * it should be the first operation in the batch that should be followed with
   * the update of local participant's direction and participants' positions
   * in the same batch object.
   *
   * @param scale A scale that defines how to convert units from the
   * coordinate system of an application (pixels or centimeters) into meters
   * used by the spatial audio coordinate system.
   * @param forward A vector describing the direction the application
   * considers as forward. The value can be either +1, 0, or -1 and must be
   * orthogonal to up and right.
   * @param up A vector describing the direction the application considers as
   * up. The value can be either +1, 0, or -1 and must be orthogonal to forward
   * and right.
   * @param right A vector describing the direction the application considers
   * as right. The value can be either +1, 0, or -1 and must be orthogonal to up
   * and forward.
   */
  void set_spatial_environment(const spatial_scale& scale,
                               const spatial_position& forward,
                               const spatial_position& up,
                               const spatial_position& right);

  /**
   * @brief Gets the const reference to current actions which were applied this
   * spatial_audio_batch_update.
   * @return Reference to currently set actions.
   */
  const std::vector<action>& get_actions() const { return actions_; }

  /**
   * @brief Gets an R-value reference to the current actions that were applied
   * to the spatial_audio_batch_update. This API moves the actions in the
   * process.
   * @return The R-value reference to the currently set actions.
   */
  std::vector<action>&& move_actions() && { return std::move(actions_); }

 private:
  std::vector<action> actions_{};
};

}  // namespace dolbyio::comms
