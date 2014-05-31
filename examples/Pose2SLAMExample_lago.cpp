/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file Pose2SLAMExample_lago.cpp
 * @brief A 2D Pose SLAM example that reads input from g2o, and solve the Pose2 problem
 * using LAGO (Linear Approximation for Graph Optimization). See class lago.h
 * Output is written on a file, in g2o format
 * Syntax for the script is ./Pose2SLAMExample_lago input.g2o output.g2o
 * @date May 15, 2014
 * @author Luca Carlone
 */

#include <gtsam/nonlinear/lago.h>
#include <gtsam/slam/dataset.h>
#include <gtsam/slam/PriorFactor.h>
#include <fstream>

using namespace std;
using namespace gtsam;

int main(const int argc, const char *argv[]) {

  // Read graph from file
  string g2oFile;
  if (argc < 2)
    g2oFile = "../../examples/Data/noisyToyGraph.txt";
  else
    g2oFile = argv[1];

  NonlinearFactorGraph graph;
  Values initial;
  readG2o(g2oFile, graph, initial);

  // Add prior on the pose having index (key) = 0
  NonlinearFactorGraph graphWithPrior = graph;
  noiseModel::Diagonal::shared_ptr priorModel = //
      noiseModel::Diagonal::Variances((Vector(3) << 1e-6, 1e-6, 1e-8));
  graphWithPrior.add(PriorFactor<Pose2>(0, Pose2(), priorModel));
  graphWithPrior.print();

  std::cout << "Computing LAGO estimate" << std::endl;
  Values estimateLago = lago::initialize(graphWithPrior);
  std::cout << "done!" << std::endl;

  if (argc < 3) {
    estimateLago.print("estimateLago");
  } else {
    const string outputFile = argv[2];
    std::cout << "Writing results to file: " << outputFile << std::endl;
    writeG2o(outputFile, graph, estimateLago);
    std::cout << "done! " << std::endl;
  }

  return 0;
}
