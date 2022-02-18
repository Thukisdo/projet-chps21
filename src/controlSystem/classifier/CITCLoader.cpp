#include "CITCLoader.hpp"

namespace fs = std::filesystem;
namespace tr = image::transform;

namespace control::classifier {
  namespace {

    /**
     * @brief Helper class for image loading and pipelining
     */
    class TransformationPipeline {
    public:
      /**
       * @brief Build a pipeline from a list of transformations
       * @param transformations
       */
      TransformationPipeline(std::initializer_list<tr::TransformEngine> transformations)
          : engines(transformations) {}

      /**
       * @brief Apply the pipeline to an image loaded from disk
       * @param image_path Path to the image to load
       * @return The transformed image
       */
      image::GrayscaleImage loadAndTransform(const fs::path &image_path) const;

    private:
      /**
       * @brief The list of transformations to apply when loading an image
       */
      std::vector<tr::TransformEngine> engines;
    };

    image::GrayscaleImage
    TransformationPipeline::loadAndTransform(const fs::path &image_path) const {
      image::GrayscaleImage image = image::ImageSerializer::load(image_path);
      for (auto &engine : engines) { engine.apply(image); }
      return image;
    }

    // Takes a grayscale image and load it to a clMatrix
    // The image is normalized using a given factor
    math::clMatrix normalizeToDevice(const image::GrayscaleImage &img, utils::clWrapper &wrapper,
                                     cl::CommandQueue &queue) {
      // Load the conversion kernel
      cl::Kernel kernel =
              wrapper.getKernel("kernels/NormalizeCharToFloat.cl", "normalizeCharToFloat");

      // Allocate a new cl matrix
      math::clMatrix res(img.getWidth() * img.getHeight(), 1, wrapper.getContext());

      // We need to load the image to the cl device before applying the kernel
      cl::Buffer img_buffer(wrapper.getContext(), CL_MEM_READ_ONLY,
                            img.getWidth() * img.getHeight());
      queue.enqueueWriteBuffer(img_buffer, CL_FALSE, 0, img.getWidth() * img.getHeight(),
                               img.getData());

      kernel.setArg(0, img_buffer);
      kernel.setArg(1, res.getBuffer());

      // We also normalize the matrix by a given factor
      kernel.setArg(2, 255);
      // OpenCL does not support size_t, so we cast it to unsigned long
      kernel.setArg(3, (cl_ulong) img.getSize());
      wrapper.getDefaultQueueHandler().enqueue(kernel, cl::NullRange, cl::NDRange(img.getSize()),
                                               cl::NullRange);
      return res;
    }

    std::vector<math::clMatrix> loadDirectory(const fs::path &directory,
                                              const TransformationPipeline &pipeline,
                                              utils::clWrapper &wrapper, cl::CommandQueue &queue) {
      std::vector<math::clMatrix> res;
      // We iterate on the directory, creating an OpenCL buffer for each image
      for (auto &file : fs::directory_iterator(directory)) {
        if (not file.is_regular_file()) continue;

        auto img = pipeline.loadAndTransform(file.path());
        auto cl_matrix = normalizeToDevice(img, wrapper, queue);
        res.push_back(cl_matrix);
      }
      return res;
    }
  }   // namespace


  // If the user didn't specify which classes to load, we load all of them
  // by associating each class with a directory
  void CITCLoader::loadClasses(fs::path const &input_path) {
    std::vector<std::filesystem::path> training_classes, eval_classes;

    for (auto const &p : fs::directory_iterator(input_path / "eval")) {
      if (p.is_directory()) training_classes.push_back(p.path().filename());
    }

    for (auto const &p : fs::directory_iterator(input_path / "train")) {
      if (p.is_directory()) eval_classes.push_back(p.path().filename());
    }

    std::sort(training_classes.begin(), training_classes.end());
    std::sort(eval_classes.begin(), eval_classes.end());

    // We expect both sets to have the same classes
    // So we throw an exception if they don't match
    if (not std::equal(training_classes.begin(), training_classes.end(), eval_classes.begin())) {
      tscl::logger("Training and evaluation classes are not the same", tscl::Log::Error);
      throw std::runtime_error("CITSLoader: train and eval classes are not the same");
    }

    classes = std::make_shared<CClassLabelSet>();
    for (auto const &p : training_classes) {
      ClassLabel tmp(classes->size(), p.filename().string());
      classes->append(tmp);
    }
  }

  // Load a set from an input directory
  // The directory should contain a subdirectory for each class.
  // Each class should contain a set of images.
  // Images are loaded as grayscale images and fed to the processing pipeline.
  //
  // Image -> preprocess -> resize -> postprocess -> OpenCL buffer on device
  //
  // During the openCL conversion, matrices are normalized by a given factor (255 so values are
  // contained between 0 and 1). This prevents exploding gradient during the training process
  void CITCLoader::loadSet(CTrainingSet &res, const std::filesystem::path &input_path,
                           utils::clWrapper &wrapper) {
    // Create an engine for the resize transformation
    tr::TransformEngine resize;
    resize.addTransformation(std::make_shared<tr::Resize>(target_width, target_height));
    // Instantiates the  pipeline
    TransformationPipeline pipeline({pre_process, resize, post_process});

    cl::Context context = wrapper.getContext();
    cl::CommandQueue queue(context);

    // Each class is a directory
    // inside the input path
    for (auto &c : *classes) {
      auto c_name = c.second.getName();
      fs::path target_path = input_path / c_name;

      // If for any reason the class directory doesn't exist, throw an error
      if (not fs::exists(target_path)) {
        tscl::logger("Class " + c_name + " not found in " + input_path.string(), tscl::Log::Error);
        throw std::runtime_error("CITSLoader: " + input_path.string() + " is missing class id: " +
                                 std::to_string(c.first) + " (\"" + c_name + "\")");
      }
      auto inputs = loadDirectory(target_path, pipeline, wrapper, queue);
      for (auto &i : inputs) { res.append(0, &c.second, std::move(i)); }
    }

    // Wait for all jobs to finish
    queue.finish();
  }

  std::unique_ptr<CTCollection> CITCLoader::load(const std::filesystem::path &input_path,
                                                 utils::clWrapper &wrapper) {
    if (not fs::exists(input_path) or not fs::exists(input_path / "eval") or
        not fs::exists(input_path / "train")) {
      tscl::logger("CITCLoader: " + input_path.string() + " is not a valid CITC directory",
                   tscl::Log::Error);
      throw std::invalid_argument(
              "ImageTrainingSetLoader: The input path does not exist or is invalid");
    }

    tscl::logger("Loading Classifier training collection from " + input_path.string(),
                 tscl::Log::Debug);

    // If the user didn't specify classes, we extract them from the input directory layout
    // Since each class must have one folder
    if (not classes) {
      tscl::logger("Looking for classes", tscl::Log::Debug);
      loadClasses(input_path);
      tscl::logger("Found " + std::to_string(classes->size()) + " classes", tscl::Log::Trace);
    }

    if (classes->empty()) {
      tscl::logger("CITCLoader: Need at-least one class, none were found", tscl::Log::Error);
      throw std::runtime_error("CITCLoader: Need at-least one class, none were found");
    }

    auto res = std::make_unique<CTCollection>(classes);

    // Load both sets
    loadSet(res->getEvalSet(), input_path / "eval", wrapper);
    loadSet(res->getTrainingSet(), input_path / "train", wrapper);

    tscl::logger("Loaded " + std::to_string(res->getTrainingSet().size()) + " inputs",
                 tscl::Log::Trace);

    return res;
  }


}   // namespace control::classifier