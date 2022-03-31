#include "clFTensor.hpp"

namespace math {

  clFTensor::clFTensor(size_t x, size_t y, size_t z) : x_dim(x), y_dim(y), z_dim(z) {
    size_t size = x * y * z;
    data = cl::Buffer(CL_MEM_READ_WRITE, size * sizeof(float));

    // By default, OpenCL only migrates data to the device when it is needed.
    // Since we only use sub-buffers, we need to explicitly migrate the data
    // to the device, otherwise the data will remain in the host memory.
    auto queue = cl::CommandQueue::getDefault();
    queue.enqueueMigrateMemObjects({data}, 0);
  }

  clFMatrix clFTensor::getMatrix(size_t z) {
    if (z > z_dim) { throw std::out_of_range("z index out of range"); }

    size_t size = x_dim * y_dim;
    cl_buffer_region region = {z * size * sizeof(float), size * sizeof(float)};
    cl::Buffer subbuffer =
            data.createSubBuffer(CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region);

    return {subbuffer, x_dim, y_dim};
  }

  std::vector<clFMatrix> clFTensor::getMatrices() {
    std::vector<clFMatrix> matrices;
    for (size_t i = 0; i < z_dim; i++) { matrices.push_back(getMatrix(i)); }
    return matrices;
  }

}   // namespace math
