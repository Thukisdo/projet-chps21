#pragma once

#include "MLPModel.hpp"

namespace nnet {

  /** Utility class for serializing and deserializing MLPModels
   *
   *
   */
  class Utf8MLPModelSerializer {
  public:
    /**
     * @returns a new MLPModelSerializer, or nullptr on failure
     */
    virtual MLPModel<float> readFromFile(const std::filesystem::path &path) = 0;

    /**
     * @returns a new MLPModelSerializer, or nullptr on failure
     */
    virtual MLPModel<float> readFromStream(std::istream &stream) = 0;

    /**
     *
     * @param path Path to the file to write to
     * @param model The model to write
     * @return true on success, false on failure
     */
    virtual bool writeToFile(const std::filesystem::path &path, const MLPModel<float> &model) = 0;

    /**
     *
     * @param stream The stream to write to
     * @param model The model to write
     * @return true on success, false on failure
     */
    virtual bool writeToStream(std::ostream &stream, const MLPModel<float> &model) = 0;
  };
}   // namespace nnet