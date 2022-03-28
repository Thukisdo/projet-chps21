#pragma once

#include "Model.hpp"
#include "neuralNetwork/CNN/CNN.hpp"
#include "neuralNetwork/Perceptron/MLPerceptron.hpp"

namespace cnnet {
  using namespace nnet;
  using namespace math;

  class CNNModel : public Model {
    friend class CNNModelFactory;

  public:
    CNNModel(std::shared_ptr<utils::clWrapper> wrapper_ptr);

    /**
     * @brief Builds a new CNN model with the given topology
     * @param topology
     * @param mlp_topology The topology of the MLP without the first layer
     * @param wrapper_ptr
     * @return
     */
    static std::unique_ptr<CNNModel> random(CNNTopology const &topology, MLPTopology &mlp_topology,
                                            std::shared_ptr<utils::clWrapper> wrapper_ptr);

    CNNModel(const CNNModel &) = delete;
    CNNModel(CNNModel &&other) noexcept : Model(std::move(other)) {
      cnn = std::move(other.cnn);
      mlp = std::move(other.mlp);
      flatten = std::move(other.flatten);
    };

    /**
     * @brief Save the model to the given path
     * @param path The path where the model should be saved
     * Note that the model saving format may use multiple files
     * @return True if the model was saved successfully, false otherwise
     */
    bool save(const std::filesystem::path &path) const override {
      assert(false && "Not implemented");
      return false;
    }

    /**
     * @brief Replace this model by one loaded from the given path
     * If loading fails, this model will be left unchanged
     * @param path The path where the model should be loaded
     * @return True if the model was loaded successfully, false otherwise
     */
    bool load(const std::filesystem::path &path) override {
      assert(false && "Not implemented");
      return false;
    }

    math::clFMatrix predict(math::clFMatrix const &input) const override;

    [[nodiscard]] CNN &getCnn() { return *cnn; }
    [[nodiscard]] CNN const &getCnn() const { return *cnn; }

    [[nodiscard]] clFMatrix &getFlatten() { return flatten; }
    [[nodiscard]] clFMatrix const &getFlatten() const { return flatten; }

    [[nodiscard]] MLPerceptron &getMlp() { return *mlp; }
    [[nodiscard]] MLPerceptron const &getMlp() const { return *mlp; }

  private:
    std::unique_ptr<CNN> cnn;
    std::unique_ptr<MLPerceptron> mlp;

    // TODO : Remove me
    clFMatrix flatten;
  };

}   // namespace cnnet