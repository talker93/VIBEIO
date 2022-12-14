/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/media_source/file/utils/audio_buffer.h"

// added contents - 1130
using namespace std;
//#include "comms/sample/media_source/file/utils/AudioFileIf.h"
#include <iostream>
//using std::cout;
//using std::endl;
//
//
//static const int kBlockSize = 1024;
//float **ppfAudioData = 0;
//CAudioFileIf *phAudioFile = 0;
//std::fstream hOutputFile;
//CAudioFileIf::FileSpec_t stFileSpec;
//
//std::string sOutFilePath = "new_sample.txt";

// open the txt file
//hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
//if (!hOutputFile.is_open())
//{
//    cout << "Text file open error!";
//}

// added content finished


namespace dolbyio::comms::sample {

audio_buffer::audio_buffer(int samples, int sample_rate, int channels)
    : size_(channels * samples),
      data_(new int16_t[size_]),
      sample_rate_(sample_rate),
      channels_(channels),
      samples_(samples),
      index_(0) {}

void audio_buffer::reset() {
  index_ = 0;
}

void audio_buffer::push(int16_t value) {
  if (index_ < size_)
    data_[index_++] = value;
}

bool audio_buffer::full() {
  return index_ == size_;
}

const int16_t* audio_buffer::data() const {
  return data_.get();
}

int audio_buffer::sample_rate() const {
  return sample_rate_;
}

int audio_buffer::channels() const {
  return channels_;
}

int audio_buffer::samples() const {
  return samples_;
}

};  // namespace dolbyio::comms::sample
