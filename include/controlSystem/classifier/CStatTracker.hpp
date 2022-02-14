
#pragma once
#include "CTrainingSet.hpp"
#include "Matrix.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <vector>

namespace control::classifier {

  /** @brief Shared state that is used for dumping metrics to file
   *
   */
  class CTrackerState {
  public:
    CTrackerState(std::filesystem::path const &output_path, const CClassLabelSet &class_labels) {
      auto output_dir = output_path / "class_data";
      std::filesystem::create_directories(output_dir);

      avg_prec_output = std::make_shared<std::ofstream>(output_dir / "avg_prec.dat");
      avg_recall_output = std::make_shared<std::ofstream>(output_dir / "avg_recall.dat");
      avg_f1_output = std::make_shared<std::ofstream>(output_dir / "avg_f1.dat");

      prec_outputs = std::make_shared<std::vector<std::ofstream>>();
      recall_outputs = std::make_shared<std::vector<std::ofstream>>();
      f1_outputs = std::make_shared<std::vector<std::ofstream>>();

      for (auto &c : class_labels) {
        auto c_name = c.second.getName();
        prec_outputs->emplace_back(output_dir / (c_name + "_prec.dat"));
        recall_outputs->emplace_back(output_dir / (c_name + "_recall.dat"));
        f1_outputs->emplace_back(output_dir / (c_name + "_f1.dat"));
      }
    }

    std::ostream &getAvgPrecOutput() { return *avg_prec_output; }
    std::ostream &getAvgRecallOutput() { return *avg_recall_output; }
    std::ostream &getAvgF1Output() { return *avg_f1_output; }

    std::ostream &getPrecOutput(size_t index) {
      if (not prec_outputs or index > prec_outputs->size())
        throw std::invalid_argument("CTrackerState: Invalid stream index");
      return prec_outputs->at(index);
    }

    std::ostream &getRecallOutput(size_t index) {
      if (not recall_outputs or index > recall_outputs->size())
        throw std::invalid_argument("CTrackerState: Invalid stream index");
      return recall_outputs->at(index);
    }

    std::ostream &getF1Output(size_t index) {
      if (not f1_outputs or index > f1_outputs->size())
        throw std::invalid_argument("CTrackerState: Invalid stream index");
      return f1_outputs->at(index);
    }

  private:
    std::shared_ptr<std::ofstream> avg_prec_output;
    std::shared_ptr<std::ofstream> avg_recall_output;
    std::shared_ptr<std::ofstream> avg_f1_output;

    std::shared_ptr<std::vector<std::ofstream>> prec_outputs;
    std::shared_ptr<std::vector<std::ofstream>> recall_outputs;
    std::shared_ptr<std::vector<std::ofstream>> f1_outputs;
  };

  class CStatTracker;

  /** @brief Containers for classifier metrics and serialization
   *
   */
  class CStats {
    friend CStatTracker;
    friend std::ostream &operator<<(std::ostream &os, CStats const &stat);

  public:
    CStats() = delete;
    void dumpToFiles();

    [[nodiscard]] double getAvgPrec() const { return avg_precision; }
    [[nodiscard]] double getAvgRecall() const { return avg_recall; }
    [[nodiscard]] double getAvgF1() const { return avg_f1score; }

  private:
    CStats(size_t epoch, math::Matrix<size_t> const &confusion,
           std::shared_ptr<CTrackerState> state);

    size_t epoch;
    std::shared_ptr<CTrackerState> state;
    math::DoubleMatrix stats;
    double avg_precision, avg_recall, avg_f1score;
  };

  std::ostream &operator<<(std::ostream &os, CStats const &stat);


  /** @brief Helper class to optimize classifier metrics, including the f1_score
   *
   */
  class CStatTracker {
  public:
    CStatTracker(std::filesystem::path const &output_path, const CClassLabelSet &class_labels)
        : epoch(0), state(std::make_shared<CTrackerState>(output_path, class_labels)) {}

    [[nodiscard]] CStats computeStats(math::Matrix<size_t> const &confusion) {
      return {epoch, confusion, state};
    }

    [[nodiscard]] size_t getEpoch() const { return epoch; }
    void nextEpoch() { epoch++; }

  private:
    std::shared_ptr<CTrackerState> state;
    size_t epoch;
  };
}   // namespace control::classifier