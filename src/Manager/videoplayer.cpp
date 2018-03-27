#include "videoplayer.h"

#include "../DataModel/opendatafile.h"
#include "../DataModel/vitnessdatamodel.h"
#include <elidedlabel.h>
#include <helplink.h>

#include <sstream>

#include <QVideoWidget>
#include <QtWidgets>

using namespace AlenkaFile;
using namespace std;

namespace {

const string VIDEO_STR_PREFIX = "video";

} // namespace

VideoPlayer::VideoPlayer(QWidget *parent) : QWidget(parent) {
  videoWidget = new QVideoWidget(this);
  player = new QMediaPlayer(this);
  player->setVideoOutput(videoWidget);
  // TODO: Figure out why an interval of less than a second doesn't work.
  //  player->setNotifyInterval(100);

  buildUI();

  connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this,
          SLOT(selectMontage(int)));
  connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int, double)), this,
          SLOT(seek(int)));
  connect(player, SIGNAL(positionChanged(qint64)), this,
          SLOT(updatePlayPosition(qint64)));
}

void VideoPlayer::changeFile(OpenDataFile *file) {
  this->file = file;

  if (file) {
    selectMontage(OpenDataFile::infoTable.getSelectedMontage());
    seek(OpenDataFile::infoTable.getPosition());
  } else {
    stopAndSetPlayer();
  }
}

void VideoPlayer::togglePlay() {
  if ((playing = !playing))
    player->play();
  else
    player->pause();
}

void VideoPlayer::selectMontage(const int montageIndex) {
  if (!file)
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

void VideoPlayer::toggleMute() {
  QIcon icon;
  if ((muted = !muted))
    icon = style()->standardIcon(QStyle::SP_MediaVolumeMuted);
  else
    icon = style()->standardIcon(QStyle::SP_MediaVolume);

  muteButton->setIcon(icon);
  player->setMuted(muted);
}

void VideoPlayer::updatePlayPauseButton(const QMediaPlayer::State state) {
  QIcon icon;
  if (QMediaPlayer::PlayingState == state)
    icon = style()->standardIcon(QStyle::SP_MediaPause);
  else
    icon = style()->standardIcon(QStyle::SP_MediaPlay);
  playPauseButton->setIcon(icon);
}

void VideoPlayer::updateTimeLabel() {
  QTime time(0, 0), duration(0, 0);

  if (player->isVideoAvailable()) {
    time = time.addMSecs(player->position());
    duration = duration.addMSecs(player->duration());
  }

  timeLabel->setText(time.toString() + "/" + duration.toString());
}

void VideoPlayer::updateErrorLabel(const QMediaPlayer::Error err) {
  QString msg, color = "black";

  if (err != QMediaPlayer::NoError) {
    msg = player->errorString();
    if (msg.isEmpty())
      msg = "Unknown error";
    msg = " (" + msg + ")";
    color = "red";
  }

  errorLabel->setText(currentVideoFile.url.toString() + msg);
  errorLabel->setStyleSheet("QLabel { color : " + color + "; }");
}

void VideoPlayer::updatePlayPosition(const qint64 videoPosition) {
  if (file && playing) {
    double position = videoPosition / 1000 - currentVideoFile.offset;
    position *= file->file->getSamplingFrequency();
    position += currentVideoFile.position;

    const int intPosition = static_cast<int>(round(position));
    OpenDataFile::infoTable.setPosition(intPosition);

    // We reached the end of the event that represents a file -- stop playing.
    // For some reason just calling togglePlay() here causes a crash. I think
    // that player->pause() destroys some object that is still needed in the
    // main event loop when this slot returns. Creating a single shot timer
    // effectively executes togglePlay() in the earliest possible iteration of
    // the event loop. In the end there should be no perceivable difference.
    if (intPosition >= currentVideoFile.position + currentVideoFile.duration)
      QTimer::singleShot(0, this, SLOT(togglePlay()));
  }
}

void VideoPlayer::seek(const int position) {
  if (!file || playing)
    return; // If the player is playing, we don't need to seek.

  bool found;
  VideoFile videoFile;
  std::tie(found, videoFile) = selectFile(position);

  if (found) {
    bool const isNewVideo = currentVideoFile.url != videoFile.url;
    if (isNewVideo)
      stopAndSetPlayer(videoFile);

    seekPlayer(position, videoFile);

    if (isNewVideo)
      startPlayerPaused();
  } else {
    stopAndSetPlayer();
  }
}

void VideoPlayer::stopAndSetPlayer(const VideoFile &newVideoFile) {
  player->stop();
  currentVideoFile = newVideoFile;
  player->setMedia(currentVideoFile.url);
}

void VideoPlayer::seekPlayer(const int position, const VideoFile &videoFile) {
  double videoPosition = position - videoFile.position;
  videoPosition /= file->file->getSamplingFrequency();
  videoPosition += videoFile.offset;
  player->setPosition(static_cast<qint64>(videoPosition) * 1000);
}

void VideoPlayer::startPlayerPaused() {
  // You must call play at least once otherwise only black is displayed.
  player->play();

  if (playing)
    togglePlay();
  else
    player->pause();

  assert(!playing && "We mus be in paused state");
  assert(QMediaPlayer::PlayingState != player->state() &&
         "The player should not be playing at this point");

  updateErrorLabel();
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

      if (url.isValid())
        fileList.push_back({event.position, event.duration, offset, url});
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

void VideoPlayer::buildUI() {
  auto box = new QVBoxLayout();
  box->addWidget(videoWidget);
  auto controlBbox = new QHBoxLayout();

  playPauseButton = new QPushButton();
  playPauseButton->setToolTip("play/pause");
  controlBbox->addWidget(playPauseButton);
  connect(playPauseButton, SIGNAL(clicked(bool)), this, SLOT(togglePlay()));
  connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this,
          SLOT(updatePlayPauseButton(QMediaPlayer::State)));

  timeLabel = new QLabel();
  timeLabel->setToolTip("position in video/video duration");
  controlBbox->addWidget(timeLabel);
  connect(player, SIGNAL(positionChanged(qint64)), this,
          SLOT(updateTimeLabel()));
  updateTimeLabel();

  muteButton = new QPushButton();
  muteButton->setToolTip("mute");
  controlBbox->addWidget(muteButton);
  connect(muteButton, SIGNAL(clicked(bool)), this, SLOT(toggleMute()));
  toggleMute();

  auto volume = new QSlider(Qt::Horizontal);
  volume->setToolTip("volume");
  controlBbox->addWidget(volume);
  const int volumeWidth = 40;
  volume->setMinimumWidth(volumeWidth);
  volume->setMaximumWidth(volumeWidth);
  volume->setTickInterval(100);
  connect(volume, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));
  volume->setValue(100);

  errorLabel = new ElidedLabel();
  controlBbox->addWidget(errorLabel);
  const int pathWidth = 100;
  errorLabel->setMinimumWidth(pathWidth);
  connect(player, SIGNAL(error(QMediaPlayer::Error)), this,
          SLOT(updateErrorLabel(QMediaPlayer::Error)));
  updateErrorLabel(QMediaPlayer::NoError);

  // This must be here so that the help is pinned to the right.
  controlBbox->addStretch(1);

  auto helpLink =
      new HelpLink("https://github.com/machta/Alenka/wiki/"
                   "User-Manual#video-player----video-file-synchronization",
                   "User Manual: Video Player");
  controlBbox->addWidget(helpLink);

  box->addLayout(controlBbox);
  setLayout(box);
}
