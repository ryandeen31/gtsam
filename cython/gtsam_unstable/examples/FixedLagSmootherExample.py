"""
Demonstration of the fixed-lag smoothers using a planar robot example
and multiple odometry-like sensors
Author: Frank Dellaert (C++), Jeremy Aguilon (Python)
"""

import numpy as np

import gtsam
import gtsam_unstable

# Create noise models
PRIOR_NOISE = gtsam.noiseModel_Diagonal.Sigmas(np.array([0.3, 0.3, 0.1]))
MEASUREMENT_NOISE = gtsam.noiseModel_Diagonal.Sigmas(np.array([0.1, 0.2]))

def _timestamp_key_value(key, value):
    return gtsam_unstable.FixedLagSmootherKeyTimestampMapValue(
        key, value
    )


def BatchFixedLagSmootherExample():
    # Define a batch fixed lag smoother, which uses
    # Levenberg-Marquardt to perform the nonlinear optimization
    lag = 2.0
    smoother_batch = gtsam_unstable.BatchFixedLagSmoother(lag)


    # Create containers to store the factors and linearization points
    # that will be sent to the smoothers
    new_factors = gtsam.NonlinearFactorGraph()
    new_values = gtsam.Values()
    new_timestamps = gtsam_unstable.FixedLagSmootherKeyTimestampMap()


    # Create  a prior on the first pose, placing it at the origin
    prior_mean = gtsam.Pose2(0, 0, 0)
    X1 = 0
    new_factors.push_back(gtsam.PriorFactorPose2(X1, prior_mean, PRIOR_NOISE))
    new_values.insert(X1, prior_mean)
    new_timestamps.insert(_timestamp_key_value(X1, 0.0))

    delta_time = 0.25
    time = 0.25

    while time <= 3.0:
        previous_key = 1000 * (time - delta_time)
        current_key = 1000 * time

        # assign current key to the current timestamp
        new_timestamps.insert(_timestamp_key_value(current_key, time))

        # Add a guess for this pose to the new values
        # Assume that the robot moves at 2 m/s. Position is time[s] * 2[m/s]
        current_pose = gtsam.Pose2(time * 2, 0, 0)
        new_values.insert(current_key, current_pose)

        # Add odometry factors from two different sources with different error stats
        odometry_measurement_1 = gtsam.Pose2(0.61, -0.08, 0.02)
        odometry_noise_1 = gtsam.noiseModel_Diagonal.Sigmas(np.array([0.1, 0.1, 0.05]))
        new_factors.push_back(gtsam.BetweenFactorPose2(
            previous_key, current_key, odometry_measurement_1, odometry_noise_1
        ))

        odometry_measurement_2 = gtsam.Pose2(0.47, 0.03, 0.01)
        odometry_noise_2 = gtsam.noiseModel_Diagonal.Sigmas(np.array([0.05, 0.05, 0.05]))
        new_factors.push_back(gtsam.BetweenFactorPose2(
            previous_key, current_key, odometry_measurement_2, odometry_noise_2
        ))

        # Update the smoothers with the new factors
        smoother_batch.update(new_factors, new_values, new_timestamps)

        print("Timestamp = " + str(time) + ", Key = " + str(current_key))
        print(smoother_batch.calculateEstimatePose2(current_key))

        new_timestamps.clear()
        new_values.clear()
        new_factors.resize(0)


        time += delta_time

if __name__ == '__main__':
    BatchFixedLagSmootherExample()
    print("Example complete")
