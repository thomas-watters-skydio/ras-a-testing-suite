#include <gtest/gtest.h>
#include <sys/time.h>
#include <filesystem>
#include "../environment.hpp"
#include <curl/curl.h>
using namespace RASATestingSuite;

inline uint64_t micros() {
    struct timeval t;
    gettimeofday(&t, nullptr);
    return t.tv_sec * ((uint64_t)1000000) + t.tv_usec;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

class Camera : public ::testing::Test {
protected:
    const std::shared_ptr<mavsdk::Ftp> ftp;
    const std::shared_ptr<PassthroughTester> link;
    const TestTargetAddress target;


    Camera() :
          ftp(Environment::getInstance()->getFtpPlugin()),
          link(Environment::getInstance()->getPassthroughTester()),
          target(Environment::getInstance()->getTargetAddress()) {
        link->flushAll();
    }

    template <int MSG>
    typename msg_helper<MSG>::decode_type requestMessageCommand() {
        // make sure to flush all existing messages
        link->flush<MSG>(target);
        link->send<COMMAND_LONG>(target,
                                 MAV_CMD_REQUEST_MESSAGE, 0,
                                 static_cast<float>(msg_helper<MSG>::ID), NAN, NAN, NAN, NAN, NAN, NAN);
        auto ack = link->receive<COMMAND_ACK>(target);
        EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
        EXPECT_EQ(ack.command, MAV_CMD_REQUEST_MESSAGE);
        return link->receive<MSG>(target, 1000);
    }
};

TEST_F(Camera, RequestCameraInformation) {
    auto conf = Environment::getInstance()->getConfig({"Camera", "RequestCameraInformation"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<CAMERA_INFORMATION>();
}

TEST_F(Camera, RequestCameraSettings) {
    auto conf = Environment::getInstance()->getConfig({"Camera", "RequestCameraSettings"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<CAMERA_SETTINGS>();
}

TEST_F(Camera, SetCameraMode) {
    auto conf = Environment::getInstance()->getConfig({"Camera", "SetCameraMode"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    link->send<COMMAND_LONG>(target, MAV_CMD_SET_CAMERA_MODE, 0, 0, CAMERA_MODE_IMAGE, NAN, NAN, NAN, NAN, NAN);
    auto ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack.command, MAV_CMD_SET_CAMERA_MODE);
}

TEST_F(Camera, RequestStorageInformation) {
    auto conf = Environment::getInstance()->getConfig({"Camera", "RequestStorageInformation"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<STORAGE_INFORMATION>();
}

TEST_F(Camera, CaptureImage) {
    auto conf = Environment::getInstance()->getConfig({"Camera", "CaptureImage"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }

    // demand to capture 3 images with 1s in between
    link->send<COMMAND_LONG>(target, MAV_CMD_IMAGE_START_CAPTURE, 0, 0, 1.f, 3, 0, NAN, NAN, NAN);
    auto ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack.command, MAV_CMD_IMAGE_START_CAPTURE);

    int last_interval_time = micros();

    for (int i=0; i<3; i++) {
        link->receive<CAMERA_IMAGE_CAPTURED>(target, 2000);
        int interval = micros() - last_interval_time;
        last_interval_time = micros();

        EXPECT_GT(interval, 900000) << "Camera picture timing incorrect";
        EXPECT_LT(interval, 1100000) << "Camera picture timing incorrect";
    }

    link->send<COMMAND_LONG>(target, MAV_CMD_IMAGE_STOP_CAPTURE, 0, 0, 0, 0, 0, NAN, NAN, NAN);
    auto ack_stop = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack_stop.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack_stop.command, MAV_CMD_IMAGE_STOP_CAPTURE);

    try {
        link->receive<CAMERA_IMAGE_CAPTURED>(target, 2000);
        FAIL() << "More pictures taken than expected";
    } catch(TimeoutError&) {}
}


TEST_F(Camera, CaptureVideo) {
    auto conf = Environment::getInstance()->getConfig({"Camera", "CaptureVideo"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }

    // demand to capture 3 images with 1s in between
    link->send<COMMAND_LONG>(target, MAV_CMD_VIDEO_START_CAPTURE, 0, 1, 5, NAN, NAN, NAN, NAN, NAN);
    auto ack = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack.command, MAV_CMD_VIDEO_START_CAPTURE);

    for (int i=0; i<3; i++) {
        link->receive<CAMERA_CAPTURE_STATUS>(target, 2000);
    }

    link->send<COMMAND_LONG>(target, MAV_CMD_VIDEO_STOP_CAPTURE, 0, 0, 0, 0, 0, NAN, NAN, NAN);
    auto ack_stop = link->receive<COMMAND_ACK>(target);
    EXPECT_EQ(ack_stop.result, MAV_RESULT_ACCEPTED);
    EXPECT_EQ(ack_stop.command, MAV_CMD_VIDEO_STOP_CAPTURE);

    // we should no longer get image capture notifications
    try {
        link->receive<CAMERA_CAPTURE_STATUS>(target, 2000);
        FAIL() << "More pictures taken than expected";
    } catch(TimeoutError&) {}
}


TEST_F(Camera, RequestVideoStreamInformation) {
    auto conf = Environment::getInstance()->getConfig({"Camera", "RequestVideoStreamInformation"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    requestMessageCommand<VIDEO_STREAM_INFORMATION>();
}

TEST_F(Camera, DownloadCameraDefintionFile) {
    auto conf = Environment::getInstance()->getConfig({"Camera", "DownloadCameraDefintionFile"});
    if (!conf || conf["skip"].as<bool>(false)) {
        GTEST_SKIP();
    }
    auto cam_information = requestMessageCommand<CAMERA_INFORMATION>();
    std::string cam_definition_uri{cam_information.cam_definition_uri};
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();

    std::filesystem::path downloaded_file = temp_dir /
                                            cam_definition_uri.substr(
                                                cam_definition_uri.find_last_of('/') + 1);
    std::filesystem::remove(downloaded_file);

    if (cam_definition_uri.rfind("mftp", 0) == 0) {
        // using mavlink ftp to download
        auto prom = std::promise<mavsdk::Ftp::Result>{};
        auto future = prom.get_future();
        ftp->download_async(
            cam_definition_uri, temp_dir,
            [&prom](const mavsdk::Ftp::Result result, const mavsdk::Ftp::ProgressData& progress) {
                (void)progress;
                if (result != mavsdk::Ftp::Result::Next) {
                    prom.set_value(result);
                }
            });
        future.wait_for(std::chrono::seconds(5));
        auto res = future.get();
        ASSERT_EQ(res, mavsdk::Ftp::Result::Success);

    } else if (cam_definition_uri.rfind("http", 0) == 0) {
        CURL * curl;
        CURLcode res;

        curl = curl_easy_init();
        if (curl) {
            FILE* fp = fopen(downloaded_file.c_str(), "wb");
            curl_easy_setopt(curl, CURLOPT_URL, cam_definition_uri.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            fclose(fp);
            if (res != 0) {
                FAIL() << "http download failed";
            }
        } else {
            FAIL() << "CURL failed";
        }
    } else {
        FAIL() << "cam_definition_uri does not specify mftp or http based transport URI";
    }


}

