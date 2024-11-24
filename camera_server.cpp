#include <libcamera/libcamera.h>
#include "lib/httplib.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <memory>
#include <thread>

using namespace libcamera;
using namespace std::chrono_literals;

static std::shared_ptr<Camera> camera;

int main() {
    // Initialize the Camera Manager
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    // Print out available cameras
    for (auto const &camera : cm->cameras())
        std::cout << "camera ID: " << camera->id() << std::endl;

    // Check if any cameras were found
    auto cameras = cm->cameras();
    if (cameras.empty()) {
        std::string error_msg = "No cameras were identified on the system.";
        std::cout << error_msg << std::endl;
        cm->stop();
        return EXIT_FAILURE;
    }

    // Get the first camera's ID
    std::string cameraId = cameras[0]->id();
    auto camera = cm->get(cameraId);

    // Acquire the camera, moving it to the "Acquired" state
    int ret = camera->acquire();
    if (ret < 0) {
        std::cerr << "Failed to acquire camera" << std::endl;
        return ret;
    }

    // Generate the camera configuration for video recording stream
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({StreamRole::VideoRecording});
    StreamConfiguration &streamConfig = config->at(0);
    std::cout << "Default viewfinder configuration is: " << streamConfig.toString() << std::endl;

    // Validate the configuration
    config->validate();
    std::cout << "Validated viewfinder configuration is: " << streamConfig.toString() << std::endl;

    // Configure the camera
    ret = camera->configure(config.get());
    if (ret < 0) {
        std::cerr << "Failed to configure camera" << std::endl;
        camera->release();  // Release the camera resources in case of failure
        return ret;
    }

    // Allocate buffers for the configured stream
    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);
    for (StreamConfiguration &cfg : *config) {
        ret = allocator->allocate(cfg.stream());
        if (ret < 0) {
            std::cerr << "Can't allocate buffers" << std::endl;
            return -ENOMEM;
        }

        size_t allocated = allocator->buffers(cfg.stream()).size();
        std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
    }

	Stream *stream = streamConfig.stream();
	const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
	std::vector<std::unique_ptr<Request>> requests;


	//Release the camera resources when done
	camera->release();

    return 0;
}
