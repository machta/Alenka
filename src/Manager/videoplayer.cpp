#include "videoplayer.h"

#include "../DataModel/opendatafile.h"
#include "../DataModel/vitnessdatamodel.h"

#include <sstream>

#include <QHBoxLayout>
#include <QMediaPlayer>
#include <QVideoWidget>

using namespace AlenkaFile;
using namespace std;

namespace {

const string VIDEO_STR_PREFIX = "video";

} // namespace

VideoPlayer::VideoPlayer(QWidget *parent) : QWidget(parent) {
  videoWidget = new QVideoWidget(this);
  player = new QMediaPlayer(this);
  player->setVideoOutput(videoWidget);

  auto box = new QHBoxLayout();
  box->addWidget(videoWidget);
  setLayout(box);

  connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this,
          SLOT(selectMontage(int)));

  connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int, double)), this,
          SLOT(seek(int)));
}

void VideoPlayer::changeFile(OpenDataFile *file) {
  this->file = file;

  if (file) {
    selectMontage(OpenDataFile::infoTable.getSelectedMontage());
    seek(OpenDataFile::infoTable.getPosition());
  }
}

void VideoPlayer::togglePlay() {
  if ((playing = !playing))
    player->play();
}

void VideoPlayer::selectMontage(const int montageIndex) {
  if (nullptr == file)
    return;

  for (auto e : connections)
    disconnect(e);
  connections.clear();

  const AbstractMontageTable *mt = file->dataModel->montageTable();

  if (0 < mt->rowCount()) {
    auto vitness = VitnessEventTable::vitness(mt->eventTable(montageIndex));

    auto c = connect(vitness, SIGNAL(valueChanged(int, int)), this,
                     SLOT(updateFileList()));
    connections.push_back(c);

    c = connect(vitness, SIGNAL(rowsInserted(int, int)), this,
                SLOT(updateFileList()));
    connections.push_back(c);

    c = connect(vitness, SIGNAL(rowsRemoved(int, int)), this,
                SLOT(updateFileList()));
    connections.push_back(c);

    updateFileList();
  }
}

void VideoPlayer::updateFileList() {
  const AbstractEventTable *mt = file->dataModel->montageTable()->eventTable(
      OpenDataFile::infoTable.getSelectedMontage());
  const int count = mt->rowCount();

  fileList.clear();
  for (int i = 0; i < count; ++i)
    addVideoFile(mt->row(i));

  seek(OpenDataFile::infoTable.getPosition());
}

void VideoPlayer::seek(const int position) {
  if (playing)
    return;
  assert(nullptr != file);

  bool found;
  VideoFile videoFile;
  std::tie(found, videoFile) = selectFile(position);

  if (found) {
    bool const isNewVideo = currentVideoUrl != videoFile.url;
    if (isNewVideo) {
      currentVideoUrl = videoFile.url;
      player->setMedia(currentVideoUrl);
    }

    // Seek.
    double videoPosition = position - videoFile.position;
    videoPosition /= file->file->getSamplingFrequency();
    videoPosition += videoFile.offset;
    player->setPosition(static_cast<qint64>(videoPosition) * 1000);

    if (isNewVideo && !playing) {
      // You must call play at least once otherwise no video is displayed.
      player->play();
      player->pause();
    }
  } else {
    player->stop();
    currentVideoUrl = QUrl();
  }
}

void VideoPlayer::addVideoFile(const Event &event) {
  const size_t prefixSize = VIDEO_STR_PREFIX.size();
  const string str = event.description;

  if (str.substr(0, prefixSize) == VIDEO_STR_PREFIX) {
    stringstream ss(str);
    ss.ignore(prefixSize);

    double offset;
    ss >> offset;
    offset = max<double>(0, offset);

    if (ss.good()) {
      string rest;
      ss >> rest;
      QUrl url(QString::fromStdString(rest));

      if (url.isValid()) {
        fileList.push_back({event.position, event.duration, offset, url});
      }
    }
  }
}

pair<bool, VideoPlayer::VideoFile> VideoPlayer::selectFile(const int position) {
  const auto it = std::find_if(
      fileList.begin(), fileList.end(), [position](const VideoFile &file) {
        return file.position <= position &&
               position < file.position + file.duration;
      });

  if (it == fileList.end())
    return {false, {}};
  else
    return {true, *it};
}
