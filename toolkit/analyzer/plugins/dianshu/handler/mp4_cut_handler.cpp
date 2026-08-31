#include "ocall_def.h"
#include "ypc/core/byte.h"

#include <cstdio>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <utility>

// 用于存储截取视频片段的结果以及其他需要的参数
define_nt(mp4_file_path, std::string);
define_nt(total_sec, int);  // 总时长(总秒数表示)
define_nt(start_time, int); // 截取视频片段的开始时间
define_nt(end_time, int);   // 截取视频片段的结束时间
define_nt(video_clip, std::vector<unsigned char>); // 视频片段数据
typedef ::ff::util::ntobject<mp4_file_path, total_sec, start_time, end_time,
                             video_clip>
    video_clip_t;

typename ypc::cast_obj_to_package<video_clip_t>::type global_mp4_info_pkg;

namespace {
class scoped_file_cleanup final {
public:
  explicit scoped_file_cleanup(std::string path) : path_(std::move(path)) {}
  scoped_file_cleanup(const scoped_file_cleanup &) = delete;
  scoped_file_cleanup &operator=(const scoped_file_cleanup &) = delete;
  scoped_file_cleanup(scoped_file_cleanup &&) = delete;
  scoped_file_cleanup &operator=(scoped_file_cleanup &&) = delete;
  ~scoped_file_cleanup() { (void)std::remove(path_.c_str()); }

private:
  std::string path_;
};
} // namespace

uint32_t handle_mp4_cut(const ypc::bytes &in, ypc::bytes &out) {
  LOG(INFO) << "handle mp4_cut";
  auto pkg = ypc::make_package<
      typename ypc::cast_obj_to_package<video_clip_t>::type>::from_bytes(in);
  std::string filename = pkg.get<::mp4_file_path>();

  std::string raw_mp4_path = filename.c_str();
  std::string temp_mp4_output = raw_mp4_path.substr(0, 8) + ".mp4";
  scoped_file_cleanup raw_cleanup(raw_mp4_path);
  scoped_file_cleanup output_cleanup(temp_mp4_output);

  // 限制视频截取的最大时长20s
  double maxDuration = 20.0;

  cv::VideoCapture cap(raw_mp4_path);

  if (!cap.isOpened()) {
    LOG(ERROR) << "Error opening video file";
    return -1;
  }

  double fps = cap.get(cv::CAP_PROP_FPS);
  double totalDuration = cap.get(cv::CAP_PROP_FRAME_COUNT) / fps;
  double durationToCapture = std::min(maxDuration, totalDuration);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0, totalDuration - durationToCapture);
  double startTime = dis(gen);

  int startFrame = static_cast<int>(startTime * fps);
  int endFrame = static_cast<int>((startTime + durationToCapture) * fps);

  int fourcc = cv::VideoWriter::fourcc('a', 'v', 'c', '1');
  cv::Size frameSize(cap.get(cv::CAP_PROP_FRAME_WIDTH),
                     cap.get(cv::CAP_PROP_FRAME_HEIGHT));
  cv::VideoWriter writer(temp_mp4_output, fourcc, fps, frameSize, true);

  if (!writer.isOpened()) {
    LOG(INFO) << "Error: Cannot open video writer.";
    return -1;
  }

  cap.set(cv::CAP_PROP_POS_FRAMES, startFrame);
  cv::Mat frame;

  while (startFrame <= endFrame && cap.read(frame)) {
    writer.write(frame);
    startFrame++;
  }

  cap.release();
  writer.release();

  std::ifstream videoFile(temp_mp4_output, std::ios::binary);
  if (!videoFile.is_open()) {
    LOG(INFO) << "Error: Cannot open temp video file.";
    return -1;
  }

  // 正确初始化vector并读取文件
  std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(videoFile)),
                                    std::istreambuf_iterator<char>());
  videoFile.close();

  // 使用完毕后，删除临时文件
  std::remove(temp_mp4_output.c_str());
  std::remove(filename.c_str());

  global_mp4_info_pkg.set<::total_sec>(totalDuration);
  global_mp4_info_pkg.set<::start_time>(startTime);
  global_mp4_info_pkg.set<::end_time>(startTime + durationToCapture);
  global_mp4_info_pkg.set<::video_clip>(buffer);

  out = ypc::make_bytes<ypc::bytes>::for_package(global_mp4_info_pkg);
  return 0;
}
