#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>

#include <QMediaPlayer>
#include <QUrl>

#include <vector>

class OpenDataFile;
class QPushButton;
class QVideoWidget;
class QLabel;
class ElidedLabel;
namespace AlenkaFile {
struct Event;
}

/**
 * @brief Implements position synchronized vide player.
 */
class VideoPlayer : public QWidget {
  Q_OBJECT

  struct VideoFile {
    int position, duration;
    double offset;
    QUrl url;
  };

  OpenDataFile *file = nullptr;
  std::vector<VideoFile> fileList;
  VideoFile currentVideoFile;
  bool playing = false;
  bool muted = true;

  QVideoWidget *videoWidget;
  QMediaPlayer *player;
  QPushButton *playPauseButton, *muteButton;
  QLabel *timeLabel;
  ElidedLabel *errorLabel;
  std::vector<QMetaObject::Connection> connections;

public:
  explicit VideoPlayer(QWidget *parent = nullptr);

  void changeFile(OpenDataFile *file);

public slots:
  void togglePlay();

private slots:
  void selectMontage(int montageIndex);
  void updateFileList();
  void toggleMute();
  void updatePlayPauseButton(QMediaPlayer::State state);
  void updateTimeLabel();
  void updateErrorLabel(QMediaPlayer::Error err = QMediaPlayer::NoError);
  void updatePlayPosition(qint64 videoPosition);
  void seek(int position);

private:
  void stopAndSetPlayer(const VideoFile &newVideoFile = VideoFile());
  void seekPlayer(int position, const VideoFile &videoFile);
  void startPlayerPaused();
  void addVideoFile(const AlenkaFile::Event &event);
  std::pair<bool, VideoFile> selectFile(int position);
  void buildUI();
};

#endif // VIDEOPLAYER_H
