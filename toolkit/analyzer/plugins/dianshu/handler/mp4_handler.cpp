#include "ocall_def.h"
#include "ypc/core/byte.h"

#include <cmath>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>
#include <random>

define_nt(video_frame, std::vector<unsigned char>);
define_nt(total_duration, double);
define_nt(frame_ts, double);
typedef ::ff::util::ntobject<total_duration, video_frame, frame_ts>
    video_frame_t;

int random_get_frame_index(int total_frames, int frame_capture_len) {
  (void)frame_capture_len;
  if (total_frames <= 0) {
    return -1;
  }
  std::vector<int> frame_index;
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> dist(0, total_frames - 1);
  return dist(rng);
}

uint32_t handle_mp4(const ypc::bytes &in, ypc::bytes &out) {
  LOG(INFO) << "handle mp4";
  std::string ifs((const char *)in.data(), in.size());
  cv::VideoCapture cap(ifs);
  // check if the file was opened successfully
  if (!cap.isOpened()) {
    LOG(ERROR) << "Error: Could not open the MP4 file.";
    return 1;
  }

  int total_frames = cap.get(cv::CAP_PROP_FRAME_COUNT); // 获取视频总帧数
  LOG(INFO) << "Frame count: " << total_frames;
  double fps = cap.get(cv::CAP_PROP_FPS); // 获取视频帧率
  if (total_frames <= 0 || !std::isfinite(fps) || fps <= 0.0) {
    LOG(ERROR) << "Invalid frame count or FPS";
    return 1;
  }
  double total_duration =
      total_frames / fps; // 计算视频总时长 = 视频总帧数 / 视频帧率
  LOG(INFO) << "Total time (duration) of the video: " << total_duration
            << " seconds";
  const int frame_capture_len = 1;
  int frame_index =
      random_get_frame_index(total_frames, frame_capture_len); // 随机选取一帧
  int max_tries = 8;
  cv::Mat frame; // 用于存储视频帧数据
  while (max_tries > 0) {
    LOG(INFO) << "Frame index: " << frame_index;
    // read frame
    cap.set(cv::CAP_PROP_POS_FRAMES, frame_index);
    if (cap.read(frame)) {
      break;
    }
    frame_index >>= 1;
    max_tries--;
  }
  if (max_tries == 0) {
    LOG(ERROR) << "Failed to read frame";
    return 2;
  }

  size_t frame_bytes_size =
      frame.total() * frame.elemSize(); // 计算帧数据的字节数
  // LOG(INFO) << "Size of the frame in bytes: " << frame_bytes_size;
  std::vector<int> compression_params;                    // 压缩参数
  compression_params.push_back(cv::IMWRITE_JPEG_QUALITY); // 设置JPEG压缩质量

  int quality = 100; // 初始高质量
  bool ret = false;
  std::vector<unsigned char> buf;

  compression_params.back() = quality;
  ret = cv::imencode(".jpg", frame, buf, compression_params);

  // 检查是否编码成功
  if (!ret) {
    LOG(ERROR) << "Failed to encode frame to memory buffer";
    return 3;
  }

  // 将视频帧数据和时间戳打包为一个对象
  typename ypc::cast_obj_to_package<video_frame_t>::type pkg;
  pkg.set<::video_frame>(buf);
  pkg.set<::total_duration>(total_duration);
  pkg.set<::frame_ts>(total_duration * frame_index / total_frames);
  out = ypc::make_bytes<ypc::bytes>::for_package(pkg);
  cap.release();
  return 0;
}
