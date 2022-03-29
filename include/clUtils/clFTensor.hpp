#pragma once
#include "clFMatrix.hpp"

namespace math {

  /**
   * @brief Represents a tensor of clFMatrix.
   * Each matrix is  of size x * y, the z dimension is the number of matrices.
   *
   * This class can be used for efficient batched matrix multiplication.
   */
  class clFTensor {
  public:
    clFTensor(size_t x, size_t y, size_t z);

    /**
     * @brief Returns the submatrix at the given index.
     * Note that the returned matrix is a view of the internal matrix, and any change made to it
     * will be reflected in the tensor
     *
     * Submatrices created this way remains valid even if the tensor is destroyed, since openCl
     * keeps track of buffers and subbuffers.
     *
     * @param z The index of the submatrix.
     * @return A submatrix inside the tensor, throws on error
     */
    clFMatrix getMatrix(size_t z);

    /**
     * @brief Returns an array of submatrices.
     *
     * * Submatrices created this way remains valid even if the tensor is destroyed, since openCl
     * keeps track of buffers and subbuffers.
     * @return An array of submatrices inside the tensor
     */
    std::vector<clFMatrix> getMatrices();

    /**
     * @brief Get the x dimension of the tensor, corresponding to the number of columns in each
     * matrix
     * @return
     */
    size_t getX() const { return x_dim; }

    /**
     * @brief Get the y dimension of the tensor, corresponding to the number of rows in each matrix
     * @return
     */
    size_t getY() const { return y_dim; }

    /**
     * @brief Get the z dimension of the tensor, corresponding to the number of matrices
     * @return
     */
    size_t getZ() const { return z_dim; }

  private:
    cl::Buffer data;
    size_t x_dim;
    size_t y_dim;
    size_t z_dim;
  };

}   // namespace math
