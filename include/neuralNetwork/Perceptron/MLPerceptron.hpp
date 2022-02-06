#pragma once

#include "Matrix.hpp"
#include "Utils.hpp"
#include "neuralNetwork/ActivationFunction.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace nnet {

  class MLPTopology {
    friend std::ostream &operator<<(std::ostream &os, const MLPTopology &topology);

  public:
    MLPTopology() = default;
    MLPTopology(std::initializer_list<size_t> list) : layers(list) {}
    explicit MLPTopology(std::vector<size_t> sizes) : layers(std::move(sizes)) {}


    size_t &operator[](size_t i) {
      if (i > layers.size()) { throw std::out_of_range("Index out of range"); }
      return layers[i];
    }

    size_t const &operator[](size_t i) const {
      if (i > layers.size()) { throw std::out_of_range("Index out of range"); }
      return layers[i];
    }

    [[nodiscard]] size_t getInputSize() const { return layers.empty() ? 0 : layers.front(); }
    void setInputSize(size_t i) { layers.front() = i; }

    [[nodiscard]] size_t getOutputSize() const { return layers.empty() ? 0 : layers.back(); }
    void setOutputSize(size_t i) { layers.back() = i; }
    void push_back(size_t i) { layers.push_back(i); }

    [[nodiscard]] bool empty() const { return layers.empty(); }
    [[nodiscard]] size_t size() const { return layers.size(); }

    using iterator = std::vector<size_t>::iterator;
    using const_iterator = std::vector<size_t>::const_iterator;

    iterator begin() { return layers.begin(); }
    iterator end() { return layers.end(); }

    static MLPTopology fromString(const std::string &str);
    [[nodiscard]] const_iterator begin() const { return layers.begin(); }
    [[nodiscard]] const_iterator end() const { return layers.end(); }

  private:
    std::vector<size_t> layers;
  };

  /**
   * @brief Base interface for MLPerceptron
   *
   */
  class MLPBase {
  public:
    MLPBase() = default;

    virtual ~MLPBase() = default;

    virtual void setTopology(MLPTopology const &topology) = 0;
    [[nodiscard]] MLPTopology const &getTopology() const { return topology; }

    virtual void randomizeWeight() = 0;

  protected:
    MLPTopology topology;
  };

  /**
   * @brief A neural network that supports most fp precision as template
   * parameters
   * 
   */
  class MLPerceptron final : public MLPBase {
  public:

    /**
     * @brief Construct a new Neural Network object with no layer
     *
     */
    MLPerceptron() = default;

    MLPerceptron(const MLPerceptron &other) = default;

    MLPerceptron(MLPerceptron &&other) noexcept = default;

    MLPerceptron &operator=(const MLPerceptron &) = default;
    MLPerceptron &operator=(MLPerceptron &&other) noexcept = default;

    /** @brief Runs the neural network on the inputs
     * The outputs are returned as a matrix of reals
     *
     * @tparam iterator
     * @param begin
     * @param end
     * @return
     */
    math::FloatMatrix predict(math::FloatMatrix const &input) const;

    /**
     * @brief Take a vector of sizes correspondig to the number of neurons
     * in each layer and build the network accordingly. Note that weights are not
     * initialized after this.
     *
     * @param layers
     */
    void setTopology(MLPTopology const &topology) override;
    void setActivationFunction(af::ActivationFunctionType type) {
      for (auto &activation_function : activation_functions) { activation_function = type; }
    }

    void setActivationFunction(af::ActivationFunctionType af, size_t layer) {
      if (layer >= weights.size()) { throw std::invalid_argument("Invalid layer"); }

      activation_functions[layer] = af;
    }

    [[nodiscard]] const std::vector<af::ActivationFunctionType> &getActivationFunctions() const {
      return activation_functions;
    }

    /**
     * @brief Randomizes the weights and biases of the network
     *
     * @param seed
     */
    void randomizeWeight() override;

    [[nodiscard]] std::vector<math::FloatMatrix> &getWeights() { return weights; }

    [[nodiscard]] const std::vector<math::FloatMatrix> &getWeights() const { return weights; }

    [[nodiscard]] std::vector<math::FloatMatrix> &getBiases() { return biases; }

    [[nodiscard]] const std::vector<math::FloatMatrix> &getBiases() const { return biases; }


  private:
    std::vector<math::FloatMatrix> weights;
    std::vector<math::FloatMatrix> biases;

    // We want every layer to have its own activation function
    std::vector<af::ActivationFunctionType> activation_functions;
  };

  // std::ostream& operator<<(std::ostream& os, const Pair<T, U>& p)
  std::ostream &operator<<(std::ostream &os, const MLPerceptron &nn);

}   // namespace nnet